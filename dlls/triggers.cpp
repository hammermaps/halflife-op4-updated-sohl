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
/*

===== triggers.cpp ========================================================

  spawn and use functions for editor-placed triggers              

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "saverestore.h"
#include "trains.h" // trigger_camera has train functionality
#include "gamerules.h"
#include "skill.h"
#include "movewith.h" // LRC
#include "locus.h"	   // LRC
#include "ctf/ctfplay_gamerules.h"
#include "ctf/CTFGoalFlag.h"
#include "UserMessages.h"
#include "weapons.h"
#include "logger.h"

#define SF_TRIGGER_PUSH_START_OFF 2		   //spawnflag that makes trigger_push spawn turned OFF
#define SF_TRIGGER_HURT_TARGETONCE 1	   // Only fire hurt target once
#define SF_TRIGGER_HURT_START_OFF 2		   //spawnflag that makes trigger_push spawn turned OFF
#define SF_TRIGGER_HURT_NO_CLIENTS 8	   //spawnflag that makes trigger_push spawn turned OFF
#define SF_TRIGGER_HURT_CLIENTONLYFIRE 16  // trigger hurt will only fire its target if it is hurting a client
#define SF_TRIGGER_HURT_CLIENTONLYTOUCH 32 // only clients may touch this trigger.

extern void SetMovedir(entvars_t* pev);
extern Vector VecBModelOrigin(entvars_t* pevBModel);

class CFrictionModifier : public CBaseEntity
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT ChangeFriction(CBaseEntity* pOther);
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	static TYPEDESCRIPTION m_SaveData[];

	float m_frictionFraction; // Sorry, couldn't resist this name :)
};

LINK_ENTITY_TO_CLASS(func_friction, CFrictionModifier);

// Global Savedata for changelevel friction modifier
TYPEDESCRIPTION CFrictionModifier::m_SaveData[] =
	{
		DEFINE_FIELD(CFrictionModifier, m_frictionFraction, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CFrictionModifier, CBaseEntity);


// Modify an entity's friction
void CFrictionModifier::Spawn()
{
	pev->solid = SOLID_TRIGGER;
	SetModel(ENT(pev), STRING(pev->model)); // set size and link into world
	pev->movetype = MOVETYPE_NONE;
	SetTouch(&CFrictionModifier::ChangeFriction);
}


// Sets toucher's friction to m_frictionFraction (1.0 = normal friction)
void CFrictionModifier::ChangeFriction(CBaseEntity* pOther)
{
	if (pOther->pev->movetype != MOVETYPE_BOUNCEMISSILE && pOther->pev->movetype != MOVETYPE_BOUNCE)
		pOther->pev->friction = m_frictionFraction;
}



// Sets toucher's friction to m_frictionFraction (1.0 = normal friction)
bool CFrictionModifier::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "modifier"))
	{
		m_frictionFraction = atof(pkvd->szValue) / 100.0;
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}


// This trigger will fire when the level spawns (or respawns if not fire once)
// It will check a global state before firing.  It supports delay and killtargets

#define SF_AUTO_FIREONCE 0x0001

class CAutoTrigger : public CBaseDelay
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Precache() override;
	void Think() override;

	int ObjectCaps() override { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

private:
	int m_globalstate;
	USE_TYPE triggerType;
};
LINK_ENTITY_TO_CLASS(trigger_auto, CAutoTrigger);

TYPEDESCRIPTION CAutoTrigger::m_SaveData[] =
	{
		DEFINE_FIELD(CAutoTrigger, m_globalstate, FIELD_STRING),
		DEFINE_FIELD(CAutoTrigger, triggerType, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CAutoTrigger, CBaseDelay);

bool CAutoTrigger::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "globalstate"))
	{
		m_globalstate = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "triggerstate"))
	{
		int type = atoi(pkvd->szValue);
		switch (type)
		{
		case 0:
			triggerType = USE_OFF;
			break;
		case 2:
			triggerType = USE_TOGGLE;
			break;
		default:
			triggerType = USE_ON;
			break;
		}
		return true;
	}

	return CBaseDelay::KeyValue(pkvd);
}


void CAutoTrigger::Spawn()
{
	Precache();
}


void CAutoTrigger::Precache()
{
	SetNextThink(0.1);
}


void CAutoTrigger::Think()
{
	if (FStringNull(m_globalstate) || gGlobalState.EntityGetState(m_globalstate) == GLOBAL_ON)
	{
		SUB_UseTargets(this, triggerType, 0);
		if ((pev->spawnflags & SF_AUTO_FIREONCE) != 0)
			UTIL_Remove(this);
	}
}



#define SF_RELAY_FIREONCE 0x0001

class CTriggerRelay : public CBaseDelay
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	int ObjectCaps() override { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

private:
	USE_TYPE triggerType;
};
LINK_ENTITY_TO_CLASS(trigger_relay, CTriggerRelay);

TYPEDESCRIPTION CTriggerRelay::m_SaveData[] =
	{
		DEFINE_FIELD(CTriggerRelay, triggerType, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CTriggerRelay, CBaseDelay);

bool CTriggerRelay::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "triggerstate"))
	{
		int type = atoi(pkvd->szValue);
		switch (type)
		{
		case 0:
			triggerType = USE_OFF;
			break;
		case 2:
			triggerType = USE_TOGGLE;
			break;
		default:
			triggerType = USE_ON;
			break;
		}
		return true;
	}

	return CBaseDelay::KeyValue(pkvd);
}


void CTriggerRelay::Spawn()
{
}




void CTriggerRelay::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	SUB_UseTargets(this, triggerType, 0);
	if ((pev->spawnflags & SF_RELAY_FIREONCE) != 0)
		UTIL_Remove(this);
}


//**********************************************************
// The Multimanager Entity - when fired, will fire up to 16 targets
// at specified times.
// FLAG:		THREAD (create clones when triggered)
// FLAG:		CLONE (this is a clone for a threaded execution)

#define SF_MULTIMAN_CLONE 0x80000000
#define SF_MULTIMAN_THREAD 0x00000001
#define SF_MULTIMAN_SAMETRIG 0x00000002  // LRC - fire all targets with the same USE_TYPE

class CMultiManager : public CBaseToggle
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void EXPORT ManagerThink();
	void EXPORT ManagerUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

#if _DEBUG
	void EXPORT ManagerReport();
#endif

	bool HasTarget(string_t targetname) override;

	int ObjectCaps() override { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	STATE GetState() override { return m_iState; }  // LRC

	static TYPEDESCRIPTION m_SaveData[];

	int m_cTargets;							  // the total number of targets in this manager's fire list.
	int m_index;							  // Current target
	float m_startTime;						  // Time we started firing
	int m_iTargetName[MAX_MULTI_TARGETS];	  // list if indexes into global string array
	float m_flTargetDelay[MAX_MULTI_TARGETS]; // delay (in seconds) from time of manager fire to target fire
	STATE m_iState;							  // LRC

	// LRC - new multi_manager fields
	float m_flMgrWait;		 // wait time before firing
	float m_flMaxWait;		 // maximum random wait time
	int m_iMode;			 // 0=timed, 1=pick random, 2=each random
	int m_iszThreadName;	 // thread name for cloned managers
	int m_iszLocusThread;	 // locus thread name
	USE_TYPE m_triggerType;  // trigger type override
private:
	inline bool IsClone() { return (pev->spawnflags & SF_MULTIMAN_CLONE) != 0; }
	inline bool ShouldClone()
	{
		if (IsClone())
			return false;

		return (pev->spawnflags & SF_MULTIMAN_THREAD) != 0;
	}

	CMultiManager* Clone();
};
LINK_ENTITY_TO_CLASS(multi_manager, CMultiManager);

// Global Savedata for multi_manager
TYPEDESCRIPTION CMultiManager::m_SaveData[] =
	{
		DEFINE_FIELD(CMultiManager, m_cTargets, FIELD_INTEGER),
		DEFINE_FIELD(CMultiManager, m_index, FIELD_INTEGER),
		DEFINE_FIELD(CMultiManager, m_startTime, FIELD_TIME),
		DEFINE_ARRAY(CMultiManager, m_iTargetName, FIELD_STRING, MAX_MULTI_TARGETS),
		DEFINE_ARRAY(CMultiManager, m_flTargetDelay, FIELD_FLOAT, MAX_MULTI_TARGETS),
		// LRC - new fields
		DEFINE_FIELD(CMultiManager, m_flMgrWait, FIELD_FLOAT),
		DEFINE_FIELD(CMultiManager, m_flMaxWait, FIELD_FLOAT),
		DEFINE_FIELD(CMultiManager, m_iMode, FIELD_INTEGER),
		DEFINE_FIELD(CMultiManager, m_iszThreadName, FIELD_STRING),
		DEFINE_FIELD(CMultiManager, m_iszLocusThread, FIELD_STRING),
		DEFINE_FIELD(CMultiManager, m_triggerType, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CMultiManager, CBaseToggle);

bool CMultiManager::KeyValue(KeyValueData* pkvd)
{
	// UNDONE: Maybe this should do something like this:
	//CBaseToggle::KeyValue( pkvd );
	// if ( !pkvd->fHandled )
	// ... etc.

	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flMgrWait = atof(pkvd->szValue);
		return true;
	}
	// LRC - support for master, threadname, mode, triggerstate
	else if (FStrEq(pkvd->szKeyName, "master"))
	{
		m_sMaster = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszThreadName"))
	{
		m_iszThreadName = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszLocusThread"))
	{
		m_iszLocusThread = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "mode"))
	{
		m_iMode = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "triggerstate"))
	{
		int type = atoi(pkvd->szValue);
		switch (type)
		{
		case 0: m_triggerType = USE_OFF; break;
		case 1: m_triggerType = USE_ON; break;
		case 2: m_triggerType = USE_TOGGLE; break;
		default: m_triggerType = USE_TOGGLE; break;
		}
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "maxwait"))
	{
		m_flMaxWait = atof(pkvd->szValue);
		return true;
	}
	else // add this field to the target list
	{
		// this assumes that additional fields are targetnames and their values are delay values.
		if (m_cTargets < MAX_MULTI_TARGETS)
		{
			char tmp[128];

			UTIL_StripToken(pkvd->szKeyName, tmp, sizeof(tmp));
			m_iTargetName[m_cTargets] = ALLOC_STRING(tmp);
			m_flTargetDelay[m_cTargets] = atof(pkvd->szValue);
			m_cTargets++;
			return true;
		}
	}

	return false;
}


void CMultiManager::Spawn()
{
	pev->solid = SOLID_NOT;
	SetUse(&CMultiManager::ManagerUse);
	SetThink(&CMultiManager::ManagerThink);

	// Sort targets
	// Quick and dirty bubble sort
	bool swapped = true;

	while (swapped)
	{
		swapped = false;
		for (int i = 1; i < m_cTargets; i++)
		{
			if (m_flTargetDelay[i] < m_flTargetDelay[i - 1])
			{
				// Swap out of order elements
				int name = m_iTargetName[i];
				float delay = m_flTargetDelay[i];
				m_iTargetName[i] = m_iTargetName[i - 1];
				m_flTargetDelay[i] = m_flTargetDelay[i - 1];
				m_iTargetName[i - 1] = name;
				m_flTargetDelay[i - 1] = delay;
				swapped = true;
			}
		}
	}
}


bool CMultiManager::HasTarget(string_t targetname)
{
	for (int i = 0; i < m_cTargets; i++)
		if (FStrEq(STRING(targetname), STRING(m_iTargetName[i])))
			return true;

	return false;
}


// Designers were using this to fire targets that may or may not exist --
// so I changed it to use the standard target fire code, made it a little simpler.
void CMultiManager::ManagerThink()
{
	float time;

	time = gpGlobals->time - m_startTime;
	while (m_index < m_cTargets && m_flTargetDelay[m_index] <= time)
	{
		FireTargets(STRING(m_iTargetName[m_index]), m_hActivator, this, USE_TOGGLE, 0);
		m_index++;
	}

	if (m_index >= m_cTargets) // have we fired all targets?
	{
		m_iState = STATE_OFF;  // LRC - we've finished
		SetThink(NULL);
		if (IsClone())
		{
			UTIL_Remove(this);
			return;
		}
		SetUse(&CMultiManager::ManagerUse); // allow manager re-use
	}
	else
		pev->nextthink = m_startTime + m_flTargetDelay[m_index];
}

CMultiManager* CMultiManager::Clone()
{
	CMultiManager* pMulti = GetClassPtr((CMultiManager*)NULL);

	edict_t* pEdict = pMulti->pev->pContainingEntity;
	memcpy(pMulti->pev, pev, sizeof(*pev));
	pMulti->pev->pContainingEntity = pEdict;

	pMulti->pev->spawnflags |= SF_MULTIMAN_CLONE;
	pMulti->m_cTargets = m_cTargets;
	memcpy(pMulti->m_iTargetName, m_iTargetName, sizeof(m_iTargetName));
	memcpy(pMulti->m_flTargetDelay, m_flTargetDelay, sizeof(m_flTargetDelay));

	// LRC - copy new fields to clone
	pMulti->m_iszThreadName = m_iszThreadName;
	pMulti->m_triggerType = m_triggerType;
	pMulti->m_iMode = m_iMode;
	pMulti->m_flMgrWait = m_flMgrWait;
	pMulti->m_flMaxWait = m_flMaxWait;

	return pMulti;
}


// The USE function builds the time table and starts the entity thinking.
void CMultiManager::ManagerUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// LRC - master support
	if (!FStringNull(m_sMaster) && !UTIL_IsMasterTriggered(m_sMaster, pActivator))
		return;

	// In multiplayer games, clone the MM and execute in the clone (like a thread)
	// to allow multiple players to trigger the same multimanager
	if (ShouldClone())
	{
		CMultiManager* pClone = Clone();
		pClone->ManagerUse(pActivator, pCaller, useType, value);
		return;
	}

	m_hActivator = pActivator;
	m_index = 0;
	m_startTime = gpGlobals->time;

	// LRC - apply wait time if set
	if (m_flMgrWait != 0)
	{
		float fWait;
		if (m_flMaxWait != 0)
			fWait = RANDOM_FLOAT(m_flMgrWait, m_flMaxWait);
		else
			fWait = m_flMgrWait;
		m_startTime += fWait;
	}

	m_iState = STATE_ON;  // LRC - we're firing targets

	SetUse(NULL); // disable use until all targets have fired

	SetThink(&CMultiManager::ManagerThink);
	SetNextThink(0);
}

#if _DEBUG
void CMultiManager::ManagerReport()
{
	int cIndex;

	for (cIndex = 0; cIndex < m_cTargets; cIndex++)
	{
		ALERT(at_console, "%s %f\n", STRING(m_iTargetName[cIndex]), m_flTargetDelay[cIndex]);
	}
}
#endif

//***********************************************************


//
// Render parameters trigger
//
// This entity will copy its render parameters (renderfx, rendermode, rendercolor, renderamt)
// to its targets when triggered.
//


// Flags to indicate masking off various render parameters that are normally copied to the targets
#define SF_RENDER_MASKFX (1 << 0)
#define SF_RENDER_MASKAMT (1 << 1)
#define SF_RENDER_MASKMODE (1 << 2)
#define SF_RENDER_MASKCOLOR (1 << 3)

class CRenderFxManager : public CBaseEntity
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

LINK_ENTITY_TO_CLASS(env_render, CRenderFxManager);


void CRenderFxManager::Spawn()
{
	pev->solid = SOLID_NOT;
}

void CRenderFxManager::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!FStringNull(pev->target))
	{
		edict_t* pentTarget = NULL;
		while (true)
		{
			pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(pev->target));
			if (FNullEnt(pentTarget))
				break;

			entvars_t* pevTarget = VARS(pentTarget);
			if (!FBitSet(pev->spawnflags, SF_RENDER_MASKFX))
				pevTarget->renderfx = pev->renderfx;
			if (!FBitSet(pev->spawnflags, SF_RENDER_MASKAMT))
				pevTarget->renderamt = pev->renderamt;
			if (!FBitSet(pev->spawnflags, SF_RENDER_MASKMODE))
				pevTarget->rendermode = pev->rendermode;
			if (!FBitSet(pev->spawnflags, SF_RENDER_MASKCOLOR))
				pevTarget->rendercolor = pev->rendercolor;
		}
	}
}



class CBaseTrigger : public CBaseToggle
{
public:
	void EXPORT TeleportTouch(CBaseEntity* pOther);
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT MultiTouch(CBaseEntity* pOther);
	void EXPORT HurtTouch(CBaseEntity* pOther);
	void EXPORT CDAudioTouch(CBaseEntity* pOther);
	void ActivateMultiTrigger(CBaseEntity* pActivator);
	void EXPORT MultiWaitOver();
	void EXPORT CounterUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void InitTrigger();

	int ObjectCaps() override { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

LINK_ENTITY_TO_CLASS(trigger, CBaseTrigger);

/*
================
InitTrigger
================
*/
void CBaseTrigger::InitTrigger()
{
	// trigger angles are used for one-way touches.  An angle of 0 is assumed
	// to mean no restrictions, so use a yaw of 360 instead.
	if (pev->angles != g_vecZero)
		SetMovedir(pev);
	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_NONE;
	SetModel(ENT(pev), STRING(pev->model)); // set size and link into world
	if (CVAR_GET_FLOAT("showtriggers") == 0)
		SetBits(pev->effects, EF_NODRAW);
}


//
// Cache user-entity-field values until spawn is called.
//

bool CBaseTrigger::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "damage"))
	{
		pev->dmg = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "count"))
	{
		m_cTriggersLeft = (int)atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "damagetype"))
	{
		m_bitsDamageInflict = atoi(pkvd->szValue);
		return true;
	}

	return CBaseToggle::KeyValue(pkvd);
}

class CTriggerHurt : public CBaseTrigger
{
public:
	void Spawn() override;
	void EXPORT RadiationThink();
};

LINK_ENTITY_TO_CLASS(trigger_hurt, CTriggerHurt);

//
// trigger_monsterjump
//
class CTriggerMonsterJump : public CBaseTrigger
{
public:
	void Spawn() override;
	void Touch(CBaseEntity* pOther) override;
	void Think() override;
};

LINK_ENTITY_TO_CLASS(trigger_monsterjump, CTriggerMonsterJump);


void CTriggerMonsterJump::Spawn()
{
	SetMovedir(pev);

	InitTrigger();

	DontThink();
	pev->speed = 200;
	m_flHeight = 150;

	if (!FStringNull(pev->targetname))
	{ // if targetted, spawn turned off
		pev->solid = SOLID_NOT;
		UTIL_SetOrigin(pev, pev->origin); // Unlink from trigger list
		SetUse(&CTriggerMonsterJump::ToggleUse);
	}
}


void CTriggerMonsterJump::Think()
{
	pev->solid = SOLID_NOT;			  // kill the trigger for now !!!UNDONE
	UTIL_SetOrigin(pev, pev->origin); // Unlink from trigger list
	SetThink(NULL);
}

void CTriggerMonsterJump::Touch(CBaseEntity* pOther)
{
	entvars_t* pevOther = pOther->pev;

	if (!FBitSet(pevOther->flags, FL_MONSTER))
	{ // touched by a non-monster.
		return;
	}

	pevOther->origin.z += 1;

	if (FBitSet(pevOther->flags, FL_ONGROUND))
	{ // clear the onground so physics don't bitch
		pevOther->flags &= ~FL_ONGROUND;
	}

	// toss the monster!
	pevOther->velocity = pev->movedir * pev->speed;
	pevOther->velocity.z += m_flHeight;
	SetNextThink(0);
}


//=====================================
//
// trigger_cdaudio - starts/stops cd audio tracks
//
class CTriggerCDAudio : public CBaseTrigger
{
public:
	void Spawn() override;

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void PlayTrack();
	void Touch(CBaseEntity* pOther) override;
};

LINK_ENTITY_TO_CLASS(trigger_cdaudio, CTriggerCDAudio);

//
// Changes tracks or stops CD when player touches
//
// !!!HACK - overloaded HEALTH to avoid adding new field
void CTriggerCDAudio::Touch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
	{ // only clients may trigger these events
		return;
	}

	PlayTrack();
}

void CTriggerCDAudio::Spawn()
{
	InitTrigger();
}

void CTriggerCDAudio::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	PlayTrack();
}

void PlayCDTrack(int iTrack)
{
	// manually find the single player.
	CBaseEntity* pClient = UTIL_GetLocalPlayer();

	// Can't play if the client is not connected!
	if (!pClient)
		return;

	if (iTrack < -1 || iTrack > 30)
	{
		ALERT(at_console, "TriggerCDAudio - Track %d out of range\n");
		return;
	}

	if (iTrack == -1)
	{
		CLIENT_COMMAND(pClient->edict(), "cd stop\n");
	}
	else
	{
		char string[64];

		snprintf(string, sizeof(string), "cd play %3d\n", iTrack);
		CLIENT_COMMAND(pClient->edict(), string);
	}
}


// only plays for ONE client, so only use in single play!
void CTriggerCDAudio::PlayTrack()
{
	PlayCDTrack((int)pev->health);

	SetTouch(NULL);
	UTIL_Remove(this);
}


// This plays a CD track when fired or when the player enters it's radius
class CTargetCDAudio : public CPointEntity
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void Think() override;
	void Play();
};

LINK_ENTITY_TO_CLASS(target_cdaudio, CTargetCDAudio);

bool CTargetCDAudio::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "radius"))
	{
		pev->scale = atof(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

void CTargetCDAudio::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	if (pev->scale > 0)
		SetNextThink(1.0);
}

void CTargetCDAudio::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	Play();
}

// only plays for ONE client, so only use in single play!
void CTargetCDAudio::Think()
{
	// manually find the single player.
	CBaseEntity* pClient = UTIL_GetLocalPlayer();

	// Can't play if the client is not connected!
	if (!pClient)
		return;

	SetNextThink(0.5);

	if ((pClient->pev->origin - pev->origin).Length() <= pev->scale)
		Play();
}

void CTargetCDAudio::Play()
{
	PlayCDTrack((int)pev->health);
	UTIL_Remove(this);
}

//=====================================

//
// trigger_hurt - hurts anything that touches it. if the trigger has a targetname, firing it will toggle state
//
//int gfToggleState = 0; // used to determine when all radiation trigger hurts have called 'RadiationThink'

void CTriggerHurt::Spawn()
{
	InitTrigger();
	SetTouch(&CTriggerHurt::HurtTouch);

	if (!FStringNull(pev->targetname))
	{
		SetUse(&CTriggerHurt::ToggleUse);
	}
	else
	{
		SetUse(NULL);
	}

	if ((m_bitsDamageInflict & DMG_RADIATION) != 0)
	{
		SetThink(&CTriggerHurt::RadiationThink);
		SetNextThink(RANDOM_FLOAT(0.0, 0.5));
	}

	if (FBitSet(pev->spawnflags, SF_TRIGGER_HURT_START_OFF)) // if flagged to Start Turned Off, make trigger nonsolid.
		pev->solid = SOLID_NOT;

	UTIL_SetOrigin(pev, pev->origin); // Link into the list
}

// trigger hurt that causes radiation will do a radius
// check and set the player's geiger counter level
// according to distance from center of trigger

void CTriggerHurt::RadiationThink()
{

	edict_t* pentPlayer;
	CBasePlayer* pPlayer = NULL;
	float flRange;
	entvars_t* pevTarget;
	Vector vecSpot1;
	Vector vecSpot2;
	Vector vecRange;
	Vector origin;
	Vector view_ofs;

	// check to see if a player is in pvs
	// if not, continue

	// set origin to center of trigger so that this check works
	origin = pev->origin;
	view_ofs = pev->view_ofs;

	pev->origin = (pev->absmin + pev->absmax) * 0.5;
	pev->view_ofs = pev->view_ofs * 0.0;

	pentPlayer = FIND_CLIENT_IN_PVS(edict());

	pev->origin = origin;
	pev->view_ofs = view_ofs;

	// reset origin

	if (!FNullEnt(pentPlayer))
	{

		pPlayer = GetClassPtr((CBasePlayer*)VARS(pentPlayer));

		pevTarget = VARS(pentPlayer);

		// get range to player;

		vecSpot1 = (pev->absmin + pev->absmax) * 0.5;
		vecSpot2 = (pevTarget->absmin + pevTarget->absmax) * 0.5;

		vecRange = vecSpot1 - vecSpot2;
		flRange = vecRange.Length();

		// if player's current geiger counter range is larger
		// than range to this trigger hurt, reset player's
		// geiger counter range

		if (pPlayer->m_flgeigerRange >= flRange)
			pPlayer->m_flgeigerRange = flRange;
	}

	SetNextThink(0.25);
}

//
// ToggleUse - If this is the USE function for a trigger, its state will toggle every time it's fired
//
void CBaseTrigger::ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// LRC - ShouldToggle: respect USE_ON / USE_OFF / USE_TOGGLE
	if (!ShouldToggle(useType, pev->solid == SOLID_TRIGGER))
		return;

	// SoHL 1.5 - Always update, even when pActivator is null, so stale activators are not reused.
	m_hActivator = pActivator;

	if (pev->solid == SOLID_NOT)
	{ // if the trigger is off, turn it on
		pev->solid = SOLID_TRIGGER;

		// Force retouch
		gpGlobals->force_retouch++;
	}
	else
	{ // turn the trigger off
		pev->solid = SOLID_NOT;
	}
	UTIL_SetOrigin(pev, pev->origin);
}

// When touched, a hurt trigger does DMG points of damage each half-second
void CBaseTrigger::HurtTouch(CBaseEntity* pOther)
{
	float fldmg;

	if (0 == pOther->pev->takedamage)
		return;

	if ((pev->spawnflags & SF_TRIGGER_HURT_CLIENTONLYTOUCH) != 0 && !pOther->IsPlayer())
	{
		// this trigger is only allowed to touch clients, and this ain't a client.
		return;
	}

	if ((pev->spawnflags & SF_TRIGGER_HURT_NO_CLIENTS) != 0 && pOther->IsPlayer())
		return;

	static_assert(MAX_PLAYERS <= 32, "Rework the player mask logic to support more than 32 players");

	// HACKHACK -- In multiplayer, players touch this based on packet receipt.
	// So the players who send packets later aren't always hurt.  Keep track of
	// how much time has passed and whether or not you've touched that player
	if (g_pGameRules->IsMultiplayer())
	{
		if (pev->dmgtime > gpGlobals->time)
		{
			if (gpGlobals->time != pev->pain_finished)
			{ // too early to hurt again, and not same frame with a different entity
				if (pOther->IsPlayer())
				{
					int playerMask = 1 << (pOther->entindex() - 1);

					// If I've already touched this player (this time), then bail out
					if ((pev->impulse & playerMask) != 0)
						return;

					// Mark this player as touched
					// BUGBUG - There can be only 32 players!
					pev->impulse |= playerMask;
				}
				else
				{
					return;
				}
			}
		}
		else
		{
			// New clock, "un-touch" all players
			pev->impulse = 0;
			if (pOther->IsPlayer())
			{
				int playerMask = 1 << (pOther->entindex() - 1);

				// Mark this player as touched
				// BUGBUG - There can be only 32 players!
				pev->impulse |= playerMask;
			}
		}
	}
	else // Original code -- single player
	{
		if (pev->dmgtime > gpGlobals->time && gpGlobals->time != pev->pain_finished)
		{ // too early to hurt again, and not same frame with a different entity
			return;
		}
	}



	// If this is time_based damage (poison, radiation), override the pev->dmg with a
	// default for the given damage type.  Monsters only take time-based damage
	// while touching the trigger.  Player continues taking damage for a while after
	// leaving the trigger

	fldmg = pev->dmg * 0.5; // 0.5 seconds worth of damage, pev->dmg is damage/second


	// JAY: Cut this because it wasn't fully realized.  Damage is simpler now.
#if 0
	switch (m_bitsDamageInflict)
	{
	default: break;
	case DMG_POISON:		fldmg = POISON_DAMAGE/4; break;
	case DMG_NERVEGAS:		fldmg = NERVEGAS_DAMAGE/4; break;
	case DMG_RADIATION:		fldmg = RADIATION_DAMAGE/4; break;
	case DMG_PARALYZE:		fldmg = PARALYZE_DAMAGE/4; break; // UNDONE: cut this? should slow movement to 50%
	case DMG_ACID:			fldmg = ACID_DAMAGE/4; break;
	case DMG_SLOWBURN:		fldmg = SLOWBURN_DAMAGE/4; break;
	case DMG_SLOWFREEZE:	fldmg = SLOWFREEZE_DAMAGE/4; break;
	}
#endif

	if (fldmg < 0)
	{
		bool bApplyHeal = true;

		if (g_pGameRules->IsMultiplayer() && pOther->IsPlayer())
		{
			bApplyHeal = pOther->pev->deadflag == DEAD_NO;
		}

		if (bApplyHeal)
		{
			pOther->TakeHealth(-fldmg, m_bitsDamageInflict);
		}
	}
	else
	{
		// SoHL 1.5 - Use activator as attacker for frag attribution
		entvars_t* pevAttacker = pev;
		if (m_hActivator != 0)
			pevAttacker = m_hActivator->pev;
		pOther->TakeDamage(pev, pevAttacker, fldmg, m_bitsDamageInflict);
	}

	// Store pain time so we can get all of the other entities on this frame
	pev->pain_finished = gpGlobals->time;

	// Apply damage every half second
	pev->dmgtime = gpGlobals->time + 0.5; // half second delay until this trigger can hurt toucher again



	if (!FStringNull(pev->target))
	{
		// trigger has a target it wants to fire.
		if ((pev->spawnflags & SF_TRIGGER_HURT_CLIENTONLYFIRE) != 0)
		{
			// if the toucher isn't a client, don't fire the target!
			if (!pOther->IsPlayer())
			{
				return;
			}
		}

		SUB_UseTargets(pOther, USE_TOGGLE, 0);
		if ((pev->spawnflags & SF_TRIGGER_HURT_TARGETONCE) != 0)
			pev->target = 0;
	}
}


/*QUAKED trigger_multiple (.5 .5 .5) ? notouch
Variable sized repeatable trigger.  Must be targeted at one or more entities.
If "health" is set, the trigger must be killed to activate each time.
If "delay" is set, the trigger waits some time after activating before firing.
"wait" : Seconds between triggerings. (.2 default)
If notouch is set, the trigger is only fired by other entities, not by touching.
NOTOUCH has been obsoleted by trigger_relay!
sounds
1)      secret
2)      beep beep
3)      large switch
4)
NEW
if a trigger has a NETNAME, that NETNAME will become the TARGET of the triggered object.
*/
class CTriggerMultiple : public CBaseTrigger
{
public:
	void Spawn() override;
};

LINK_ENTITY_TO_CLASS(trigger_multiple, CTriggerMultiple);


void CTriggerMultiple::Spawn()
{
	if (m_flWait == 0)
		m_flWait = 0.2;

	InitTrigger();

	ASSERTSZ(pev->health == 0, "trigger_multiple with health");
	//	UTIL_SetOrigin(pev, pev->origin);
	//	SetModel( ENT(pev), STRING(pev->model) );
	//	if (pev->health > 0)
	//		{
	//		if (FBitSet(pev->spawnflags, SPAWNFLAG_NOTOUCH))
	//			ALERT(at_error, "trigger_multiple spawn: health and notouch don't make sense");
	//		pev->max_health = pev->health;
	//UNDONE: where to get pfnDie from?
	//		pev->pfnDie = multi_killed;
	//		pev->takedamage = DAMAGE_YES;
	//		pev->solid = SOLID_BBOX;
	//		UTIL_SetOrigin(pev, pev->origin);  // make sure it links into the world
	//		}
	//	else
	{
		SetTouch(&CTriggerMultiple::MultiTouch);
	}
}


/*QUAKED trigger_once (.5 .5 .5) ? notouch
Variable sized trigger. Triggers once, then removes itself.  You must set the key "target" to the name of another object in the level that has a matching
"targetname".  If "health" is set, the trigger must be killed to activate.
If notouch is set, the trigger is only fired by other entities, not by touching.
if "killtarget" is set, any objects that have a matching "target" will be removed when the trigger is fired.
if "angle" is set, the trigger will only fire when someone is facing the direction of the angle.  Use "360" for an angle of 0.
sounds
1)      secret
2)      beep beep
3)      large switch
4)
*/
class CTriggerOnce : public CTriggerMultiple
{
public:
	void Spawn() override;
};

LINK_ENTITY_TO_CLASS(trigger_once, CTriggerOnce);
void CTriggerOnce::Spawn()
{
	m_flWait = -1;

	CTriggerMultiple::Spawn();
}



void CBaseTrigger::MultiTouch(CBaseEntity* pOther)
{
	entvars_t* pevToucher;

	pevToucher = pOther->pev;

	// Only touch clients, monsters, or pushables (depending on flags)
	if (((pevToucher->flags & FL_CLIENT) != 0 && (pev->spawnflags & SF_TRIGGER_NOCLIENTS) == 0) ||
		((pevToucher->flags & FL_MONSTER) != 0 && (pev->spawnflags & SF_TRIGGER_ALLOWMONSTERS) != 0) ||
		(pev->spawnflags & SF_TRIGGER_PUSHABLES) != 0 && FClassnameIs(pevToucher, "func_pushable"))
	{

#if 0
		// if the trigger has an angles field, check player's facing direction
		if (pev->movedir != g_vecZero)
		{
			UTIL_MakeVectors( pevToucher->angles );
			if ( DotProduct( gpGlobals->v_forward, pev->movedir ) < 0 )
				return;         // not facing the right way
		}
#endif

		ActivateMultiTrigger(pOther);
	}
}


//
// the trigger was just touched/killed/used
// self.enemy should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
//
void CBaseTrigger::ActivateMultiTrigger(CBaseEntity* pActivator)
{
	if (pev->nextthink > gpGlobals->time)
		return; // still waiting for reset time

	if (!UTIL_IsMasterTriggered(m_sMaster, pActivator))
		return;

	if (FClassnameIs(pev, "trigger_secret"))
	{
		if (pev->enemy == NULL || !FClassnameIs(pev->enemy, "player"))
			return;
		gpGlobals->found_secrets++;
	}

	if (!FStringNull(pev->noise))
		EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);

	// don't trigger again until reset
	// pev->takedamage = DAMAGE_NO;

	m_hActivator = pActivator;
	SUB_UseTargets(m_hActivator, USE_TOGGLE, 0);

	if (!FStringNull(pev->message) && pActivator->IsPlayer())
	{
		UTIL_ShowMessage(STRING(pev->message), pActivator);
		//		CLIENT_PRINTF( ENT( pActivator->pev ), print_center, STRING(pev->message) );
	}

	if (m_flWait > 0)
	{
		SetThink(&CBaseTrigger::MultiWaitOver);
		SetNextThink(m_flWait);
	}
	else
	{
		// we can't just remove (self) here, because this is a touch function
		// called while C code is looping through area links...
		SetTouch(NULL);
		SetNextThink(0.1);
		SetThink(&CBaseTrigger::SUB_Remove);
	}
}


// the wait time has passed, so set back up for another activation
void CBaseTrigger::MultiWaitOver()
{
	//	if (pev->max_health)
	//		{
	//		pev->health		= pev->max_health;
	//		pev->takedamage	= DAMAGE_YES;
	//		pev->solid		= SOLID_BBOX;
	//		}
	SetThink(NULL);
}


// ========================= COUNTING TRIGGER =====================================

//
// GLOBALS ASSUMED SET:  g_eoActivator
//
void CBaseTrigger::CounterUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	m_cTriggersLeft--;
	m_hActivator = pActivator;

	if (m_cTriggersLeft < 0)
		return;

	bool fTellActivator =
		(m_hActivator != 0) &&
		FClassnameIs(m_hActivator->pev, "player") &&
		!FBitSet(pev->spawnflags, SPAWNFLAG_NOMESSAGE);
	if (m_cTriggersLeft != 0)
	{
		if (fTellActivator)
		{
			// UNDONE: I don't think we want these Quakesque messages
			switch (m_cTriggersLeft)
			{
			case 1:
				ALERT(at_console, "Only 1 more to go...");
				break;
			case 2:
				ALERT(at_console, "Only 2 more to go...");
				break;
			case 3:
				ALERT(at_console, "Only 3 more to go...");
				break;
			default:
				ALERT(at_console, "There are more to go...");
				break;
			}
		}
		return;
	}

	// !!!UNDONE: I don't think we want these Quakesque messages
	if (fTellActivator)
		ALERT(at_console, "Sequence completed!");

	ActivateMultiTrigger(m_hActivator);
}


/*QUAKED trigger_counter (.5 .5 .5) ? nomessage
Acts as an intermediary for an action that takes multiple inputs.
If nomessage is not set, it will print "1 more.. " etc when triggered and
"sequence complete" when finished.  After the counter has been triggered "cTriggersLeft"
times (default 2), it will fire all of it's targets and remove itself.
*/
class CTriggerCounter : public CBaseTrigger
{
public:
	void Spawn() override;
};
LINK_ENTITY_TO_CLASS(trigger_counter, CTriggerCounter);

void CTriggerCounter::Spawn()
{
	// By making the flWait be -1, this counter-trigger will disappear after it's activated
	// (but of course it needs cTriggersLeft "uses" before that happens).
	m_flWait = -1;

	if (m_cTriggersLeft == 0)
		m_cTriggersLeft = 2;
	SetUse(&CTriggerCounter::CounterUse);
}

// ====================== TRIGGER_CHANGELEVEL ================================

class CTriggerVolume : public CPointEntity // Derive from point entity so this doesn't move across levels
{
public:
	void Spawn() override;
};

LINK_ENTITY_TO_CLASS(trigger_transition, CTriggerVolume);

// Define space that travels across a level transition
void CTriggerVolume::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SetModel(ENT(pev), STRING(pev->model)); // set size and link into world
	pev->model = iStringNull;
	pev->modelindex = 0;
}


// Fires a target after level transition and then dies
class CFireAndDie : public CBaseDelay
{
public:
	void Spawn() override;
	void Precache() override;
	void Think() override;
	int ObjectCaps() override { return CBaseDelay::ObjectCaps() | FCAP_FORCE_TRANSITION; } // Always go across transitions
};
LINK_ENTITY_TO_CLASS(fireanddie, CFireAndDie);

void CFireAndDie::Spawn()
{
	pev->classname = MAKE_STRING("fireanddie");
	// Don't call Precache() - it should be called on restore
}


void CFireAndDie::Precache()
{
	// This gets called on restore
	SetNextThink(m_flDelay);
}


void CFireAndDie::Think()
{
	SUB_UseTargets(this, USE_TOGGLE, 0);
	UTIL_Remove(this);
}


#define SF_CHANGELEVEL_USEONLY 0x0002
class CChangeLevel : public CBaseTrigger
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT UseChangeLevel(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT TriggerChangeLevel();
	void EXPORT ExecuteChangeLevel();
	void EXPORT TouchChangeLevel(CBaseEntity* pOther);
	void ChangeLevelNow(CBaseEntity* pActivator);

	static edict_t* FindLandmark(const char* pLandmarkName);
	static int ChangeList(LEVELLIST* pLevelList, int maxList);
	static bool AddTransitionToList(LEVELLIST* pLevelList, int listCount, const char* pMapName, const char* pLandmarkName, edict_t* pentLandmark);
	static bool InTransitionVolume(CBaseEntity* pEntity, char* pVolumeName);

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	char m_szMapName[cchMapNameMost];	   // trigger_changelevel only:  next map
	char m_szLandmarkName[cchMapNameMost]; // trigger_changelevel only:  landmark on next map
	int m_changeTarget;
	float m_changeTargetDelay;
};
LINK_ENTITY_TO_CLASS(trigger_changelevel, CChangeLevel);

// Global Savedata for changelevel trigger
TYPEDESCRIPTION CChangeLevel::m_SaveData[] =
	{
		DEFINE_ARRAY(CChangeLevel, m_szMapName, FIELD_CHARACTER, cchMapNameMost),
		DEFINE_ARRAY(CChangeLevel, m_szLandmarkName, FIELD_CHARACTER, cchMapNameMost),
		DEFINE_FIELD(CChangeLevel, m_changeTarget, FIELD_STRING),
		DEFINE_FIELD(CChangeLevel, m_changeTargetDelay, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CChangeLevel, CBaseTrigger);

//
// Cache user-entity-field values until spawn is called.
//

bool CChangeLevel::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "map"))
	{
		if (strlen(pkvd->szValue) >= cchMapNameMost)
		{
			LOG_ERROR("Map name '%s' too long (32 chars)", pkvd->szValue);
			return true;
		}
		strcpy(m_szMapName, pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "landmark"))
	{
		if (strlen(pkvd->szValue) >= cchMapNameMost)
		{
			LOG_ERROR("Landmark name '%s' too long (32 chars)", pkvd->szValue);
			return true;
		}
		strcpy(m_szLandmarkName, pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "changetarget"))
	{
		m_changeTarget = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "changedelay"))
	{
		m_changeTargetDelay = atof(pkvd->szValue);
		return true;
	}

	return CBaseTrigger::KeyValue(pkvd);
}


/*QUAKED trigger_changelevel (0.5 0.5 0.5) ? NO_INTERMISSION
When the player touches this, he gets sent to the map listed in the "map" variable.  Unless the NO_INTERMISSION flag is set, the view will go to the info_intermission spot and display stats.
*/

void CChangeLevel::Spawn()
{
	if (FStrEq(m_szMapName, ""))
		ALERT(at_console, "a trigger_changelevel doesn't have a map");

	if (FStrEq(m_szLandmarkName, ""))
		ALERT(at_console, "trigger_changelevel to %s doesn't have a landmark", m_szMapName);

	if (0 == stricmp(m_szMapName, STRING(gpGlobals->mapname)))
	{
		LOG_ERROR("trigger_changelevel points to the current map (%s), which does not work", STRING(gpGlobals->mapname));
	}

	if (!FStringNull(pev->targetname))
	{
		SetUse(&CChangeLevel::UseChangeLevel);
	}
	InitTrigger();
	if ((pev->spawnflags & SF_CHANGELEVEL_USEONLY) == 0)
		SetTouch(&CChangeLevel::TouchChangeLevel);
	//	ALERT( at_console, "TRANSITION: %s (%s)\n", m_szMapName, m_szLandmarkName );
}


void CChangeLevel::ExecuteChangeLevel()
{
	MESSAGE_BEGIN(MSG_ALL, SVC_CDTRACK);
	WRITE_BYTE(3);
	WRITE_BYTE(3);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, SVC_INTERMISSION);
	MESSAGE_END();
}


FILE_GLOBAL char st_szNextMap[cchMapNameMost];
FILE_GLOBAL char st_szNextSpot[cchMapNameMost];

edict_t* CChangeLevel::FindLandmark(const char* pLandmarkName)
{
	edict_t* pentLandmark;

	pentLandmark = FIND_ENTITY_BY_STRING(NULL, "targetname", pLandmarkName);
	while (!FNullEnt(pentLandmark))
	{
		// Found the landmark
		if (FClassnameIs(pentLandmark, "info_landmark"))
			return pentLandmark;
		else
			pentLandmark = FIND_ENTITY_BY_STRING(pentLandmark, "targetname", pLandmarkName);
	}
	LOG_ERROR("Can't find landmark %s", pLandmarkName);
	return NULL;
}


//=========================================================
// CChangeLevel:: Use - allows level transitions to be
// triggered by buttons, etc.
//
//=========================================================
void CChangeLevel::UseChangeLevel(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	ChangeLevelNow(pActivator);
}

void CChangeLevel::ChangeLevelNow(CBaseEntity* pActivator)
{
	edict_t* pentLandmark;
	LEVELLIST levels[16];

	ASSERT(!FStrEq(m_szMapName, ""));

	// Don't work in deathmatch
	if (g_pGameRules->IsDeathmatch())
		return;

	// Some people are firing these multiple times in a frame, disable
	if (gpGlobals->time == pev->dmgtime)
		return;

	pev->dmgtime = gpGlobals->time;


	CBaseEntity* pPlayer = UTIL_GetLocalPlayer();
	if (!InTransitionVolume(pPlayer, m_szLandmarkName))
	{
		ALERT(at_aiconsole, "Player isn't in the transition volume %s, aborting\n", m_szLandmarkName);
		return;
	}

	// Create an entity to fire the changetarget
	if (!FStringNull(m_changeTarget))
	{
		CFireAndDie* pFireAndDie = GetClassPtr((CFireAndDie*)NULL);
		if (pFireAndDie)
		{
			// Set target and delay
			pFireAndDie->pev->target = m_changeTarget;
			pFireAndDie->m_flDelay = m_changeTargetDelay;
			pFireAndDie->pev->origin = pPlayer->pev->origin;
			// Call spawn
			DispatchSpawn(pFireAndDie->edict());
		}
	}
	// This object will get removed in the call to CHANGE_LEVEL, copy the params into "safe" memory
	strcpy(st_szNextMap, m_szMapName);

	m_hActivator = pActivator;
	SUB_UseTargets(pActivator, USE_TOGGLE, 0);
	st_szNextSpot[0] = 0; // Init landmark to NULL

	// look for a landmark entity
	pentLandmark = FindLandmark(m_szLandmarkName);
	if (!FNullEnt(pentLandmark))
	{
		strcpy(st_szNextSpot, m_szLandmarkName);
		gpGlobals->vecLandmarkOffset = VARS(pentLandmark)->origin;
	}
	//	ALERT( at_console, "Level touches %d levels\n", ChangeList( levels, 16 ) );
	ALERT(at_console, "CHANGE LEVEL: %s %s\n", st_szNextMap, st_szNextSpot);
	CHANGE_LEVEL(st_szNextMap, st_szNextSpot);
}

//
// GLOBALS ASSUMED SET:  st_szNextMap
//
void CChangeLevel::TouchChangeLevel(CBaseEntity* pOther)
{
	if (!FClassnameIs(pOther->pev, "player"))
		return;

	ChangeLevelNow(pOther);
}


// Add a transition to the list, but ignore duplicates
// (a designer may have placed multiple trigger_changelevels with the same landmark)
bool CChangeLevel::AddTransitionToList(LEVELLIST* pLevelList, int listCount, const char* pMapName, const char* pLandmarkName, edict_t* pentLandmark)
{
	int i;

	if (!pLevelList || !pMapName || !pLandmarkName || !pentLandmark)
		return false;

	for (i = 0; i < listCount; i++)
	{
		if (pLevelList[i].pentLandmark == pentLandmark && strcmp(pLevelList[i].mapName, pMapName) == 0)
			return false;
	}
	strcpy(pLevelList[listCount].mapName, pMapName);
	strcpy(pLevelList[listCount].landmarkName, pLandmarkName);
	pLevelList[listCount].pentLandmark = pentLandmark;
	pLevelList[listCount].vecLandmarkOrigin = VARS(pentLandmark)->origin;

	return true;
}

int BuildChangeList(LEVELLIST* pLevelList, int maxList)
{
	return CChangeLevel::ChangeList(pLevelList, maxList);
}


bool CChangeLevel::InTransitionVolume(CBaseEntity* pEntity, char* pVolumeName)
{
	edict_t* pentVolume;


	if ((pEntity->ObjectCaps() & FCAP_FORCE_TRANSITION) != 0)
		return true;

	// If you're following another entity, follow it through the transition (weapons follow the player)
	if (pEntity->pev->movetype == MOVETYPE_FOLLOW)
	{
		if (pEntity->pev->aiment != NULL)
			pEntity = CBaseEntity::Instance(pEntity->pev->aiment);
	}

	bool inVolume = true; // Unless we find a trigger_transition, everything is in the volume

	pentVolume = FIND_ENTITY_BY_TARGETNAME(NULL, pVolumeName);
	while (!FNullEnt(pentVolume))
	{
		CBaseEntity* pVolume = CBaseEntity::Instance(pentVolume);

		if (pVolume && FClassnameIs(pVolume->pev, "trigger_transition"))
		{
			if (pVolume->Intersects(pEntity)) // It touches one, it's in the volume
				return true;
			else
				inVolume = false; // Found a trigger_transition, but I don't intersect it -- if I don't find another, don't go!
		}
		pentVolume = FIND_ENTITY_BY_TARGETNAME(pentVolume, pVolumeName);
	}

	return inVolume;
}


// We can only ever move 512 entities across a transition
#define MAX_ENTITY 512

// This has grown into a complicated beast
// Can we make this more elegant?
// This builds the list of all transitions on this level and which entities are in their PVS's and can / should
// be moved across.
int CChangeLevel::ChangeList(LEVELLIST* pLevelList, int maxList)
{
	edict_t *pentChangelevel, *pentLandmark;
	int i, count;

	count = 0;

	// Find all of the possible level changes on this BSP
	pentChangelevel = FIND_ENTITY_BY_STRING(NULL, "classname", "trigger_changelevel");
	if (FNullEnt(pentChangelevel))
		return 0;
	while (!FNullEnt(pentChangelevel))
	{
		CChangeLevel* pTrigger;

		pTrigger = GetClassPtr((CChangeLevel*)VARS(pentChangelevel));
		if (pTrigger)
		{
			// Find the corresponding landmark
			pentLandmark = FindLandmark(pTrigger->m_szLandmarkName);
			if (pentLandmark)
			{
				// Build a list of unique transitions
				if (AddTransitionToList(pLevelList, count, pTrigger->m_szMapName, pTrigger->m_szLandmarkName, pentLandmark))
				{
					count++;
					if (count >= maxList) // FULL!!
						break;
				}
			}
		}
		pentChangelevel = FIND_ENTITY_BY_STRING(pentChangelevel, "classname", "trigger_changelevel");
	}

	//Token table is null at this point, so don't use CSaveRestoreBuffer::IsValidSaveRestoreData here.
	if (auto pSaveData = reinterpret_cast<SAVERESTOREDATA*>(gpGlobals->pSaveData);
		nullptr != pSaveData && pSaveData->pTable)
	{
		CSave saveHelper(*pSaveData);

		for (i = 0; i < count; i++)
		{
			int j, entityCount = 0;
			CBaseEntity* pEntList[MAX_ENTITY];
			int entityFlags[MAX_ENTITY];

			// Follow the linked list of entities in the PVS of the transition landmark
			edict_t* pent = UTIL_EntitiesInPVS(pLevelList[i].pentLandmark);

			// Build a list of valid entities in this linked list (we're going to use pent->v.chain again)
			while (!FNullEnt(pent))
			{
				CBaseEntity* pEntity = CBaseEntity::Instance(pent);
				if (pEntity)
				{
					//					ALERT( at_console, "Trying %s\n", STRING(pEntity->pev->classname) );
					int caps = pEntity->ObjectCaps();
					if ((caps & FCAP_DONT_SAVE) == 0)
					{
						int flags = 0;

						// If this entity can be moved or is global, mark it
						if ((caps & FCAP_ACROSS_TRANSITION) != 0)
							flags |= FENTTABLE_MOVEABLE;
						if (!FStringNull(pEntity->pev->globalname) && !pEntity->IsDormant())
							flags |= FENTTABLE_GLOBAL;
						if (0 != flags)
						{
							pEntList[entityCount] = pEntity;
							entityFlags[entityCount] = flags;
							entityCount++;
							if (entityCount > MAX_ENTITY)
								LOG_ERROR("Too many entities across a transition!");
						}
						//						else
						//							ALERT( at_console, "Failed %s\n", STRING(pEntity->pev->classname) );
					}
					//					else
					//						ALERT( at_console, "DON'T SAVE %s\n", STRING(pEntity->pev->classname) );
				}
				pent = pent->v.chain;
			}

			for (j = 0; j < entityCount; j++)
			{
				// Check to make sure the entity isn't screened out by a trigger_transition
				if (0 != entityFlags[j] && InTransitionVolume(pEntList[j], pLevelList[i].landmarkName))
				{
					// Mark entity table with 1<<i
					int index = saveHelper.EntityIndex(pEntList[j]);
					// Flag it with the level number
					saveHelper.EntityFlagsSet(index, entityFlags[j] | (1 << i));
				}
				//				else
				//					ALERT( at_console, "Screened out %s\n", STRING(pEntList[j]->pev->classname) );
			}
		}
	}

	return count;
}

/*
go to the next level for deathmatch
only called if a time or frag limit has expired
*/
void NextLevel()
{
	edict_t* pent;
	CChangeLevel* pChange;

	// find a trigger_changelevel
	pent = FIND_ENTITY_BY_CLASSNAME(NULL, "trigger_changelevel");

	// go back to start if no trigger_changelevel
	if (FNullEnt(pent))
	{
		gpGlobals->mapname = ALLOC_STRING("start");
		pChange = GetClassPtr((CChangeLevel*)NULL);
		strcpy(pChange->m_szMapName, "start");
	}
	else
		pChange = GetClassPtr((CChangeLevel*)VARS(pent));

	strcpy(st_szNextMap, pChange->m_szMapName);
	g_fGameOver = true;

	if (pChange->pev->nextthink < gpGlobals->time)
	{
		pChange->SetThink(&CChangeLevel::ExecuteChangeLevel);
		pChange->SetNextThink(0.1);
	}
}


// ============================== LADDER =======================================

class CLadder : public CBaseTrigger
{
public:
	void Spawn() override;
	void Precache() override;
};
LINK_ENTITY_TO_CLASS(func_ladder, CLadder);

//=========================================================
// func_ladder - makes an area vertically negotiable
//=========================================================
void CLadder::Precache()
{
	// Do all of this in here because we need to 'convert' old saved games
	pev->solid = SOLID_NOT;
	pev->skin = CONTENTS_LADDER;
	if (CVAR_GET_FLOAT("showtriggers") == 0)
	{
		pev->rendermode = kRenderTransTexture;
		pev->renderamt = 0;
	}
	pev->effects &= ~EF_NODRAW;
}


void CLadder::Spawn()
{
	Precache();

	SetModel(ENT(pev), STRING(pev->model)); // set size and link into world
	pev->movetype = MOVETYPE_PUSH;
}


// ========================== A TRIGGER THAT PUSHES YOU ===============================

class CTriggerPush : public CBaseTrigger
{
public:
	void Spawn() override;
	void Touch(CBaseEntity* pOther) override;
};
LINK_ENTITY_TO_CLASS(trigger_push, CTriggerPush);

/*QUAKED trigger_push (.5 .5 .5) ? TRIG_PUSH_ONCE
Pushes the player
*/

void CTriggerPush::Spawn()
{
	if (pev->angles == g_vecZero)
		pev->angles.y = 360;
	InitTrigger();

	if (pev->speed == 0)
		pev->speed = 100;

	if (FBitSet(pev->spawnflags, SF_TRIGGER_PUSH_START_OFF)) // if flagged to Start Turned Off, make trigger nonsolid.
		pev->solid = SOLID_NOT;

	SetUse(&CTriggerPush::ToggleUse);

	UTIL_SetOrigin(pev, pev->origin); // Link into the list
}


void CTriggerPush::Touch(CBaseEntity* pOther)
{
	entvars_t* pevToucher = pOther->pev;

	// UNDONE: Is there a better way than health to detect things that have physics? (clients/monsters)
	switch (pevToucher->movetype)
	{
	case MOVETYPE_NONE:
	case MOVETYPE_PUSH:
	case MOVETYPE_NOCLIP:
	case MOVETYPE_FOLLOW:
		return;
	}

	if (pevToucher->solid != SOLID_NOT && pevToucher->solid != SOLID_BSP)
	{
		// Instant trigger, just transfer velocity and remove
		if (FBitSet(pev->spawnflags, SF_TRIG_PUSH_ONCE))
		{
			pevToucher->velocity = pevToucher->velocity + (pev->speed * pev->movedir);
			if (pevToucher->velocity.z > 0)
				pevToucher->flags &= ~FL_ONGROUND;
			UTIL_Remove(this);
		}
		else
		{ // Push field, transfer to base velocity
			Vector vecPush = (pev->speed * pev->movedir);
			if ((pevToucher->flags & FL_BASEVELOCITY) != 0)
				vecPush = vecPush + pevToucher->basevelocity;

			pevToucher->basevelocity = vecPush;

			pevToucher->flags |= FL_BASEVELOCITY;
			//			ALERT( at_console, "Vel %f, base %f\n", pevToucher->velocity.z, pevToucher->basevelocity.z );
		}
	}
}


//======================================
// teleport trigger
//
//

void CBaseTrigger::TeleportTouch(CBaseEntity* pOther)
{
	entvars_t* pevToucher = pOther->pev;
	edict_t* pentTarget = NULL;

	// Only teleport monsters or clients
	if (!FBitSet(pevToucher->flags, FL_CLIENT | FL_MONSTER))
		return;

	if (!UTIL_IsMasterTriggered(m_sMaster, pOther))
		return;

	if ((pev->spawnflags & SF_TRIGGER_ALLOWMONSTERS) == 0)
	{ // no monsters allowed!
		if (FBitSet(pevToucher->flags, FL_MONSTER))
		{
			return;
		}
	}

	if ((pev->spawnflags & SF_TRIGGER_NOCLIENTS) != 0)
	{ // no clients allowed
		if (pOther->IsPlayer())
		{
			return;
		}
	}

	pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(pev->target));
	if (FNullEnt(pentTarget))
		return;

	Vector tmp = VARS(pentTarget)->origin;

	if (pOther->IsPlayer())
	{
		tmp.z -= pOther->pev->mins.z; // make origin adjustments in case the teleportee is a player. (origin in center, not at feet)
	}

	tmp.z++;

	pevToucher->flags &= ~FL_ONGROUND;

	UTIL_SetOrigin(pevToucher, tmp);

	pevToucher->angles = pentTarget->v.angles;

	if (pOther->IsPlayer())
	{
		pevToucher->v_angle = pentTarget->v.angles;
	}

	pevToucher->fixangle = 1;
	pevToucher->velocity = pevToucher->basevelocity = g_vecZero;
}


class CTriggerTeleport : public CBaseTrigger
{
public:
	void Spawn() override;
};
LINK_ENTITY_TO_CLASS(trigger_teleport, CTriggerTeleport);

void CTriggerTeleport::Spawn()
{
	InitTrigger();

	SetTouch(&CTriggerTeleport::TeleportTouch);
}


LINK_ENTITY_TO_CLASS(info_teleport_destination, CPointEntity);



class CTriggerSave : public CBaseTrigger
{
public:
	void Spawn() override;
	void EXPORT SaveTouch(CBaseEntity* pOther);
};
LINK_ENTITY_TO_CLASS(trigger_autosave, CTriggerSave);

void CTriggerSave::Spawn()
{
	if (g_pGameRules->IsDeathmatch())
	{
		REMOVE_ENTITY(ENT(pev));
		return;
	}

	InitTrigger();
	SetTouch(&CTriggerSave::SaveTouch);
}

void CTriggerSave::SaveTouch(CBaseEntity* pOther)
{
	if (!UTIL_IsMasterTriggered(m_sMaster, pOther))
		return;

	// Only save on clients
	if (!pOther->IsPlayer())
		return;

	SetTouch(NULL);
	UTIL_Remove(this);
	SERVER_COMMAND("autosave\n");
}

#define SF_ENDSECTION_USEONLY 0x0001

class CTriggerEndSection : public CBaseTrigger
{
public:
	void Spawn() override;
	void EXPORT EndSectionTouch(CBaseEntity* pOther);
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT EndSectionUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
};
LINK_ENTITY_TO_CLASS(trigger_endsection, CTriggerEndSection);


void CTriggerEndSection::EndSectionUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// Only save on clients
	if (pActivator && !pActivator->IsNetClient())
		return;

	SetUse(NULL);

	if (!FStringNull(pev->message))
	{
		g_engfuncs.pfnEndSection(STRING(pev->message));
	}
	UTIL_Remove(this);
}

void CTriggerEndSection::Spawn()
{
	if (g_pGameRules->IsDeathmatch())
	{
		REMOVE_ENTITY(ENT(pev));
		return;
	}

	InitTrigger();

	SetUse(&CTriggerEndSection::EndSectionUse);
	// If it is a "use only" trigger, then don't set the touch function.
	if ((pev->spawnflags & SF_ENDSECTION_USEONLY) == 0)
		SetTouch(&CTriggerEndSection::EndSectionTouch);
}

void CTriggerEndSection::EndSectionTouch(CBaseEntity* pOther)
{
	// Only save on clients
	if (!pOther->IsNetClient())
		return;

	SetTouch(NULL);

	if (!FStringNull(pev->message))
	{
		g_engfuncs.pfnEndSection(STRING(pev->message));
	}
	UTIL_Remove(this);
}

bool CTriggerEndSection::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "section"))
	{
		//		m_iszSectionName = ALLOC_STRING( pkvd->szValue );
		// Store this in message so we don't have to write save/restore for this ent
		pev->message = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CBaseTrigger::KeyValue(pkvd);
}


class CTriggerGravity : public CBaseTrigger
{
public:
	void Spawn() override;
	void EXPORT GravityTouch(CBaseEntity* pOther);
};
LINK_ENTITY_TO_CLASS(trigger_gravity, CTriggerGravity);

void CTriggerGravity::Spawn()
{
	InitTrigger();
	SetTouch(&CTriggerGravity::GravityTouch);
}

void CTriggerGravity::GravityTouch(CBaseEntity* pOther)
{
	// Only save on clients
	if (!pOther->IsPlayer())
		return;

	pOther->pev->gravity = pev->gravity;
}







// this is a really bad idea.
class CTriggerChangeTarget : public CBaseDelay
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	int ObjectCaps() override { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

private:
	int m_iszNewTarget;
};
LINK_ENTITY_TO_CLASS(trigger_changetarget, CTriggerChangeTarget);

TYPEDESCRIPTION CTriggerChangeTarget::m_SaveData[] =
	{
		DEFINE_FIELD(CTriggerChangeTarget, m_iszNewTarget, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CTriggerChangeTarget, CBaseDelay);

bool CTriggerChangeTarget::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszNewTarget"))
	{
		m_iszNewTarget = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CBaseDelay::KeyValue(pkvd);
}

void CTriggerChangeTarget::Spawn()
{
}


void CTriggerChangeTarget::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity* pTarget = UTIL_FindEntityByString(NULL, "targetname", STRING(pev->target));

	if (pTarget)
	{
		pTarget->pev->target = m_iszNewTarget;
		CBaseMonster* pMonster = pTarget->MyMonsterPointer();
		if (pMonster)
		{
			pMonster->m_pGoalEnt = NULL;
		}
	}
}




#define SF_CAMERA_PLAYER_POSITION 1
#define SF_CAMERA_PLAYER_TARGET 2
#define SF_CAMERA_PLAYER_TAKECONTROL 4

class CTriggerCamera : public CBaseDelay
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void EXPORT FollowTarget();
	void Move();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	static TYPEDESCRIPTION m_SaveData[];

	EHANDLE m_hPlayer;
	EHANDLE m_hTarget;
	CBaseEntity* m_pentPath;
	int m_sPath;
	float m_flWait;
	float m_flReturnTime;
	float m_flStopTime;
	float m_moveDistance;
	float m_targetSpeed;
	float m_initialSpeed;
	float m_acceleration;
	float m_deceleration;
	bool m_state;
};
LINK_ENTITY_TO_CLASS(trigger_camera, CTriggerCamera);

// Global Savedata for changelevel friction modifier
TYPEDESCRIPTION CTriggerCamera::m_SaveData[] =
	{
		DEFINE_FIELD(CTriggerCamera, m_hPlayer, FIELD_EHANDLE),
		DEFINE_FIELD(CTriggerCamera, m_hTarget, FIELD_EHANDLE),
		DEFINE_FIELD(CTriggerCamera, m_pentPath, FIELD_CLASSPTR),
		DEFINE_FIELD(CTriggerCamera, m_sPath, FIELD_STRING),
		DEFINE_FIELD(CTriggerCamera, m_flWait, FIELD_FLOAT),
		DEFINE_FIELD(CTriggerCamera, m_flReturnTime, FIELD_TIME),
		DEFINE_FIELD(CTriggerCamera, m_flStopTime, FIELD_TIME),
		DEFINE_FIELD(CTriggerCamera, m_moveDistance, FIELD_FLOAT),
		DEFINE_FIELD(CTriggerCamera, m_targetSpeed, FIELD_FLOAT),
		DEFINE_FIELD(CTriggerCamera, m_initialSpeed, FIELD_FLOAT),
		DEFINE_FIELD(CTriggerCamera, m_acceleration, FIELD_FLOAT),
		DEFINE_FIELD(CTriggerCamera, m_deceleration, FIELD_FLOAT),
		DEFINE_FIELD(CTriggerCamera, m_state, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CTriggerCamera, CBaseDelay);

void CTriggerCamera::Spawn()
{
	pev->movetype = MOVETYPE_NOCLIP;
	pev->solid = SOLID_NOT; // Remove model & collisions
	pev->renderamt = 0;		// The engine won't draw this model if this is set to 0 and blending is on
	pev->rendermode = kRenderTransTexture;

	m_initialSpeed = pev->speed;
	if (m_acceleration == 0)
		m_acceleration = 500;
	if (m_deceleration == 0)
		m_deceleration = 500;
}


bool CTriggerCamera::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "moveto"))
	{
		m_sPath = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "acceleration"))
	{
		m_acceleration = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "deceleration"))
	{
		m_deceleration = atof(pkvd->szValue);
		return true;
	}

	return CBaseDelay::KeyValue(pkvd);
}



void CTriggerCamera::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType, m_state))
		return;

	// Toggle state
	m_state = !m_state;
	if (!m_state)
	{
		m_flReturnTime = gpGlobals->time;
		return;
	}
	if (!pActivator || !pActivator->IsPlayer())
	{
		pActivator = UTIL_GetLocalPlayer();

		if (!pActivator)
		{
			return;
		}
	}

	auto player = static_cast<CBasePlayer*>(pActivator);

	m_hPlayer = pActivator;

	m_flReturnTime = gpGlobals->time + m_flWait;
	pev->speed = m_initialSpeed;
	m_targetSpeed = m_initialSpeed;

	if (FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TARGET))
	{
		m_hTarget = m_hPlayer;
	}
	else
	{
		m_hTarget = GetNextTarget();
	}

	// Nothing to look at!
	if (m_hTarget == NULL)
	{
		return;
	}


	if (FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TAKECONTROL))
	{
		player->EnableControl(false);
	}

	if (!FStringNull(m_sPath))
	{
		m_pentPath = Instance(FIND_ENTITY_BY_TARGETNAME(NULL, STRING(m_sPath)));
	}
	else
	{
		m_pentPath = NULL;
	}

	m_flStopTime = gpGlobals->time;
	if (m_pentPath)
	{
		if (m_pentPath->pev->speed != 0)
			m_targetSpeed = m_pentPath->pev->speed;

		m_flStopTime += m_pentPath->GetDelay();
	}

	// copy over player information
	if (FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_POSITION))
	{
		UTIL_SetOrigin(pev, pActivator->pev->origin + pActivator->pev->view_ofs);
		pev->angles.x = -pActivator->pev->angles.x;
		pev->angles.y = pActivator->pev->angles.y;
		pev->angles.z = 0;
		pev->velocity = pActivator->pev->velocity;
	}
	else
	{
		pev->velocity = Vector(0, 0, 0);
	}

	SET_VIEW(pActivator->edict(), edict());

	player->m_hViewEntity = this;

	SetModel(ENT(pev), STRING(pActivator->pev->model));

	// follow the player down
	SetThink(&CTriggerCamera::FollowTarget);
	SetNextThink(0);

	m_moveDistance = 0;
	Move();
}


void CTriggerCamera::FollowTarget()
{
	if (m_hPlayer == NULL)
		return;

	if (m_hTarget == NULL || m_flReturnTime < gpGlobals->time)
	{
		auto player = static_cast<CBasePlayer*>(static_cast<CBaseEntity*>(m_hPlayer));

		if (player->IsAlive())
		{
			SET_VIEW(player->edict(), player->edict());
			player->EnableControl(true);
		}

		player->m_hViewEntity = nullptr;
		player->m_bResetViewEntity = false;

		SUB_UseTargets(this, USE_TOGGLE, 0);
		pev->avelocity = Vector(0, 0, 0);
		m_state = false;
		return;
	}

	Vector vecGoal = UTIL_VecToAngles(m_hTarget->pev->origin - pev->origin);
	vecGoal.x = -vecGoal.x;

	if (pev->angles.y > 360)
		pev->angles.y -= 360;

	if (pev->angles.y < 0)
		pev->angles.y += 360;

	float dx = vecGoal.x - pev->angles.x;
	float dy = vecGoal.y - pev->angles.y;

	if (dx < -180)
		dx += 360;
	if (dx > 180)
		dx = dx - 360;

	if (dy < -180)
		dy += 360;
	if (dy > 180)
		dy = dy - 360;

	pev->avelocity.x = dx * 40 * 0.01;
	pev->avelocity.y = dy * 40 * 0.01;


	if (!(FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TAKECONTROL)))
	{
		pev->velocity = pev->velocity * 0.8;
		if (pev->velocity.Length() < 10.0)
			pev->velocity = g_vecZero;
	}

	SetNextThink(0);

	Move();
}

void CTriggerCamera::Move()
{
	// Not moving on a path, return
	if (!m_pentPath)
		return;

	// Subtract movement from the previous frame
	m_moveDistance -= pev->speed * gpGlobals->frametime;

	// Have we moved enough to reach the target?
	if (m_moveDistance <= 0)
	{
		// Fire the passtarget if there is one
		if (!FStringNull(m_pentPath->pev->message))
		{
			FireTargets(STRING(m_pentPath->pev->message), this, this, USE_TOGGLE, 0);
			if (FBitSet(m_pentPath->pev->spawnflags, SF_CORNER_FIREONCE))
				m_pentPath->pev->message = 0;
		}
		// Time to go to the next target
		m_pentPath = m_pentPath->GetNextTarget();

		// Set up next corner
		if (!m_pentPath)
		{
			pev->velocity = g_vecZero;
		}
		else
		{
			if (m_pentPath->pev->speed != 0)
				m_targetSpeed = m_pentPath->pev->speed;

			Vector delta = m_pentPath->pev->origin - pev->origin;
			m_moveDistance = delta.Length();
			pev->movedir = delta.Normalize();
			m_flStopTime = gpGlobals->time + m_pentPath->GetDelay();
		}
	}

	if (m_flStopTime > gpGlobals->time)
		pev->speed = UTIL_Approach(0, pev->speed, m_deceleration * gpGlobals->frametime);
	else
		pev->speed = UTIL_Approach(m_targetSpeed, pev->speed, m_acceleration * gpGlobals->frametime);

	float fraction = 2 * gpGlobals->frametime;
	pev->velocity = ((pev->movedir * pev->speed) * fraction) + (pev->velocity * (1 - fraction));
}

class CTriggerPlayerFreeze : public CBaseDelay
{
public:
	void Spawn() override;

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

public:
	bool m_bUnFrozen;
};

LINK_ENTITY_TO_CLASS(trigger_playerfreeze, CTriggerPlayerFreeze);
LINK_ENTITY_TO_CLASS(player_freeze, CTriggerPlayerFreeze);

void CTriggerPlayerFreeze::Spawn()
{
	if (g_pGameRules->IsDeathmatch())
		REMOVE_ENTITY(edict());
	else
		m_bUnFrozen = true;
}

void CTriggerPlayerFreeze::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// LRC - ShouldToggle: respect USE_ON / USE_OFF / USE_TOGGLE
	if (!ShouldToggle(useType, m_bUnFrozen))
		return;

	m_bUnFrozen = !m_bUnFrozen;

	//TODO: not made for multiplayer
	auto pPlayer = static_cast<CBasePlayer*>(UTIL_GetLocalPlayer());

	if (!pPlayer)
	{
		return;
	}

	pPlayer->EnableControl(m_bUnFrozen);
}

/**
*	@brief Kills anything that touches it without gibbing it
*/
class CTriggerKillNoGib : public CBaseTrigger
{
public:
	void Spawn() override;

	void EXPORT KillTouch(CBaseEntity* pOther);
};

LINK_ENTITY_TO_CLASS(trigger_kill_nogib, CTriggerKillNoGib);

void CTriggerKillNoGib::Spawn()
{
	InitTrigger();

	SetTouch(&CTriggerKillNoGib::KillTouch);
	SetUse(nullptr);
}

void CTriggerKillNoGib::KillTouch(CBaseEntity* pOther)
{
	if (pOther->pev->takedamage != DAMAGE_NO)
		pOther->TakeDamage(pev, pOther->pev, 500000, DMG_NEVERGIB);
}

class CTriggerXenReturn : public CBaseTrigger
{
public:
	using BaseClass = CBaseTrigger;

	void Spawn() override;

	void EXPORT ReturnTouch(CBaseEntity* pOther);
};

LINK_ENTITY_TO_CLASS(trigger_xen_return, CTriggerXenReturn);

LINK_ENTITY_TO_CLASS(info_displacer_earth_target, CPointEntity);
LINK_ENTITY_TO_CLASS(info_displacer_xen_target, CPointEntity);

void CTriggerXenReturn::Spawn()
{
	InitTrigger();

	SetTouch(&CTriggerXenReturn::ReturnTouch);
	SetUse(nullptr);
}

void CTriggerXenReturn::ReturnTouch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
		return;

	auto pPlayer = static_cast<CBasePlayer*>(pOther);

	float flDist = 8192;

	CBaseEntity* pTarget = nullptr;

	//Find the earth target nearest to the player's original location.
	for (auto pDestination : UTIL_FindEntitiesByClassname("info_displacer_earth_target"))
	{
		const float flThisDist = (pPlayer->m_DisplacerReturn - pDestination->pev->origin).Length();

		if (flDist > flThisDist)
		{
			pTarget = pDestination;

			flDist = flThisDist;
		}
	}

	if (pTarget && !FNullEnt(pTarget->pev))
	{
		pPlayer->pev->flags &= ~FL_SKIPLOCALHOST;

		auto vecDest = pTarget->pev->origin;

		vecDest.z -= pPlayer->pev->mins.z;
		vecDest.z += 1;

		UTIL_SetOrigin(pPlayer->pev, vecDest);

		pPlayer->pev->angles = pTarget->pev->angles;
		pPlayer->pev->v_angle = pTarget->pev->angles;
		pPlayer->pev->fixangle = 1;

		pPlayer->pev->basevelocity = g_vecZero;
		pPlayer->pev->velocity = g_vecZero;

		pPlayer->pev->gravity = 1.0;

		pPlayer->m_SndRoomtype = pPlayer->m_DisplacerSndRoomtype;

		EMIT_SOUND(pPlayer->edict(), CHAN_WEAPON, "weapons/displacer_self.wav", RANDOM_FLOAT(0.8, 0.9), ATTN_NORM);
	}
}

const auto SF_GENEWORM_HIT_TARGET_ONCE = 1 << 0;
const auto SF_GENEWORM_HIT_START_OFF = 1 << 1;
const auto SF_GENEWORM_HIT_NO_CLIENTS = 1 << 3;
const auto SF_GENEWORM_HIT_FIRE_CLIENT_ONLY = 1 << 4;
const auto SF_GENEWORM_HIT_TOUCH_CLIENT_ONLY = 1 << 5;

class COFTriggerGeneWormHit : public CBaseTrigger
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void Precache() override;
	void Spawn() override;

	void EXPORT GeneWormHitTouch(CBaseEntity* pOther);

	static const char* pAttackSounds[];

	float m_flLastDamageTime;
};

TYPEDESCRIPTION COFTriggerGeneWormHit::m_SaveData[] =
	{
		DEFINE_FIELD(COFTriggerGeneWormHit, m_flLastDamageTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(COFTriggerGeneWormHit, CBaseTrigger);

const char* COFTriggerGeneWormHit::pAttackSounds[] =
	{
		"zombie/claw_strike1.wav",
		"zombie/claw_strike2.wav",
		"zombie/claw_strike3.wav"};

LINK_ENTITY_TO_CLASS(trigger_geneworm_hit, COFTriggerGeneWormHit);

void COFTriggerGeneWormHit::Precache()
{
	PRECACHE_SOUND_ARRAY(pAttackSounds);
}

void COFTriggerGeneWormHit::Spawn()
{
	Precache();

	InitTrigger();

	SetTouch(&COFTriggerGeneWormHit::GeneWormHitTouch);

	if (!FStringNull(pev->targetname))
	{
		SetUse(&COFTriggerGeneWormHit::ToggleUse);
	}
	else
	{
		SetUse(nullptr);
	}

	if ((pev->spawnflags & SF_GENEWORM_HIT_START_OFF) != 0)
	{
		pev->solid = SOLID_NOT;
	}

	UTIL_SetOrigin(pev, pev->origin);

	pev->dmg = gSkillData.geneWormDmgHit;
	m_flLastDamageTime = gpGlobals->time;
}

void COFTriggerGeneWormHit::GeneWormHitTouch(CBaseEntity* pOther)
{
	if (gpGlobals->time - m_flLastDamageTime >= 2 && pOther->pev->takedamage != DAMAGE_NO)
	{
		if ((pev->spawnflags & SF_GENEWORM_HIT_TOUCH_CLIENT_ONLY) != 0)
		{
			if (!pOther->IsPlayer())
				return;
		}

		if ((pev->spawnflags & SF_GENEWORM_HIT_NO_CLIENTS) == 0 || !pOther->IsPlayer())
		{
			if (!g_pGameRules->IsMultiplayer())
			{
				if (pev->dmgtime > gpGlobals->time && gpGlobals->time != pev->pain_finished)
					return;
			}
			else if (pev->dmgtime <= gpGlobals->time)
			{
				pev->impulse = 0;
				if (pOther->IsPlayer())
					pev->impulse |= 1 << (pOther->entindex() - 1);
			}
			else if (gpGlobals->time != pev->pain_finished)
			{
				if (!pOther->IsPlayer())
					return;

				const auto playerBit = 1 << (pOther->entindex() - 1);

				if ((pev->impulse & playerBit) != 0)
					return;

				pev->impulse |= playerBit;
			}

			pOther->TakeDamage(pev, pev, pev->dmg, DMG_CRUSH);

			EMIT_SOUND_DYN(pOther->edict(), CHAN_BODY, pAttackSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackSounds) - 1)], VOL_NORM, 0.1, 0, RANDOM_LONG(-5, 5) + 100);

			pev->pain_finished = gpGlobals->time;
			m_flLastDamageTime = gpGlobals->time;

			if (!FStringNull(pev->target) &&
				((pev->spawnflags & SF_GENEWORM_HIT_FIRE_CLIENT_ONLY) == 0 || pOther->IsPlayer()))
			{
				SUB_UseTargets(pOther, USE_TOGGLE, 0);

				if ((pev->spawnflags & SF_GENEWORM_HIT_TARGET_ONCE) != 0)
					pev->target = iStringNull;
			}
		}
	}
}

class CTriggerCTFGeneric : public CBaseTrigger
{
public:
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	void Spawn() override;

	void Touch(CBaseEntity* pOther) override;

	bool KeyValue(KeyValueData* pkvd) override;

	USE_TYPE triggerType;
	CTFTeam team_no;
	float trigger_delay;
	float m_flTriggerDelayTime;
	int score;
	int team_score;
};

LINK_ENTITY_TO_CLASS(trigger_ctfgeneric, CTriggerCTFGeneric);

void CTriggerCTFGeneric::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	Touch(nullptr);
}

void CTriggerCTFGeneric::Spawn()
{
	InitTrigger();

	m_flTriggerDelayTime = 0;
}

void CTriggerCTFGeneric::Touch(CBaseEntity* pOther)
{
	if (m_flTriggerDelayTime <= gpGlobals->time)
	{
		CBasePlayer* pOtherPlayer = nullptr;

		if (pOther)
		{
			if (0 != score || !pOther->IsPlayer())
			{
				return;
			}

			pOtherPlayer = static_cast<CBasePlayer*>(pOther);

			if (team_no != CTFTeam::None && team_no != pOtherPlayer->m_iTeamNum)
			{
				return;
			}
		}

		SUB_UseTargets(this, triggerType, 0);

		if (0 != team_score && team_no > CTFTeam::None && static_cast<int>(team_no) <= MaxTeams)
			teamscores[static_cast<int>(team_no) - 1] += team_score;

		if (pOtherPlayer && score != 0)
		{
			pOtherPlayer->m_iCTFScore += score;
			pOtherPlayer->m_iOffense += score;
			g_engfuncs.pfnMessageBegin(MSG_ALL, gmsgCTFScore, 0, 0);
			g_engfuncs.pfnWriteByte(pOtherPlayer->entindex());
			g_engfuncs.pfnWriteByte(pOtherPlayer->m_iCTFScore);
			g_engfuncs.pfnMessageEnd();

			g_engfuncs.pfnMessageBegin(MSG_ALL, gmsgScoreInfo, 0, 0);
			g_engfuncs.pfnWriteByte(pOtherPlayer->entindex());
			g_engfuncs.pfnWriteShort(pev->frags);
			g_engfuncs.pfnWriteShort(pOtherPlayer->m_iDeaths);
			g_engfuncs.pfnMessageEnd();

			UTIL_LogPrintf(
				"\"%s<%i><%u><%s>\" triggered \"%s\"\n",
				STRING(pOtherPlayer->pev->targetname),
				g_engfuncs.pfnGetPlayerUserId(pOtherPlayer->edict()),
				g_engfuncs.pfnGetPlayerWONId(pOtherPlayer->edict()),
				GetTeamName(pOtherPlayer->edict()),
				STRING(pev->targetname));
		}

		if (0 != team_score)
		{
			//TOOD: not sure why this check is here since pev must be valid if the entity exists
			if (!pOther && 0 == score && pev)
			{
				UTIL_LogPrintf((char*)"World triggered \"%s\"\n", STRING(pev->targetname));
			}

			DisplayTeamFlags(nullptr);
		}

		m_flTriggerDelayTime = gpGlobals->time + trigger_delay;
	}
}

bool CTriggerCTFGeneric::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq("team_no", pkvd->szKeyName))
	{
		team_no = static_cast<CTFTeam>(atoi(pkvd->szValue));
		return true;
	}
	else if (FStrEq("trigger_delay", pkvd->szKeyName))
	{
		trigger_delay = atof(pkvd->szValue);

		if (trigger_delay == 0)
			trigger_delay = 5;

		return true;
	}
	else if (FStrEq("score", pkvd->szKeyName))
	{
		score = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq("team_score", pkvd->szKeyName))
	{
		team_score = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq("triggerstate", pkvd->szKeyName))
	{
		switch (atoi(pkvd->szValue))
		{
		case 1:
			triggerType = USE_ON;
			break;

		case 2:
			triggerType = USE_TOGGLE;
			break;

		default:
			triggerType = USE_OFF;
			break;
		}

		return true;
	}

	return false;
}

// ========================
// LRC - New Trigger Entities
// ========================

//=========================================================
// multi_watcher
// Watches multiple entity states and triggers based on conditions
//=========================================================
#define SF_MULTIWATCHER_START_ON 0x0001
#define SF_MULTIWATCHER_NOT 0x0004

class CMultiWatcher : public CBaseToggle
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT WatcherThink();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	STATE GetState() override { return m_iState; }

	int m_cTargets;
	int m_iszTargetName[MAX_MULTI_TARGETS];
	int m_iLogic;	  // 0=AND, 1=OR
	STATE m_iState;
};

LINK_ENTITY_TO_CLASS(multi_watcher, CMultiWatcher);
LINK_ENTITY_TO_CLASS(watcher, CMultiWatcher);  // LRC - alias for multi_watcher

TYPEDESCRIPTION CMultiWatcher::m_SaveData[] =
	{
		DEFINE_FIELD(CMultiWatcher, m_cTargets, FIELD_INTEGER),
		DEFINE_ARRAY(CMultiWatcher, m_iszTargetName, FIELD_STRING, MAX_MULTI_TARGETS),
		DEFINE_FIELD(CMultiWatcher, m_iLogic, FIELD_INTEGER),
		DEFINE_FIELD(CMultiWatcher, m_iState, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CMultiWatcher, CBaseToggle);

void CMultiWatcher::Spawn()
{
	pev->solid = SOLID_NOT;
	m_iState = STATE_OFF;

	if (pev->spawnflags & SF_MULTIWATCHER_START_ON)
	{
		SetThink(&CMultiWatcher::WatcherThink);
		SetNextThink(0.5);
	}

	SetUse(NULL);
}

bool CMultiWatcher::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "logic"))
	{
		m_iLogic = atoi(pkvd->szValue);
		return true;
	}
	else if (m_cTargets < MAX_MULTI_TARGETS)
	{
		char tmp[128];
		UTIL_StripToken(pkvd->szKeyName, tmp, sizeof(tmp));
		m_iszTargetName[m_cTargets] = ALLOC_STRING(tmp);
		m_cTargets++;
		return true;
	}

	return CBaseToggle::KeyValue(pkvd);
}

void CMultiWatcher::WatcherThink()
{
	STATE oldState = m_iState;
	int cActive = 0;

	for (int i = 0; i < m_cTargets; i++)
	{
		CBaseEntity* pTarget = UTIL_FindEntityByTargetname(nullptr, STRING(m_iszTargetName[i]));
		if (pTarget && pTarget->GetState() == STATE_ON)
			cActive++;
	}

	bool bConditionMet;
	if (m_iLogic == 1) // OR
		bConditionMet = (cActive > 0);
	else // AND
		bConditionMet = (cActive >= m_cTargets);

	if (pev->spawnflags & SF_MULTIWATCHER_NOT)
		bConditionMet = !bConditionMet;

	m_iState = bConditionMet ? STATE_ON : STATE_OFF;

	if (m_iState != oldState)
	{
		if (!FStringNull(pev->target))
		{
			USE_TYPE useType = (m_iState == STATE_ON) ? USE_ON : USE_OFF;
			FireTargets(STRING(pev->target), this, this, useType, 0);
		}
	}

	SetNextThink(0.5);
}

//=========================================================
// trigger_command
// Executes server console commands when triggered
//=========================================================
class CTriggerCommand : public CBaseEntity
{
public:
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

LINK_ENTITY_TO_CLASS(trigger_command, CTriggerCommand);

void CTriggerCommand::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!FStringNull(pev->netname))
	{
		char szCommand[256];
		snprintf(szCommand, sizeof(szCommand), "%s\n", STRING(pev->netname));
		SERVER_COMMAND(szCommand);
	}
}

//=========================================================
// trigger_changecvar
// Changes server cvars when triggered
//=========================================================
class CTriggerChangeCvar : public CBaseEntity
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	string_t m_iszCvarName;
	string_t m_iszNewValue;
};

LINK_ENTITY_TO_CLASS(trigger_changecvar, CTriggerChangeCvar);

bool CTriggerChangeCvar::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "cvarname"))
	{
		m_iszCvarName = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "cvarvalue"))
	{
		m_iszNewValue = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void CTriggerChangeCvar::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!FStringNull(m_iszCvarName) && !FStringNull(m_iszNewValue))
	{
		CVAR_SET_STRING(STRING(m_iszCvarName), STRING(m_iszNewValue));
	}
}

//=========================================================
// trigger_inout
// Fires only when entering/leaving the trigger volume
//=========================================================
#define SF_TRIGGERINOUT_EVERYTHING 0x0001

class CTriggerInOut : public CBaseTrigger
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT InOutTouch(CBaseEntity* pOther);
	void EXPORT InOutThink();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	string_t m_iszAltTarget;  // target to fire on exit
	string_t m_iszBothTarget; // target to fire on both
	EHANDLE m_hInsideEntities[16];
	int m_cInsideEntities;
};

LINK_ENTITY_TO_CLASS(trigger_inout, CTriggerInOut);

TYPEDESCRIPTION CTriggerInOut::m_SaveData[] =
	{
		DEFINE_FIELD(CTriggerInOut, m_iszAltTarget, FIELD_STRING),
		DEFINE_FIELD(CTriggerInOut, m_iszBothTarget, FIELD_STRING),
		DEFINE_FIELD(CTriggerInOut, m_cInsideEntities, FIELD_INTEGER),
		DEFINE_ARRAY(CTriggerInOut, m_hInsideEntities, FIELD_EHANDLE, 16),
};

IMPLEMENT_SAVERESTORE(CTriggerInOut, CBaseTrigger);

void CTriggerInOut::Spawn()
{
	InitTrigger();
	SetTouch(&CTriggerInOut::InOutTouch);
	SetThink(&CTriggerInOut::InOutThink);
	SetNextThink(0.5);
	m_cInsideEntities = 0;
}

bool CTriggerInOut::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszAltTarget"))
	{
		m_iszAltTarget = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszBothTarget"))
	{
		m_iszBothTarget = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CBaseTrigger::KeyValue(pkvd);
}

void CTriggerInOut::InOutTouch(CBaseEntity* pOther)
{
	if (!(pev->spawnflags & SF_TRIGGERINOUT_EVERYTHING))
	{
		if (!pOther->IsPlayer())
			return;
	}

	// Check if already inside
	for (int i = 0; i < m_cInsideEntities; i++)
	{
		if (m_hInsideEntities[i] == pOther)
			return; // Already tracked
	}

	// Entity is entering
	if (m_cInsideEntities < 16)
	{
		m_hInsideEntities[m_cInsideEntities] = pOther;
		m_cInsideEntities++;

		// Fire enter target
		if (!FStringNull(pev->target))
			FireTargets(STRING(pev->target), pOther, this, USE_TOGGLE, 0);
		if (!FStringNull(m_iszBothTarget))
			FireTargets(STRING(m_iszBothTarget), pOther, this, USE_ON, 0);
	}
}

void CTriggerInOut::InOutThink()
{
	// Check each tracked entity to see if it has left
	for (int i = m_cInsideEntities - 1; i >= 0; i--)
	{
		CBaseEntity* pEntity = m_hInsideEntities[i];

		bool bStillInside = false;
		if (pEntity)
		{
			// Simple bounding box containment check
			if (pEntity->pev->absmin.x <= pev->absmax.x &&
				pEntity->pev->absmax.x >= pev->absmin.x &&
				pEntity->pev->absmin.y <= pev->absmax.y &&
				pEntity->pev->absmax.y >= pev->absmin.y &&
				pEntity->pev->absmin.z <= pev->absmax.z &&
				pEntity->pev->absmax.z >= pev->absmin.z)
			{
				bStillInside = true;
			}
		}

		if (!bStillInside)
		{
			// Entity has left
			if (!FStringNull(m_iszAltTarget))
				FireTargets(STRING(m_iszAltTarget), pEntity, this, USE_TOGGLE, 0);
			if (!FStringNull(m_iszBothTarget))
				FireTargets(STRING(m_iszBothTarget), pEntity, this, USE_OFF, 0);

			// Remove from tracked list by shifting remaining entries
			for (int j = i; j < m_cInsideEntities - 1; j++)
				m_hInsideEntities[j] = m_hInsideEntities[j + 1];
			m_cInsideEntities--;
		}
	}

	SetNextThink(0.5);
}

//=========================================================
// trigger_bounce
// Bounces touching entities
//=========================================================
class CTriggerBounce : public CBaseTrigger
{
public:
	void Spawn() override;
	void EXPORT BounceTouch(CBaseEntity* pOther);
};

LINK_ENTITY_TO_CLASS(trigger_bounce, CTriggerBounce);

void CTriggerBounce::Spawn()
{
	InitTrigger();
	SetTouch(&CTriggerBounce::BounceTouch);
}

void CTriggerBounce::BounceTouch(CBaseEntity* pOther)
{
	if (pev->spawnflags != 0 && !pOther->IsPlayer())
		return;

	float flBounceScale = (pev->speed != 0) ? pev->speed : 1.0f;

	// Reflect the velocity
	Vector vecBounce = pOther->pev->velocity;
	vecBounce = vecBounce + (2 * DotProduct(-vecBounce, pev->movedir)) * pev->movedir;
	vecBounce = vecBounce * flBounceScale;

	pOther->pev->velocity = vecBounce;
}

//=========================================================
// trigger_onsight
// Fires when entities can see each other
//=========================================================
class CTriggerOnSight : public CBaseDelay
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT OnSightThink();
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	STATE GetState() override { return m_fFired ? STATE_ON : STATE_OFF; }

	string_t m_iszLookEntity;
	string_t m_iszSeenEntity;
	bool m_fFired;
};

LINK_ENTITY_TO_CLASS(trigger_onsight, CTriggerOnSight);

TYPEDESCRIPTION CTriggerOnSight::m_SaveData[] =
	{
		DEFINE_FIELD(CTriggerOnSight, m_iszLookEntity, FIELD_STRING),
		DEFINE_FIELD(CTriggerOnSight, m_iszSeenEntity, FIELD_STRING),
		DEFINE_FIELD(CTriggerOnSight, m_fFired, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CTriggerOnSight, CBaseDelay);

void CTriggerOnSight::Spawn()
{
	m_fFired = false;
}

bool CTriggerOnSight::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "lookentity"))
	{
		m_iszLookEntity = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "seenentity"))
	{
		m_iszSeenEntity = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CBaseDelay::KeyValue(pkvd);
}

void CTriggerOnSight::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	SetThink(&CTriggerOnSight::OnSightThink);
	SetNextThink(0.1);
	m_fFired = false;
}

void CTriggerOnSight::OnSightThink()
{
	CBaseEntity* pLooker = nullptr;
	CBaseEntity* pSeen = nullptr;

	if (!FStringNull(m_iszLookEntity))
		pLooker = UTIL_FindEntityByTargetname(nullptr, STRING(m_iszLookEntity));
	if (!FStringNull(m_iszSeenEntity))
		pSeen = UTIL_FindEntityByTargetname(nullptr, STRING(m_iszSeenEntity));

	if (!pLooker || !pSeen)
	{
		SetNextThink(1.0);
		return;
	}

	// Check line of sight
	TraceResult tr;
	UTIL_TraceLine(pLooker->pev->origin + pLooker->pev->view_ofs,
		pSeen->pev->origin, ignore_monsters, pLooker->edict(), &tr);

	if (tr.flFraction >= 1.0f || tr.pHit == pSeen->edict())
	{
		if (!m_fFired)
		{
			m_fFired = true;
			SUB_UseTargets(pLooker, USE_TOGGLE, 0);
		}
	}
	else
	{
		m_fFired = false;
	}

	SetNextThink(0.1);
}

//=========================================================
// trigger_startpatrol
// Starts monster patrol routes
//=========================================================
class CTriggerStartPatrol : public CBaseEntity
{
public:
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

LINK_ENTITY_TO_CLASS(trigger_startpatrol, CTriggerStartPatrol);

void CTriggerStartPatrol::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (FStringNull(pev->target))
		return;

	CBaseEntity* pMonster = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));
	if (!pMonster)
		return;

	CBaseMonster* pMonsterPtr = pMonster->MyMonsterPointer();
	if (!pMonsterPtr)
		return;

	// Find the first path corner
	if (!FStringNull(pev->message))
	{
		CBaseEntity* pPath = UTIL_FindEntityByTargetname(nullptr, STRING(pev->message));
		if (pPath)
		{
			pMonsterPtr->m_pGoalEnt = pPath;
			pMonsterPtr->m_movementGoal = MOVEGOAL_PATHCORNER;
		}
	}
}

//=========================================================
// trigger_motion
// Applies position/angle/velocity transformations to entities
//=========================================================
class CTriggerMotion : public CPointEntity
{
public:
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	bool KeyValue(KeyValueData* pkvd) override;

	string_t m_iszPosition;
	string_t m_iszAngles;
	string_t m_iszVelocity;
};

LINK_ENTITY_TO_CLASS(trigger_motion, CTriggerMotion);

bool CTriggerMotion::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszPosition"))
	{
		m_iszPosition = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszAngles"))
	{
		m_iszAngles = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszVelocity"))
	{
		m_iszVelocity = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

void CTriggerMotion::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (FStringNull(pev->target))
		return;

	CBaseEntity* pTarget = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));
	if (!pTarget)
		return;

	if (!FStringNull(m_iszPosition))
	{
		Vector vecPos = CalcLocus_Position(this, pActivator, STRING(m_iszPosition));
		UTIL_SetOrigin(pTarget->pev, vecPos);
	}

	if (!FStringNull(m_iszAngles))
	{
		Vector vecAng = CalcLocus_Position(this, pActivator, STRING(m_iszAngles));
		pTarget->pev->angles = vecAng;
	}

	if (!FStringNull(m_iszVelocity))
	{
		Vector vecVel = CalcLocus_Velocity(this, pActivator, STRING(m_iszVelocity));
		pTarget->pev->velocity = vecVel;
	}
}

//=========================================================
// motion_manager
// Continuous motion control entity
//=========================================================
class CMotionManager : public CPointEntity
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT MotionThink();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	string_t m_iszPosition;
	string_t m_iszVelocity;
	EHANDLE m_hTarget;
	bool m_bActive;
};

LINK_ENTITY_TO_CLASS(motion_manager, CMotionManager);

TYPEDESCRIPTION CMotionManager::m_SaveData[] =
	{
		DEFINE_FIELD(CMotionManager, m_iszPosition, FIELD_STRING),
		DEFINE_FIELD(CMotionManager, m_iszVelocity, FIELD_STRING),
		DEFINE_FIELD(CMotionManager, m_hTarget, FIELD_EHANDLE),
		DEFINE_FIELD(CMotionManager, m_bActive, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CMotionManager, CPointEntity);

void CMotionManager::Spawn()
{
	m_bActive = false;
}

bool CMotionManager::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszPosition"))
	{
		m_iszPosition = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszVelocity"))
	{
		m_iszVelocity = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

void CMotionManager::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (useType == USE_OFF || (useType == USE_TOGGLE && m_bActive))
	{
		m_bActive = false;
		SetThink(NULL);
		DontThink();
		return;
	}

	m_bActive = true;

	if (!FStringNull(pev->target))
		m_hTarget = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));

	SetThink(&CMotionManager::MotionThink);
	SetNextThink(0);
}

void CMotionManager::MotionThink()
{
	if (!m_bActive)
	{
		DontThink();
		return;
	}

	CBaseEntity* pTarget = m_hTarget;

	// Re-resolve target if EHANDLE became invalid (e.g. after save/restore)
	if (!pTarget && !FStringNull(pev->target))
	{
		m_hTarget = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));
		pTarget = m_hTarget;
	}

	if (!pTarget)
	{
		m_bActive = false;
		DontThink();
		return;
	}

	if (!FStringNull(m_iszPosition))
	{
		Vector vecPos = CalcLocus_Position(this, pTarget, STRING(m_iszPosition));
		UTIL_SetOrigin(pTarget->pev, vecPos);
	}

	if (!FStringNull(m_iszVelocity))
	{
		Vector vecVel = CalcLocus_Velocity(this, pTarget, STRING(m_iszVelocity));
		pTarget->pev->velocity = vecVel;
	}

	SetNextThink(0);
}

//=========================================================
// RenderFxFader
// Subsidiary entity for env_render fade effects
//=========================================================
class CRenderFxFader : public CBaseEntity
{
public:
	void Spawn() override;
	void EXPORT FadeThink();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	float m_flStartTime;
	float m_flDuration;
	float m_flStartAmt;
	float m_flTargetAmt;
	EHANDLE m_hTarget;
	int m_iszTarget;
};

LINK_ENTITY_TO_CLASS(render_fx_fader, CRenderFxFader);

TYPEDESCRIPTION CRenderFxFader::m_SaveData[] =
	{
		DEFINE_FIELD(CRenderFxFader, m_flStartTime, FIELD_TIME),
		DEFINE_FIELD(CRenderFxFader, m_flDuration, FIELD_FLOAT),
		DEFINE_FIELD(CRenderFxFader, m_flStartAmt, FIELD_FLOAT),
		DEFINE_FIELD(CRenderFxFader, m_flTargetAmt, FIELD_FLOAT),
		DEFINE_FIELD(CRenderFxFader, m_hTarget, FIELD_EHANDLE),
		DEFINE_FIELD(CRenderFxFader, m_iszTarget, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CRenderFxFader, CBaseEntity);

void CRenderFxFader::Spawn()
{
	m_flStartTime = gpGlobals->time;
	SetThink(&CRenderFxFader::FadeThink);
	SetNextThink(0);
}

void CRenderFxFader::FadeThink()
{
	if (!m_hTarget)
	{
		UTIL_Remove(this);
		return;
	}

	float fElapsed = gpGlobals->time - m_flStartTime;
	float fFraction = (m_flDuration > 0) ? fElapsed / m_flDuration : 1.0f;

	if (fFraction >= 1.0f)
	{
		m_hTarget->pev->renderamt = m_flTargetAmt;
		UTIL_Remove(this);
		return;
	}

	m_hTarget->pev->renderamt = m_flStartAmt + (m_flTargetAmt - m_flStartAmt) * fFraction;
	SetNextThink(0);
}
//=========================================================
// env_customize
// LRC - Modifies entity properties (model, visibility, solidity,
// framerate, body, skin, class, blood color, voice pitch)
// when triggered or on spawn.
//=========================================================
#include "talkmonster.h"
#include "animation.h"

#define SF_CUSTOM_AFFECTDEAD 1  // also affect dead monsters
#define SF_CUSTOM_ONCE       2  // remove after firing
#define SF_CUSTOM_DEBUG      4  // debug output

#define CUSTOM_FLAG_NOCHANGE  0
#define CUSTOM_FLAG_ON        1
#define CUSTOM_FLAG_OFF       2
#define CUSTOM_FLAG_TOGGLE    3
#define CUSTOM_FLAG_USETYPE   4
#define CUSTOM_FLAG_INVUSETYPE 5

class CEnvCustomize : public CBaseEntity
{
public:
void Spawn() override;
void Precache() override;
void PostSpawn() override;
void DesiredAction() override;
void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

void Affect(CBaseEntity* pTarget, USE_TYPE useType);
int GetActionFor(int iField, bool bActive, USE_TYPE useType);

int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

bool KeyValue(KeyValueData* pkvd) override;
bool Save(CSave& save) override;
bool Restore(CRestore& restore) override;
static TYPEDESCRIPTION m_SaveData[];

int m_iszModel = iStringNull;
int m_iClass = 0;         // 0 = no change
int m_iPlayerReact = -1;  // -1 = no change
int m_iVisible = CUSTOM_FLAG_NOCHANGE;
int m_iSolid = CUSTOM_FLAG_NOCHANGE;
int m_voicePitch = 0;     // 0 = no change
int m_iBloodColor = 0;    // 0 = no change
float m_fFramerate = -1;  // -1 = no change
float m_fController0 = -1;
float m_fController1 = -1;
float m_fController2 = -1;
float m_fController3 = -1;
};

LINK_ENTITY_TO_CLASS(env_customize, CEnvCustomize);

TYPEDESCRIPTION CEnvCustomize::m_SaveData[] =
{
DEFINE_FIELD(CEnvCustomize, m_iszModel, FIELD_STRING),
DEFINE_FIELD(CEnvCustomize, m_iClass, FIELD_INTEGER),
DEFINE_FIELD(CEnvCustomize, m_iPlayerReact, FIELD_INTEGER),
DEFINE_FIELD(CEnvCustomize, m_iVisible, FIELD_INTEGER),
DEFINE_FIELD(CEnvCustomize, m_iSolid, FIELD_INTEGER),
DEFINE_FIELD(CEnvCustomize, m_voicePitch, FIELD_INTEGER),
DEFINE_FIELD(CEnvCustomize, m_iBloodColor, FIELD_INTEGER),
DEFINE_FIELD(CEnvCustomize, m_fFramerate, FIELD_FLOAT),
DEFINE_FIELD(CEnvCustomize, m_fController0, FIELD_FLOAT),
DEFINE_FIELD(CEnvCustomize, m_fController1, FIELD_FLOAT),
DEFINE_FIELD(CEnvCustomize, m_fController2, FIELD_FLOAT),
DEFINE_FIELD(CEnvCustomize, m_fController3, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CEnvCustomize, CBaseEntity);

bool CEnvCustomize::KeyValue(KeyValueData* pkvd)
{
if (FStrEq(pkvd->szKeyName, "m_iVisible"))
{
m_iVisible = atoi(pkvd->szValue);
return true;
}
else if (FStrEq(pkvd->szKeyName, "m_iSolid"))
{
m_iSolid = atoi(pkvd->szValue);
return true;
}
else if (FStrEq(pkvd->szKeyName, "m_iszModel"))
{
m_iszModel = ALLOC_STRING(pkvd->szValue);
return true;
}
else if (FStrEq(pkvd->szKeyName, "m_voicePitch"))
{
m_voicePitch = atoi(pkvd->szValue);
return true;
}
else if (FStrEq(pkvd->szKeyName, "m_iClass"))
{
m_iClass = atoi(pkvd->szValue);
return true;
}
else if (FStrEq(pkvd->szKeyName, "m_iPlayerReact"))
{
m_iPlayerReact = atoi(pkvd->szValue);
return true;
}
else if (FStrEq(pkvd->szKeyName, "m_bloodColor") || FStrEq(pkvd->szKeyName, "m_iBloodColor"))
{
m_iBloodColor = atoi(pkvd->szValue);
return true;
}
else if (FStrEq(pkvd->szKeyName, "m_fFramerate"))
{
m_fFramerate = atof(pkvd->szValue);
return true;
}
else if (FStrEq(pkvd->szKeyName, "m_fController0"))
{
m_fController0 = atof(pkvd->szValue);
return true;
}
else if (FStrEq(pkvd->szKeyName, "m_fController1"))
{
m_fController1 = atof(pkvd->szValue);
return true;
}
else if (FStrEq(pkvd->szKeyName, "m_fController2"))
{
m_fController2 = atof(pkvd->szValue);
return true;
}
else if (FStrEq(pkvd->szKeyName, "m_fController3"))
{
m_fController3 = atof(pkvd->szValue);
return true;
}
return CBaseEntity::KeyValue(pkvd);
}

void CEnvCustomize::Spawn()
{
	// Initialize body/skin to -1 so unspecified fields mean "no change"
	pev->body = -1;
	pev->skin = -1;
	Precache();
}

void CEnvCustomize::Precache()
{
if (m_iszModel)
PRECACHE_MODEL((char*)STRING(m_iszModel));
}

void CEnvCustomize::PostSpawn()
{
if (!pev->targetname)
{
// no name - auto-apply when spawned
UTIL_DesiredAction(this);
}
}

void CEnvCustomize::DesiredAction()
{
Use(this, this, USE_TOGGLE, 0);
}

void CEnvCustomize::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
if (FStringNull(pev->target))
{
if (pActivator && pActivator != this)
Affect(pActivator, useType);
else if (pev->spawnflags & SF_CUSTOM_DEBUG)
LOG_DEBUG("env_customize \"%s\" was fired without a locus!", STRING(pev->targetname));
}
else
{
bool bFound = false;
// Try targetname first
CBaseEntity* pTarget = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));
while (pTarget)
{
Affect(pTarget, useType);
bFound = true;
pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(pev->target));
}
// Then try classname
pTarget = UTIL_FindEntityByClassname(nullptr, STRING(pev->target));
while (pTarget)
{
Affect(pTarget, useType);
bFound = true;
pTarget = UTIL_FindEntityByClassname(pTarget, STRING(pev->target));
}
if (!bFound && (pev->spawnflags & SF_CUSTOM_DEBUG))
LOG_DEBUG("env_customize \"%s\" does nothing; can't find entity \"%s\".",
STRING(pev->targetname), STRING(pev->target));
}

if (pev->spawnflags & SF_CUSTOM_ONCE)
UTIL_Remove(this);
}

int CEnvCustomize::GetActionFor(int iField, bool bActive, USE_TYPE useType)
{
switch (iField)
{
case CUSTOM_FLAG_NOCHANGE:
return CUSTOM_FLAG_NOCHANGE;
case CUSTOM_FLAG_ON:
return bActive ? CUSTOM_FLAG_NOCHANGE : CUSTOM_FLAG_ON;
case CUSTOM_FLAG_OFF:
return bActive ? CUSTOM_FLAG_OFF : CUSTOM_FLAG_NOCHANGE;
case CUSTOM_FLAG_TOGGLE:
return bActive ? CUSTOM_FLAG_OFF : CUSTOM_FLAG_ON;
case CUSTOM_FLAG_USETYPE:
if (useType == USE_ON)
return bActive ? CUSTOM_FLAG_NOCHANGE : CUSTOM_FLAG_ON;
else if (useType == USE_OFF)
return bActive ? CUSTOM_FLAG_OFF : CUSTOM_FLAG_NOCHANGE;
else // toggle
return bActive ? CUSTOM_FLAG_OFF : CUSTOM_FLAG_ON;
case CUSTOM_FLAG_INVUSETYPE:
if (useType == USE_OFF)
return bActive ? CUSTOM_FLAG_NOCHANGE : CUSTOM_FLAG_ON;
else if (useType == USE_ON)
return bActive ? CUSTOM_FLAG_OFF : CUSTOM_FLAG_NOCHANGE;
else
return bActive ? CUSTOM_FLAG_OFF : CUSTOM_FLAG_ON;
}
return CUSTOM_FLAG_NOCHANGE;
}

void CEnvCustomize::Affect(CBaseEntity* pTarget, USE_TYPE useType)
{
CBaseMonster* pMonster = pTarget->MyMonsterPointer();
if (!FBitSet(pev->spawnflags, SF_CUSTOM_AFFECTDEAD) && pMonster
&& pMonster->m_MonsterState == MONSTERSTATE_DEAD)
return;

if (m_iszModel)
{
Vector vecMins = pTarget->pev->mins;
Vector vecMaxs = pTarget->pev->maxs;
SET_MODEL(pTarget->edict(), STRING(m_iszModel));
UTIL_SetSize(pTarget->pev, vecMins, vecMaxs);
}

// Apply bone controllers if model provides them
if (m_fController0 >= 0)
SetController(GET_MODEL_PTR(pTarget->edict()), pTarget->pev, 0, m_fController0);
if (m_fController1 >= 0)
SetController(GET_MODEL_PTR(pTarget->edict()), pTarget->pev, 1, m_fController1);
if (m_fController2 >= 0)
SetController(GET_MODEL_PTR(pTarget->edict()), pTarget->pev, 2, m_fController2);
if (m_fController3 >= 0)
SetController(GET_MODEL_PTR(pTarget->edict()), pTarget->pev, 3, m_fController3);

if (m_fFramerate >= 0)
pTarget->pev->framerate = m_fFramerate;

if (pev->body != -1)
pTarget->pev->body = pev->body;

if (pev->skin != -1)
{
if (pev->skin == -2)
pTarget->pev->skin = (pTarget->pev->skin != 0) ? 0 : 1;
else
pTarget->pev->skin = pev->skin;
}

switch (GetActionFor(m_iVisible, !(pTarget->pev->effects & EF_NODRAW), useType))
{
case CUSTOM_FLAG_ON:
pTarget->pev->effects &= ~EF_NODRAW;
break;
case CUSTOM_FLAG_OFF:
pTarget->pev->effects |= EF_NODRAW;
break;
}

switch (GetActionFor(m_iSolid, pTarget->pev->solid != SOLID_NOT, useType))
{
case CUSTOM_FLAG_ON:
if (*(STRING(pTarget->pev->model)) == '*')
pTarget->pev->solid = SOLID_BSP;
else
pTarget->pev->solid = SOLID_SLIDEBOX;
break;
case CUSTOM_FLAG_OFF:
pTarget->pev->solid = SOLID_NOT;
break;
}

if (!pMonster)
return;

if (m_iBloodColor != 0)
pMonster->m_bloodColor = m_iBloodColor;

if (m_voicePitch > 0)
{
CTalkMonster* pTalk = dynamic_cast<CTalkMonster*>(pTarget);
if (pTalk)
pTalk->m_voicePitch = m_voicePitch;
}

if (m_iClass != 0)
{
pMonster->m_iClass = m_iClass;
if (pMonster->m_hEnemy)
{
pMonster->m_hEnemy = nullptr;
pMonster->SetConditions(bits_COND_NEW_ENEMY);
}
}

if (m_iPlayerReact >= 0)
pMonster->m_iPlayerReact = m_iPlayerReact;
}

//=========================================================
// trigger_changevalue
// LRC - changes a keyvalue field on a target entity at runtime.
//=========================================================
class CTriggerChangeValue : public CBaseDelay
{
public:
bool KeyValue(KeyValueData* pkvd) override;
void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

int ObjectCaps() override { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

bool Save(CSave& save) override;
bool Restore(CRestore& restore) override;

static TYPEDESCRIPTION m_SaveData[];

private:
string_t m_iszNewValue;
};

LINK_ENTITY_TO_CLASS(trigger_changevalue, CTriggerChangeValue);

TYPEDESCRIPTION CTriggerChangeValue::m_SaveData[] =
{
DEFINE_FIELD(CTriggerChangeValue, m_iszNewValue, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CTriggerChangeValue, CBaseDelay);

bool CTriggerChangeValue::KeyValue(KeyValueData* pkvd)
{
if (FStrEq(pkvd->szKeyName, "m_iszNewValue"))
{
m_iszNewValue = ALLOC_STRING(pkvd->szValue);
return true;
}
return CBaseDelay::KeyValue(pkvd);
}

void CTriggerChangeValue::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (FStringNull(pev->target))
	{
		LOG_ERROR("trigger_changevalue with no target set");
		return;
	}

	if (FStringNull(pev->netname))
	{
		LOG_ERROR("trigger_changevalue with no key name (netname) set");
		return;
	}

	CBaseEntity* pTarget = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));

	if (pTarget)
	{
		KeyValueData mypkvd;
		mypkvd.szKeyName = (char*)STRING(pev->netname);
		mypkvd.szValue = (char*)STRING(m_iszNewValue);
		mypkvd.fHandled = 0;
		pTarget->KeyValue(&mypkvd);
	}
}

//=========================================================
// trigger_hevcharge
// LRC - charges/discharges the player's HEV suit armor.
// If the trigger has a targetname, firing it will toggle state.
//=========================================================
#define SF_HEVCHARGE_NOANNOUNCE 0x04

class CTriggerHevCharge : public CBaseTrigger
{
public:
void Spawn() override;
void EXPORT ChargeTouch(CBaseEntity* pOther);
void EXPORT AnnounceThink();
};

LINK_ENTITY_TO_CLASS(trigger_hevcharge, CTriggerHevCharge);

void CTriggerHevCharge::Spawn()
{
InitTrigger();
SetTouch(&CTriggerHevCharge::ChargeTouch);
SetThink(&CTriggerHevCharge::AnnounceThink);

if (!FStringNull(pev->targetname))
{
SetUse(&CTriggerHevCharge::ToggleUse);
}
else
{
SetUse(nullptr);
}

if (FBitSet(pev->spawnflags, SF_TRIGGER_HURT_START_OFF))
pev->solid = SOLID_NOT;

	UTIL_SetOrigin(pev, pev->origin);
}

void CTriggerHevCharge::ChargeTouch(CBaseEntity* pOther)
{
if (IsLockedByMaster())
return;

if (!pOther->IsPlayer())
return;

auto pPlayer = static_cast<CBasePlayer*>(pOther);
if (!pPlayer->HasSuit())
return;

if (pev->dmgtime > gpGlobals->time)
return;
pev->dmgtime = gpGlobals->time + 0.5;

int iNewArmor = pOther->pev->armorvalue + pev->frags;
if (iNewArmor > MAX_NORMAL_BATTERY)
iNewArmor = MAX_NORMAL_BATTERY;
if (iNewArmor < 0)
iNewArmor = 0;
if (iNewArmor == (int)pOther->pev->armorvalue)
return;

pOther->pev->armorvalue = iNewArmor;

if (pev->target)
{
SUB_UseTargets(pOther, USE_TOGGLE, 0);
}

if (g_pGameRules->IsMultiplayer() || (pev->spawnflags & SF_HEVCHARGE_NOANNOUNCE))
return;

pev->aiment = ENT(pOther->pev);
SetNextThink(1);
}

void CTriggerHevCharge::AnnounceThink()
{
CBaseEntity* pEntity = CBaseEntity::Instance(pev->aiment);
if (pEntity && pEntity->IsPlayer())
{
auto pPlayer = static_cast<CBasePlayer*>(pEntity);
int pct = (int)((pPlayer->pev->armorvalue * 100.0) * (1.0 / MAX_NORMAL_BATTERY) + 0.5);
char szCharge[64];
snprintf(szCharge, sizeof(szCharge), "!HEV_%1dP", pct / 10);
pPlayer->SetSuitUpdate(szCharge, SUIT_SENTENCE, SUIT_REPEAT_OK);
}
}

//=========================================================
// watcher_count
// LRC - watches how many entities with a given targetname are active.
//=========================================================
#define SF_WRCOUNT_FIRESTART 0x0001
#define SF_WRCOUNT_STARTED 0x8000

class CWatcherCount : public CBaseToggle
{
public:
void Spawn() override;
void EXPORT Think() override;
int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

LINK_ENTITY_TO_CLASS(watcher_count, CWatcherCount);

void CWatcherCount::Spawn()
{
pev->solid = SOLID_NOT;
SetNextThink(0.5);
}

void CWatcherCount::Think()
{
SetNextThink(0.1);
int iCount = 0;

if (FStringNull(pev->noise))
{
pev->frags = 0;
return;
}

CBaseEntity* pCurrent = nullptr;

pCurrent = UTIL_FindEntityByTargetname(nullptr, STRING(pev->noise));
while (pCurrent != nullptr)
{
iCount++;
pCurrent = UTIL_FindEntityByTargetname(pCurrent, STRING(pev->noise));
}

if (pev->spawnflags & SF_WRCOUNT_STARTED)
{
if (iCount > pev->frags)
{
if (iCount < pev->impulse && pev->frags >= pev->impulse)
FireTargets(STRING(pev->netname), this, this, USE_TOGGLE, 0);
FireTargets(STRING(pev->noise1), this, this, USE_TOGGLE, 0);
}
else if (iCount < pev->frags)
{
if (iCount >= pev->impulse && pev->frags < pev->impulse)
FireTargets(STRING(pev->message), this, this, USE_TOGGLE, 0);
FireTargets(STRING(pev->noise2), this, this, USE_TOGGLE, 0);
}
}
else
{
pev->spawnflags |= SF_WRCOUNT_STARTED;
if (pev->spawnflags & SF_WRCOUNT_FIRESTART)
{
if (iCount < pev->impulse)
FireTargets(STRING(pev->netname), this, this, USE_TOGGLE, 0);
else
FireTargets(STRING(pev->message), this, this, USE_TOGGLE, 0);
}
}
pev->frags = iCount;
}

//=========================================================
// motion_thread - LRC 
// Helper entity for SoHL motion_manager pattern.
// Continuously adjusts target position/facing each frame.
//=========================================================
#define SF_MOTION_DEBUG 1

class CMotionThread : public CPointEntity
{
public:
void Think() override;

bool Save(CSave& save) override;
bool Restore(CRestore& restore) override;
static TYPEDESCRIPTION m_SaveData[];

string_t m_iszPosition;
int m_iPosMode = 0;
string_t m_iszFacing;
int m_iFaceMode = 0;
EHANDLE m_hLocus;
EHANDLE m_hTarget;
};

LINK_ENTITY_TO_CLASS(motion_thread, CMotionThread);

TYPEDESCRIPTION CMotionThread::m_SaveData[] =
{
DEFINE_FIELD(CMotionThread, m_iszPosition, FIELD_STRING),
DEFINE_FIELD(CMotionThread, m_iPosMode, FIELD_INTEGER),
DEFINE_FIELD(CMotionThread, m_iszFacing, FIELD_STRING),
DEFINE_FIELD(CMotionThread, m_iFaceMode, FIELD_INTEGER),
DEFINE_FIELD(CMotionThread, m_hLocus, FIELD_EHANDLE),
DEFINE_FIELD(CMotionThread, m_hTarget, FIELD_EHANDLE),
};

IMPLEMENT_SAVERESTORE(CMotionThread, CPointEntity);

void CMotionThread::Think()
{
if (m_hLocus == nullptr || m_hTarget == nullptr)
{
if (FBitSet(pev->spawnflags, SF_MOTION_DEBUG))
LOG_DEBUG("motion_thread expires");
SetThink(&CMotionThread::SUB_Remove);
SetNextThink(0.1);
return;
}

SetNextThink(0); // think every frame

if (FBitSet(pev->spawnflags, SF_MOTION_DEBUG))
LOG_DEBUG("motion_thread affects %s \"%s\":",
STRING(m_hTarget->pev->classname), STRING(m_hTarget->pev->targetname));

Vector vecTemp;

if (m_iszPosition)
{
switch (m_iPosMode)
{
case 0: // set position
UTIL_AssignOrigin(m_hTarget, CalcLocus_Position(this, m_hLocus, STRING(m_iszPosition)));
m_hTarget->pev->flags &= ~FL_ONGROUND;
break;
case 1: // offset position (= fake velocity)
UTIL_AssignOrigin(m_hTarget,
m_hTarget->pev->origin + gpGlobals->frametime * CalcLocus_Velocity(this, m_hLocus, STRING(m_iszPosition)));
m_hTarget->pev->flags &= ~FL_ONGROUND;
break;
case 2: // set velocity
UTIL_SetVelocity(m_hTarget, CalcLocus_Velocity(this, m_hLocus, STRING(m_iszPosition)));
break;
case 3: // accelerate
UTIL_SetVelocity(m_hTarget,
m_hTarget->pev->velocity + gpGlobals->frametime * CalcLocus_Velocity(this, m_hLocus, STRING(m_iszPosition)));
break;
case 4: // follow position
UTIL_SetVelocity(m_hTarget,
CalcLocus_Position(this, m_hLocus, STRING(m_iszPosition)) - m_hTarget->pev->origin);
break;
}
}

if (m_iszFacing)
{
switch (m_iFaceMode)
{
case 0: // set angles
vecTemp = CalcLocus_Velocity(this, m_hLocus, STRING(m_iszFacing));
if (vecTemp != g_vecZero)
UTIL_SetAngles(m_hTarget, UTIL_VecToAngles(vecTemp));
break;
case 1: // offset angles (rotate by velocity direction)
vecTemp = CalcLocus_Velocity(this, m_hLocus, STRING(m_iszFacing));
if (vecTemp != g_vecZero)
UTIL_SetAngles(m_hTarget, m_hTarget->pev->angles + gpGlobals->frametime * UTIL_VecToAngles(vecTemp));
break;
case 2: // rotate by raw angles
UTIL_StringToRandomVector((float*)vecTemp, STRING(m_iszFacing));
UTIL_SetAngles(m_hTarget, m_hTarget->pev->angles + gpGlobals->frametime * vecTemp);
break;
case 3: // set avelocity
UTIL_StringToRandomVector((float*)vecTemp, STRING(m_iszFacing));
UTIL_SetAvelocity(m_hTarget, vecTemp);
break;
}
}
}

//=========================================================
// trigger_rottest - LRC debugging entity for rotation testing
//=========================================================
class CTriggerRotTest : public CBaseDelay
{
public:
void PostSpawn() override;
void Think() override;

bool Save(CSave& save) override;
bool Restore(CRestore& restore) override;
static TYPEDESCRIPTION m_SaveData[];

private:
CBaseEntity* m_pMarker = nullptr;
CBaseEntity* m_pReference = nullptr;
CBaseEntity* m_pBridge = nullptr;
CBaseEntity* m_pHinge = nullptr;
};

LINK_ENTITY_TO_CLASS(trigger_rottest, CTriggerRotTest);

TYPEDESCRIPTION CTriggerRotTest::m_SaveData[] =
{
DEFINE_FIELD(CTriggerRotTest, m_pMarker, FIELD_CLASSPTR),
DEFINE_FIELD(CTriggerRotTest, m_pReference, FIELD_CLASSPTR),
DEFINE_FIELD(CTriggerRotTest, m_pBridge, FIELD_CLASSPTR),
DEFINE_FIELD(CTriggerRotTest, m_pHinge, FIELD_CLASSPTR),
};

IMPLEMENT_SAVERESTORE(CTriggerRotTest, CBaseDelay);

void CTriggerRotTest::PostSpawn()
{
m_pMarker = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));
m_pReference = UTIL_FindEntityByTargetname(nullptr, STRING(pev->netname));
m_pBridge = UTIL_FindEntityByTargetname(nullptr, STRING(pev->noise1));
m_pHinge = UTIL_FindEntityByTargetname(nullptr, STRING(pev->message));
pev->armorvalue = 0; // initial angle
if (pev->armortype == 0)
pev->armortype = 30;
SetNextThink(1);
}

void CTriggerRotTest::Think()
{
if (m_pReference)
{
m_pReference->pev->origin = pev->origin;
m_pReference->pev->origin.x = m_pReference->pev->origin.x + pev->health;
}
if (m_pMarker && m_pHinge)
{
Vector vecTemp = UTIL_AxisRotationToVec((m_pHinge->pev->origin - pev->origin).Normalize(), pev->armorvalue);
m_pMarker->pev->origin = pev->origin + pev->health * vecTemp;
}
if (m_pBridge && m_pMarker && m_pReference && m_pHinge)
{
Vector vecTemp = UTIL_AxisRotationToVec(
(m_pHinge->pev->origin - pev->origin).Normalize(), pev->armorvalue / 2);
m_pBridge->pev->origin = pev->origin + pev->health * vecTemp;
}

pev->armorvalue += pev->armortype;
SetNextThink(0.05);
}
