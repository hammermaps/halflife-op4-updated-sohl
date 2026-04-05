/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "saverestore.h"
#include "client.h"
#include "decals.h"
#include "gamerules.h"
#include "game.h"
#include "pm_shared.h"
#include "movewith.h"
#include "debug.h"
#include "logger.h"

void EntvarsKeyvalue(entvars_t* pev, KeyValueData* pkvd);

void OnFreeEntPrivateData(edict_s* pEdict);

extern Vector VecBModelOrigin(entvars_t* pevBModel);

static DLL_FUNCTIONS gFunctionTable =
	{
		GameDLLInit,			   //pfnGameInit
		DispatchSpawn,			   //pfnSpawn
		DispatchThink,			   //pfnThink
		DispatchUse,			   //pfnUse
		DispatchTouch,			   //pfnTouch
		DispatchBlocked,		   //pfnBlocked
		DispatchKeyValue,		   //pfnKeyValue
		DispatchSave,			   //pfnSave
		DispatchRestore,		   //pfnRestore
		DispatchObjectCollsionBox, //pfnAbsBox

		SaveWriteFields, //pfnSaveWriteFields
		SaveReadFields,	 //pfnSaveReadFields

		SaveGlobalState,	//pfnSaveGlobalState
		RestoreGlobalState, //pfnRestoreGlobalState
		ResetGlobalState,	//pfnResetGlobalState

		ClientConnect,		   //pfnClientConnect
		ClientDisconnect,	   //pfnClientDisconnect
		ClientKill,			   //pfnClientKill
		ClientPutInServer,	   //pfnClientPutInServer
		ClientCommand,		   //pfnClientCommand
		ClientUserInfoChanged, //pfnClientUserInfoChanged
		ServerActivate,		   //pfnServerActivate
		ServerDeactivate,	   //pfnServerDeactivate

		PlayerPreThink,	 //pfnPlayerPreThink
		PlayerPostThink, //pfnPlayerPostThink

		StartFrame,		  //pfnStartFrame
		ParmsNewLevel,	  //pfnParmsNewLevel
		ParmsChangeLevel, //pfnParmsChangeLevel

		GetGameDescription,	 //pfnGetGameDescription    Returns string describing current .dll game.
		PlayerCustomization, //pfnPlayerCustomization   Notifies .dll of new customization for player.

		SpectatorConnect,	 //pfnSpectatorConnect      Called when spectator joins server
		SpectatorDisconnect, //pfnSpectatorDisconnect   Called when spectator leaves the server
		SpectatorThink,		 //pfnSpectatorThink        Called when spectator sends a command packet (usercmd_t)

		Sys_Error, //pfnSys_Error				Called when engine has encountered an error

		PM_Move,			//pfnPM_Move
		PM_Init,			//pfnPM_Init				Server version of player movement initialization
		PM_FindTextureType, //pfnPM_FindTextureType

		SetupVisibility,		  //pfnSetupVisibility        Set up PVS and PAS for networking for this client
		UpdateClientData,		  //pfnUpdateClientData       Set up data sent only to specific client
		AddToFullPack,			  //pfnAddToFullPack
		CreateBaseline,			  //pfnCreateBaseline			Tweak entity baseline for network encoding, allows setup of player baselines, too.
		RegisterEncoders,		  //pfnRegisterEncoders		Callbacks for network encoding
		GetWeaponData,			  //pfnGetWeaponData
		CmdStart,				  //pfnCmdStart
		CmdEnd,					  //pfnCmdEnd
		ConnectionlessPacket,	  //pfnConnectionlessPacket
		GetHullBounds,			  //pfnGetHullBounds
		CreateInstancedBaselines, //pfnCreateInstancedBaselines
		InconsistentFile,		  //pfnInconsistentFile
		AllowLagCompensation,	  //pfnAllowLagCompensation
};

NEW_DLL_FUNCTIONS gNewDLLFunctions =
	{
		OnFreeEntPrivateData, //pfnOnFreeEntPrivateData
		GameDLLShutdown,
};

static void SetObjectCollisionBox(entvars_t* pev);

extern "C" {

int GetEntityAPI(DLL_FUNCTIONS* pFunctionTable, int interfaceVersion)
{
	if (!pFunctionTable || interfaceVersion != INTERFACE_VERSION)
	{
		return 0;
	}

	memcpy(pFunctionTable, &gFunctionTable, sizeof(DLL_FUNCTIONS));
	return 1;
}

int GetEntityAPI2(DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion)
{
	if (!pFunctionTable || *interfaceVersion != INTERFACE_VERSION)
	{
		// Tell engine what version we had, so it can figure out who is out of date.
		*interfaceVersion = INTERFACE_VERSION;
		return 0;
	}

	memcpy(pFunctionTable, &gFunctionTable, sizeof(DLL_FUNCTIONS));
	return 1;
}

int GetNewDLLFunctions(NEW_DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion)
{
	if (!pFunctionTable || *interfaceVersion != NEW_DLL_FUNCTIONS_VERSION)
	{
		*interfaceVersion = NEW_DLL_FUNCTIONS_VERSION;
		return 0;
	}

	memcpy(pFunctionTable, &gNewDLLFunctions, sizeof(gNewDLLFunctions));
	return 1;
}
}


int DispatchSpawn(edict_t* pent)
{
	CBaseEntity* pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pent));

	if (pEntity)
	{
		// Initialize these or entities who don't link to the world won't have anything in here
		pEntity->pev->absmin = pEntity->pev->origin - Vector(1, 1, 1);
		pEntity->pev->absmax = pEntity->pev->origin + Vector(1, 1, 1);

		pEntity->Spawn();

		// Try to get the pointer again, in case the spawn function deleted the entity.
		// UNDONE: Spawn() should really return a code to ask that the entity be deleted, but
		// that would touch too much code for me to do that right now.
		pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pent));

		if (pEntity)
		{
			if (g_pGameRules && !g_pGameRules->IsAllowedToSpawn(pEntity))
				return -1; // return that this entity should be deleted
			if ((pEntity->pev->flags & FL_KILLME) != 0)
				return -1;
		}


		// Handle global stuff here
		if (pEntity && !FStringNull(pEntity->pev->globalname))
		{
			const globalentity_t* pGlobal = gGlobalState.EntityFromTable(pEntity->pev->globalname);
			if (pGlobal)
			{
				// Already dead? delete
				if (pGlobal->state == GLOBAL_DEAD)
					return -1;
				else if (!FStrEq(STRING(gpGlobals->mapname), pGlobal->levelName))
					pEntity->MakeDormant(); // Hasn't been moved to this level yet, wait but stay alive
											// In this level & not dead, continue on as normal
			}
			else
			{
				// Spawned entities default to 'On'
				gGlobalState.EntityAdd(pEntity->pev->globalname, gpGlobals->mapname, GLOBAL_ON);
				//				ALERT( at_console, "Added global entity %s (%s)\n", STRING(pEntity->pev->classname), STRING(pEntity->pev->globalname) );
			}
		}
	}

	return 0;
}

void DispatchKeyValue(edict_t* pentKeyvalue, KeyValueData* pkvd)
{
	if (!pkvd || !pentKeyvalue)
		return;

	EntvarsKeyvalue(VARS(pentKeyvalue), pkvd);

	// If the key was an entity variable, or there's no class set yet, don't look for the object, it may
	// not exist yet.
	if (0 != pkvd->fHandled || pkvd->szClassName == nullptr)
		return;

	// Get the actualy entity object
	CBaseEntity* pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pentKeyvalue));

	if (!pEntity)
		return;

	// LRC - MoveWith for all entities
	if (FStrEq(pkvd->szKeyName, "movewith"))
	{
		pEntity->m_MoveWith = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = static_cast<int32>(true);
		return;
	}

	pkvd->fHandled = static_cast<int32>(pEntity->KeyValue(pkvd));
}

void DispatchTouch(edict_t* pentTouched, edict_t* pentOther)
{
	if (gTouchDisabled)
		return;

	CBaseEntity* pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pentTouched));
	CBaseEntity* pOther = static_cast<CBaseEntity*>(GET_PRIVATE(pentOther));

	if (pEntity && pOther && ((pEntity->pev->flags | pOther->pev->flags) & FL_KILLME) == 0)
		pEntity->Touch(pOther);
}


void DispatchUse(edict_t* pentUsed, edict_t* pentOther)
{
	CBaseEntity* pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pentUsed));
	CBaseEntity* pOther = static_cast<CBaseEntity*>(GET_PRIVATE(pentOther));

	if (pEntity && (pEntity->pev->flags & FL_KILLME) == 0)
		pEntity->Use(pOther, pOther, USE_TOGGLE, 0);
}

void DispatchThink(edict_t* pent)
{
	CBaseEntity* pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pent));
	if (pEntity)
	{
		if (FBitSet(pEntity->pev->flags, FL_DORMANT))
			LOG_ERROR("Dormant entity %s is thinking!!", STRING(pEntity->pev->classname));

		// LRC - correct m_fNextThink if the engine has changed pev->nextthink
		pEntity->ThinkCorrection();

		pEntity->Think();
	}
}

void DispatchBlocked(edict_t* pentBlocked, edict_t* pentOther)
{
	CBaseEntity* pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pentBlocked));
	CBaseEntity* pOther = static_cast<CBaseEntity*>(GET_PRIVATE(pentOther));

	if (pEntity)
		pEntity->Blocked(pOther);
}

void DispatchSave(edict_t* pent, SAVERESTOREDATA* pSaveData)
{
	gpGlobals->time = pSaveData->time;

	CBaseEntity* pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pent));

	if (pEntity && CSaveRestoreBuffer::IsValidSaveRestoreData(pSaveData))
	{
		ENTITYTABLE* pTable = &pSaveData->pTable[pSaveData->currentIndex];

		if (pTable->pent != pent)
			LOG_ERROR("ENTITY TABLE OR INDEX IS WRONG!!!!");

		if ((pEntity->ObjectCaps() & FCAP_DONT_SAVE) != 0)
			return;

		// These don't use ltime & nextthink as times really, but we'll fudge around it.
		if (pEntity->pev->movetype == MOVETYPE_PUSH)
		{
			float delta = pEntity->pev->nextthink - pEntity->pev->ltime;
			pEntity->pev->ltime = gpGlobals->time;
			pEntity->pev->nextthink = pEntity->pev->ltime + delta;
		}

		pTable->location = pSaveData->size;			 // Remember entity position for file I/O
		pTable->classname = pEntity->pev->classname; // Remember entity class for respawn

		CSave saveHelper(*pSaveData);
		pEntity->Save(saveHelper);

		pTable->size = pSaveData->size - pTable->location; // Size of entity block is data size written to block
	}
}

void OnFreeEntPrivateData(edict_s* pEdict)
{
	if (pEdict && pEdict->pvPrivateData)
	{
		auto entity = reinterpret_cast<CBaseEntity*>(pEdict->pvPrivateData);

		delete entity;

		//Zero this out so the engine doesn't try to free it again.
		pEdict->pvPrivateData = nullptr;
	}
}

// Find the matching global entity.  Spit out an error if the designer made entities of
// different classes with the same global name
CBaseEntity* FindGlobalEntity(string_t classname, string_t globalname)
{
	edict_t* pent = FIND_ENTITY_BY_STRING(NULL, "globalname", STRING(globalname));
	CBaseEntity* pReturn = CBaseEntity::Instance(pent);
	if (pReturn)
	{
		if (!FClassnameIs(pReturn->pev, STRING(classname)))
		{
			LOG_INFO("Global entity found %s, wrong class %s", STRING(globalname), STRING(pReturn->pev->classname));
			pReturn = NULL;
		}
	}

	return pReturn;
}


int DispatchRestore(edict_t* pent, SAVERESTOREDATA* pSaveData, int globalEntity)
{
	gpGlobals->time = pSaveData->time;

	CBaseEntity* pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pent));

	if (pEntity && CSaveRestoreBuffer::IsValidSaveRestoreData(pSaveData))
	{
		entvars_t tmpVars;
		Vector oldOffset;

		CRestore restoreHelper(*pSaveData);
		if (0 != globalEntity)
		{
			CRestore tmpRestore(*pSaveData);
			tmpRestore.PrecacheMode(false);
			tmpRestore.ReadEntVars("ENTVARS", &tmpVars);

			// HACKHACK - reset the save pointers, we're going to restore for real this time
			pSaveData->size = pSaveData->pTable[pSaveData->currentIndex].location;
			pSaveData->pCurrentData = pSaveData->pBaseData + pSaveData->size;
			// -------------------


			const globalentity_t* pGlobal = gGlobalState.EntityFromTable(tmpVars.globalname);

			// Don't overlay any instance of the global that isn't the latest
			// pSaveData->szCurrentMapName is the level this entity is coming from
			// pGlobla->levelName is the last level the global entity was active in.
			// If they aren't the same, then this global update is out of date.
			if (!FStrEq(pSaveData->szCurrentMapName, pGlobal->levelName))
				return 0;

			// Compute the new global offset
			oldOffset = pSaveData->vecLandmarkOffset;
			CBaseEntity* pNewEntity = FindGlobalEntity(tmpVars.classname, tmpVars.globalname);
			if (pNewEntity)
			{
				//				ALERT( at_console, "Overlay %s with %s\n", STRING(pNewEntity->pev->classname), STRING(tmpVars.classname) );
				// Tell the restore code we're overlaying a global entity from another level
				restoreHelper.SetGlobalMode(true); // Don't overwrite global fields
				pSaveData->vecLandmarkOffset = (pSaveData->vecLandmarkOffset - pNewEntity->pev->mins) + tmpVars.mins;
				pEntity = pNewEntity; // we're going to restore this data OVER the old entity
				pent = ENT(pEntity->pev);
				// Update the global table to say that the global definition of this entity should come from this level
				gGlobalState.EntityUpdate(pEntity->pev->globalname, gpGlobals->mapname);
			}
			else
			{
				// This entity will be freed automatically by the engine.  If we don't do a restore on a matching entity (below)
				// or call EntityUpdate() to move it to this level, we haven't changed global state at all.
				return 0;
			}
		}

		if ((pEntity->ObjectCaps() & FCAP_MUST_SPAWN) != 0)
		{
			pEntity->Restore(restoreHelper);
			pEntity->Spawn();
		}
		else
		{
			pEntity->Restore(restoreHelper);
			pEntity->Precache();
		}

		// Again, could be deleted, get the pointer again.
		pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pent));

#if 0
		if ( pEntity && !FStringNull(pEntity->pev->globalname) && 0 != globalEntity ) 
		{
			LOG_INFO("Global %s is %s", STRING(pEntity->pev->globalname), STRING(pEntity->pev->model));
		}
#endif

		// Is this an overriding global entity (coming over the transition), or one restoring in a level
		if (0 != globalEntity)
		{
			//			ALERT( at_console, "After: %f %f %f %s\n", pEntity->pev->origin.x, pEntity->pev->origin.y, pEntity->pev->origin.z, STRING(pEntity->pev->model) );
			pSaveData->vecLandmarkOffset = oldOffset;
			if (pEntity)
			{
				UTIL_SetOrigin(pEntity->pev, pEntity->pev->origin);
				pEntity->OverrideReset();
			}
		}
		else if (pEntity && !FStringNull(pEntity->pev->globalname))
		{
			const globalentity_t* pGlobal = gGlobalState.EntityFromTable(pEntity->pev->globalname);
			if (pGlobal)
			{
				// Already dead? delete
				if (pGlobal->state == GLOBAL_DEAD)
					return -1;
				else if (!FStrEq(STRING(gpGlobals->mapname), pGlobal->levelName))
				{
					pEntity->MakeDormant(); // Hasn't been moved to this level yet, wait but stay alive
				}
				// In this level & not dead, continue on as normal
			}
			else
			{
				LOG_ERROR("Global Entity %s (%s) not in table!!!", STRING(pEntity->pev->globalname), STRING(pEntity->pev->classname));
				// Spawned entities default to 'On'
				gGlobalState.EntityAdd(pEntity->pev->globalname, gpGlobals->mapname, GLOBAL_ON);
			}
		}
	}
	return 0;
}


void DispatchObjectCollsionBox(edict_t* pent)
{
	CBaseEntity* pEntity = static_cast<CBaseEntity*>(GET_PRIVATE(pent));
	if (pEntity)
	{
		pEntity->SetObjectCollisionBox();
	}
	else
		SetObjectCollisionBox(&pent->v);
}


void SaveWriteFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount)
{
	if (!CSaveRestoreBuffer::IsValidSaveRestoreData(pSaveData))
	{
		return;
	}

	CSave saveHelper(*pSaveData);
	saveHelper.WriteFields(pname, pBaseData, pFields, fieldCount);
}


void SaveReadFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount)
{
	if (!CSaveRestoreBuffer::IsValidSaveRestoreData(pSaveData))
	{
		return;
	}

	// Always check if the player is stuck when loading a save game.
	g_CheckForPlayerStuck = true;

	CRestore restoreHelper(*pSaveData);
	restoreHelper.ReadFields(pname, pBaseData, pFields, fieldCount);
}

// give health
float CBaseEntity::TakeHealth(float flHealth, int bitsDamageType)
{
	if (0 == pev->takedamage)
		return 0;

	// heal
	if (pev->health >= pev->max_health)
		return 0;

	float flOldHealth = pev->health;
	pev->health += flHealth;

	if (pev->health > pev->max_health)
		pev->health = pev->max_health;

	return pev->health - flOldHealth;
}

// inflict damage on this entity.  bitsDamageType indicates type of damage inflicted, ie: DMG_CRUSH

bool CBaseEntity::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	Vector vecTemp;

	if (0 == pev->takedamage)
		return false;

	// UNDONE: some entity types may be immune or resistant to some bitsDamageType

	// if Attacker == Inflictor, the attack was a melee or other instant-hit attack.
	// (that is, no actual entity projectile was involved in the attack so use the shooter's origin).
	if (pevAttacker == pevInflictor)
	{
		vecTemp = pevInflictor->origin - (VecBModelOrigin(pev));
	}
	else
	// an actual missile was involved.
	{
		vecTemp = pevInflictor->origin - (VecBModelOrigin(pev));
	}

	// this global is still used for glass and other non-monster killables, along with decals.
	g_vecAttackDir = vecTemp.Normalize();

	// save damage based on the target's armor level

	// figure momentum add (don't let hurt brushes or other triggers move player)
	if ((!FNullEnt(pevInflictor)) && (pev->movetype == MOVETYPE_WALK || pev->movetype == MOVETYPE_STEP) && (pevAttacker->solid != SOLID_TRIGGER))
	{
		Vector vecDir = pev->origin - (pevInflictor->absmin + pevInflictor->absmax) * 0.5;
		vecDir = vecDir.Normalize();

		float flForce = flDamage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;

		if (flForce > 1000.0)
			flForce = 1000.0;
		pev->velocity = pev->velocity + vecDir * flForce;
	}

	// do the damage
	pev->health -= flDamage;
	if (pev->health <= 0)
	{
		Killed(pevAttacker, GIB_NORMAL);
		return false;
	}

	return true;
}


void CBaseEntity::Killed(entvars_t* pevAttacker, int iGib)
{
	pev->takedamage = DAMAGE_NO;
	pev->deadflag = DEAD_DEAD;
	UTIL_Remove(this);
}


CBaseEntity* CBaseEntity::GetNextTarget()
{
	if (FStringNull(pev->target))
		return nullptr;
	edict_t* pTarget = FIND_ENTITY_BY_TARGETNAME(nullptr, STRING(pev->target));
	if (FNullEnt(pTarget))
		return nullptr;

	return Instance(pTarget);
}

// Global Savedata for Delay
TYPEDESCRIPTION CBaseEntity::m_SaveData[] =
	{
		DEFINE_FIELD(CBaseEntity, m_pGoalEnt, FIELD_CLASSPTR),
		DEFINE_FIELD(CBaseEntity, m_EFlags, FIELD_CHARACTER),

		DEFINE_FIELD(CBaseEntity, m_pfnThink, FIELD_FUNCTION), // UNDONE: Build table of these!!!
		DEFINE_FIELD(CBaseEntity, m_pfnTouch, FIELD_FUNCTION),
		DEFINE_FIELD(CBaseEntity, m_pfnUse, FIELD_FUNCTION),
		DEFINE_FIELD(CBaseEntity, m_pfnBlocked, FIELD_FUNCTION),

		// LRC - MoveWith fields
		DEFINE_FIELD(CBaseEntity, m_MoveWith, FIELD_STRING),
		DEFINE_FIELD(CBaseEntity, m_pMoveWith, FIELD_CLASSPTR),
		DEFINE_FIELD(CBaseEntity, m_pChildMoveWith, FIELD_CLASSPTR),
		DEFINE_FIELD(CBaseEntity, m_pSiblingMoveWith, FIELD_CLASSPTR),
		DEFINE_FIELD(CBaseEntity, m_iLFlags, FIELD_INTEGER),
		DEFINE_FIELD(CBaseEntity, m_vecMoveWithOffset, FIELD_VECTOR),
		DEFINE_FIELD(CBaseEntity, m_vecRotWithOffset, FIELD_VECTOR),
		DEFINE_FIELD(CBaseEntity, m_activated, FIELD_BOOLEAN),
		// don't save m_pAssistLink, rebuild on restore
		DEFINE_FIELD(CBaseEntity, m_vecPostAssistVel, FIELD_VECTOR),
		DEFINE_FIELD(CBaseEntity, m_vecPostAssistAVel, FIELD_VECTOR),

		// LRC - Think/NextThink fields
		DEFINE_FIELD(CBaseEntity, m_fNextThink, FIELD_TIME),
		DEFINE_FIELD(CBaseEntity, m_fPevNextThink, FIELD_TIME),
};


bool CBaseEntity::Save(CSave& save)
{
	if (save.WriteEntVars("ENTVARS", pev))
		return save.WriteFields("BASE", this, m_SaveData, ARRAYSIZE(m_SaveData));

	return false;
}

bool CBaseEntity::Restore(CRestore& restore)
{
	bool status;

	status = restore.ReadEntVars("ENTVARS", pev);
	if (status)
		status = restore.ReadFields("BASE", this, m_SaveData, ARRAYSIZE(m_SaveData));

	if (pev->modelindex != 0 && !FStringNull(pev->model))
	{
		Vector mins, maxs;
		mins = pev->mins; // Set model is about to destroy these
		maxs = pev->maxs;


		PrecacheModel((char*)STRING(pev->model));
		SetModel(ENT(pev), STRING(pev->model));
		UTIL_SetSize(pev, mins, maxs); // Reset them
	}

	// LRC - correct think time after restore
	ThinkCorrection();

	return status;
}


// Initialize absmin & absmax to the appropriate box
void SetObjectCollisionBox(entvars_t* pev)
{
	if ((pev->solid == SOLID_BSP) &&
		(pev->angles != g_vecZero))
	{ // expand for rotation
		float max, v;
		int i;

		max = 0;
		for (i = 0; i < 3; i++)
		{
			v = fabs(((float*)pev->mins)[i]);
			if (v > max)
				max = v;
			v = fabs(((float*)pev->maxs)[i]);
			if (v > max)
				max = v;
		}
		for (i = 0; i < 3; i++)
		{
			((float*)pev->absmin)[i] = ((float*)pev->origin)[i] - max;
			((float*)pev->absmax)[i] = ((float*)pev->origin)[i] + max;
		}
	}
	else
	{
		pev->absmin = pev->origin + pev->mins;
		pev->absmax = pev->origin + pev->maxs;
	}

	pev->absmin.x -= 1;
	pev->absmin.y -= 1;
	pev->absmin.z -= 1;
	pev->absmax.x += 1;
	pev->absmax.y += 1;
	pev->absmax.z += 1;
}


void CBaseEntity::SetObjectCollisionBox()
{
	::SetObjectCollisionBox(pev);
}


bool CBaseEntity::Intersects(CBaseEntity* pOther)
{
	if (pOther->pev->absmin.x > pev->absmax.x ||
		pOther->pev->absmin.y > pev->absmax.y ||
		pOther->pev->absmin.z > pev->absmax.z ||
		pOther->pev->absmax.x < pev->absmin.x ||
		pOther->pev->absmax.y < pev->absmin.y ||
		pOther->pev->absmax.z < pev->absmin.z)
		return false;
	return true;
}

void CBaseEntity::MakeDormant()
{
	SetBits(pev->flags, FL_DORMANT);

	// Don't touch
	pev->solid = SOLID_NOT;
	// Don't move
	pev->movetype = MOVETYPE_NONE;
	// Don't draw
	SetBits(pev->effects, EF_NODRAW);
	// Don't think
	DontThink();
	// Relink
	UTIL_SetOrigin(pev, pev->origin);
}

bool CBaseEntity::IsDormant()
{
	return FBitSet(pev->flags, FL_DORMANT);
}

bool CBaseEntity::IsInWorld()
{
	// position
	if (pev->origin.x >= 4096)
		return false;
	if (pev->origin.y >= 4096)
		return false;
	if (pev->origin.z >= 4096)
		return false;
	if (pev->origin.x <= -4096)
		return false;
	if (pev->origin.y <= -4096)
		return false;
	if (pev->origin.z <= -4096)
		return false;
	// speed
	if (pev->velocity.x >= 2000)
		return false;
	if (pev->velocity.y >= 2000)
		return false;
	if (pev->velocity.z >= 2000)
		return false;
	if (pev->velocity.x <= -2000)
		return false;
	if (pev->velocity.y <= -2000)
		return false;
	if (pev->velocity.z <= -2000)
		return false;

	return true;
}

bool CBaseEntity::ShouldToggle(USE_TYPE useType, bool currentState)
{
	if (useType != USE_TOGGLE && useType != USE_SET)
	{
		if ((currentState && useType == USE_ON) || (!currentState && useType == USE_OFF))
			return false;
	}
	return true;
}


int CBaseEntity::DamageDecal(int bitsDamageType)
{
	if (pev->rendermode == kRenderTransAlpha)
		return -1;

	if (pev->rendermode != kRenderNormal)
		return DECAL_BPROOF1;

	return DECAL_GUNSHOT1 + RANDOM_LONG(0, 4);
}



// NOTE: szName must be a pointer to constant memory, e.g. "monster_class" because the entity
// will keep a pointer to it after this call.
CBaseEntity* CBaseEntity::Create(const char* szName, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner)
{
	edict_t* pent;
	CBaseEntity* pEntity;

	pent = CREATE_NAMED_ENTITY(MAKE_STRING(szName));
	if (FNullEnt(pent))
	{
		LOG_INFO("NULL Ent in Create!");
		return nullptr;
	}
	pEntity = Instance(pent);
	pEntity->pev->owner = pentOwner;
	pEntity->pev->origin = vecOrigin;
	pEntity->pev->angles = vecAngles;
	DispatchSpawn(pEntity->edict());
	return pEntity;
}


//=========================================================
// LRC - ShouldToggle version using GetState()
//=========================================================
int CBaseEntity::ShouldToggle(USE_TYPE useType)
{
	STATE currentState = GetState();
	if (useType != USE_TOGGLE && useType != USE_SET)
	{
		switch (currentState)
		{
		case STATE_ON:
		case STATE_TURN_ON:
			if (useType == USE_ON)
				return 0;
			break;
		case STATE_OFF:
		case STATE_TURN_OFF:
			if (useType == USE_OFF)
				return 0;
			break;
		}
	}
	return 1;
}


//=========================================================
// LRC - CBaseToggle::GetState, mapping toggle states to global states
//=========================================================
STATE CBaseToggle::GetState()
{
	switch (m_toggle_state)
	{
	case TS_AT_TOP:
		return STATE_ON;
	case TS_AT_BOTTOM:
		return STATE_OFF;
	case TS_GOING_UP:
		return STATE_TURN_ON;
	case TS_GOING_DOWN:
		return STATE_TURN_OFF;
	default:
		return STATE_OFF;
	}
}


//=========================================================
// LRC - Activate: called after all entities have been spawned
// and after loading a saved game.
//=========================================================
void CBaseEntity::Activate()
{
	// LRC - rebuild the new assistlist as the game starts
	if (m_activated)
		return;

	m_activated = true;

	// set up MoveWith connections
	InitMoveWith();

	// entity-specific init after MoveWith setup
	PostSpawn();
}


//=========================================================
// LRC - InitMoveWith: called by Activate to set up MoveWith
//=========================================================
void CBaseEntity::InitMoveWith()
{
	if (!m_MoveWith)
		return;

	CBaseEntity* pParent = UTIL_FindEntityByTargetname(NULL, STRING(m_MoveWith));
	if (!pParent)
	{
		LOG_INFO("Missing movewith entity %s", STRING(m_MoveWith));
		return;
	}

	// check for circular reference
	CBaseEntity* pTest = pParent;
	int loopbreaker = MAX_MOVEWITH_DEPTH;
	while (pTest)
	{
		if (pTest == this)
		{
			LOG_ERROR("Circular MoveWith reference: %s", STRING(pev->classname));
			return;
		}
		pTest = pTest->m_pMoveWith;
		if (--loopbreaker <= 0)
		{
			LOG_ERROR("MoveWith chain too deep for %s", STRING(pev->classname));
			return;
		}
	}

	// Check if we're already in parent's child list (e.g. after save/restore)
	CBaseEntity* pSibling = pParent->m_pChildMoveWith;
	while (pSibling)
	{
		if (pSibling == this)
			break;
		pSibling = pSibling->m_pSiblingMoveWith;
	}

	if (!pSibling) // not already linked — set up for the first time
	{
		m_pMoveWith = pParent;
		m_vecMoveWithOffset = pev->origin - pParent->pev->origin;
		m_vecRotWithOffset = pev->angles - pParent->pev->angles;

		// insert into parent's child list
		m_pSiblingMoveWith = pParent->m_pChildMoveWith;
		pParent->m_pChildMoveWith = this;

		// LRC - entities with MOVETYPE_NONE won't move even if velocity is set.
		// The engine's SV_Physics_None skips them entirely.
		// Change to an appropriate movetype so the engine will apply velocity.
		if (pev->movetype == MOVETYPE_NONE)
		{
			if (pev->solid == SOLID_BSP)
				pev->movetype = MOVETYPE_PUSH;
			else
				pev->movetype = MOVETYPE_NOCLIP;

			DEV_LOG(mw_debug, "InitMoveWith: changed %s '%s' movetype from NONE to %s",
				STRING(pev->classname),
				pev->targetname ? STRING(pev->targetname) : "<no name>",
				(pev->movetype == MOVETYPE_PUSH) ? "PUSH" : "NOCLIP");
		}
	}

	DEV_LOG(mw_debug, "InitMoveWith: [%s] '%s' -> parent [%s] '%s' offset(%.1f %.1f %.1f) rot(%.1f %.1f %.1f)",
		STRING(pev->classname),
		pev->targetname ? STRING(pev->targetname) : "<no name>",
		STRING(pParent->pev->classname),
		pParent->pev->targetname ? STRING(pParent->pev->targetname) : "<no name>",
		m_vecMoveWithOffset.x, m_vecMoveWithOffset.y, m_vecMoveWithOffset.z,
		m_vecRotWithOffset.x, m_vecRotWithOffset.y, m_vecRotWithOffset.z);
}


//=========================================================
// LRC - SetNextThink
// Set next think time relative to current time
//=========================================================
void CBaseEntity::SetNextThink(float delay, bool correctSpeed)
{
	// LRC - m_fNextThink is needed so that we can tell IsThinking
	// MOVETYPE_PUSH entities compare nextthink against pev->ltime (advanced by SV_PushMove).
	// All other movetypes compare nextthink against gpGlobals->time.
	if (pev->movetype == MOVETYPE_PUSH)
		m_fNextThink = pev->ltime + delay;
	else
		m_fNextThink = gpGlobals->time + delay;
  
	pev->nextthink = m_fPevNextThink;
}


//=========================================================
// LRC - AbsoluteNextThink
// Set next think time as an absolute (global) time
//=========================================================
void CBaseEntity::AbsoluteNextThink(float time, bool correctSpeed)
{
	m_fNextThink = time;
	m_fPevNextThink = m_fNextThink;
	pev->nextthink = m_fPevNextThink;
}


//=========================================================
// LRC - SetEternalThink
// Think continuously forever
//=========================================================
void CBaseEntity::SetEternalThink()
{
	// LRC - use the correct time base per movetype, just like SetNextThink.
	// MOVETYPE_PUSH uses pev->ltime (advanced by engine via SV_PushMove).
	// All other movetypes use gpGlobals->time (server clock).
	if (pev->movetype == MOVETYPE_PUSH)
		m_fNextThink = pev->ltime;
	else
		m_fNextThink = gpGlobals->time;
	m_fPevNextThink = m_fNextThink;
	pev->nextthink = m_fPevNextThink;
}


//=========================================================
// LRC - DontThink
// Cancel any pending thinks
//=========================================================
void CBaseEntity::DontThink()
{
	m_fNextThink = 0;
	m_fPevNextThink = 0;
	pev->nextthink = 0;
}


//=========================================================
// LRC - ResetThink
// Called by parent when a think needs to be aborted
//=========================================================
void CBaseEntity::ResetThink()
{
	m_fNextThink = 0;
	m_fPevNextThink = 0;
	pev->nextthink = 0;
	SetThink(NULL);
}


//=========================================================
// LRC - ThinkCorrection
// Checks in case the engine has changed our nextthink behind our back.
// If it has, we correct m_fNextThink to match.
//=========================================================
void CBaseEntity::ThinkCorrection()
{
	if (pev->nextthink != m_fPevNextThink)
	{
		// The engine changed nextthink without telling us. Update accordingly.
		m_fNextThink += pev->nextthink - m_fPevNextThink;
		m_fPevNextThink = pev->nextthink;
	}
}
