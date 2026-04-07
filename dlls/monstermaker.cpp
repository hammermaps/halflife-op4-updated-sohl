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
//=========================================================
// Monster Maker - this is an entity that creates monsters
// in the game.
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "saverestore.h"
#include "locus.h"
#include "logger.h"

// Monstermaker spawnflags
#define SF_MONSTERMAKER_START_ON 1	   // start active ( if has targetname )
#define SF_MONSTERMAKER_CYCLIC 4	   // drop one monster every time fired.
#define SF_MONSTERMAKER_MONSTERCLIP 8  // Children are blocked by monsterclip
#define SF_MONSTERMAKER_LEAVECORPSE 16 // Don't fade corpses
#define SF_MONSTERMAKER_FORCESPAWN 32  // Force spawn regardless of blocking entities
#define SF_MONSTERMAKER_NO_WPN_DROP 1024 // Corpses don't drop weapons

//=========================================================
// MonsterMaker - this ent creates monsters during the game.
//=========================================================
class CMonsterMaker : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT CyclicUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT MakerThink();
	void EXPORT MakeMonsterThink();
	void DeathNotice(entvars_t* pevChild) override; // monster maker children use this to tell the monster maker that they have died.
	void TryMakeMonster();		// check conditions and resolve locus positions before spawning
	CBaseMonster* MakeMonster(); // actually create the monster (returns the new entity)
	void SetLocusFromActivator(CBaseEntity* pActivator); // store activator pos/angles/vel as spawn locus

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	string_t m_iszMonsterClassname; // classname of the monster(s) that will be created.

	int m_cNumMonsters; // max number of monsters this ent can create


	int m_cLiveChildren;	// how many monsters made by this monster maker that are currently alive
	int m_iMaxLiveChildren; // max number of monsters that this maker may have out at one time.

	float m_flGround; // z coord of the ground under me, used to make sure no monsters are under the maker when it drops a new child

	STATE m_iState; // LRC
	STATE GetState() override { return m_iState; } // LRC

	bool m_fFadeChildren;  // should we make the children fadeout?
	float m_fSpawnDelay;   // delay between firing targets and spawning the monster (for env_warpball etc.)
};

LINK_ENTITY_TO_CLASS(monstermaker, CMonsterMaker);

TYPEDESCRIPTION CMonsterMaker::m_SaveData[] =
	{
		DEFINE_FIELD(CMonsterMaker, m_iszMonsterClassname, FIELD_STRING),
		DEFINE_FIELD(CMonsterMaker, m_cNumMonsters, FIELD_INTEGER),
		DEFINE_FIELD(CMonsterMaker, m_cLiveChildren, FIELD_INTEGER),
		DEFINE_FIELD(CMonsterMaker, m_flGround, FIELD_FLOAT),
		DEFINE_FIELD(CMonsterMaker, m_iMaxLiveChildren, FIELD_INTEGER),
		DEFINE_FIELD(CMonsterMaker, m_iState, FIELD_INTEGER),
		DEFINE_FIELD(CMonsterMaker, m_fFadeChildren, FIELD_BOOLEAN),
		DEFINE_FIELD(CMonsterMaker, m_fSpawnDelay, FIELD_FLOAT),
};


IMPLEMENT_SAVERESTORE(CMonsterMaker, CBaseMonster);

bool CMonsterMaker::KeyValue(KeyValueData* pkvd)
{

	if (FStrEq(pkvd->szKeyName, "monstercount"))
	{
		m_cNumMonsters = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_imaxlivechildren"))
	{
		m_iMaxLiveChildren = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "monstertype"))
	{
		m_iszMonsterClassname = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "spawndelay"))
	{
		m_fSpawnDelay = atof(pkvd->szValue);
		return true;
	}

	return CBaseMonster::KeyValue(pkvd);
}


void CMonsterMaker::Spawn()
{
	pev->solid = SOLID_NOT;

	m_cLiveChildren = 0;
	Precache();
	if (!FStringNull(pev->targetname))
	{
		if ((pev->spawnflags & SF_MONSTERMAKER_CYCLIC) != 0)
		{
			SetUse(&CMonsterMaker::CyclicUse); // drop one monster each time we fire
		}
		else
		{
			SetUse(&CMonsterMaker::ToggleUse); // so can be turned on/off

			if (FBitSet(pev->spawnflags, SF_MONSTERMAKER_START_ON))
			{ // start making monsters as soon as monstermaker spawns
				m_iState = STATE_ON; // LRC
				SetThink(&CMonsterMaker::MakerThink);
				SetNextThink(0); // must prime nextthink so MakerThink actually fires
			}
			else
			{ // wait to be activated.
				m_iState = STATE_OFF; // LRC
				SetThink(&CMonsterMaker::SUB_DoNothing);
			}
		}
	}
	else
	{ // no targetname, just start.
		SetNextThink(m_flDelay);
		m_iState = STATE_ON; // LRC
		SetThink(&CMonsterMaker::MakerThink);
	}

	if (m_cNumMonsters == 1 || (m_cNumMonsters != -1 && (pev->spawnflags & SF_MONSTERMAKER_LEAVECORPSE) != 0))
	{
		m_fFadeChildren = false;
	}
	else
	{
		m_fFadeChildren = true;
	}

	m_flGround = 0;
}

void CMonsterMaker::Precache()
{
	CBaseMonster::Precache();

	UTIL_PrecacheOther(STRING(m_iszMonsterClassname));
}

//=========================================================
// TryMakeMonster - checks conditions and resolves dynamic
// spawn position/angles/velocity from noise fields, then
// either spawns immediately or after a delay.
//=========================================================
void CMonsterMaker::TryMakeMonster()
{
	if (m_iMaxLiveChildren > 0 && m_cLiveChildren >= m_iMaxLiveChildren)
	{ // not allowed to make a new one yet. Too many live ones out right now.
		return;
	}

	// noise = targetname of entity to use as spawn position (dynamic origin)
	if (!FStringNull(pev->noise))
	{
		CBaseEntity* pTemp = UTIL_FindEntityByTargetname(nullptr, STRING(pev->noise));
		if (pTemp)
			pev->vuser1 = pTemp->pev->origin;
		else
			pev->vuser1 = pev->origin;
	}
	else
	{
		pev->vuser1 = pev->origin;
	}

	// noise1 = locus calc_position expression for a spawn offset
	if (!FStringNull(pev->noise1))
	{
		Vector vOffset = CalcLocus_Position(this, nullptr, STRING(pev->noise1));
		pev->vuser1 = pev->vuser1 + vOffset;
	}

	// noise2 = targetname of entity to use for spawn angles (dynamic angles)
	if (!FStringNull(pev->noise2))
	{
		CBaseEntity* pTemp = UTIL_FindEntityByTargetname(nullptr, STRING(pev->noise2));
		if (pTemp)
			pev->vuser2 = pTemp->pev->angles;
		else
			pev->vuser2 = pev->angles;
	}
	else
	{
		pev->vuser2 = pev->angles;
	}

	// noise3 = targetname of entity to use for spawn velocity (dynamic velocity)
	if (!FStringNull(pev->noise3))
	{
		CBaseEntity* pTemp = UTIL_FindEntityByTargetname(nullptr, STRING(pev->noise3));
		if (pTemp)
			pev->vuser3 = pTemp->pev->velocity;
	}

	if (0 == m_flGround)
	{
		// set altitude. Now that I'm activated, any breakables, etc should be out from under me.
		TraceResult tr;

		UTIL_TraceLine(pev->vuser1, pev->vuser1 - Vector(0, 0, 2048), ignore_monsters, ENT(pev), &tr);
		m_flGround = tr.vecEndPos.z;
	}

	Vector mins = pev->vuser1 - Vector(34, 34, 0);
	Vector maxs = pev->vuser1 + Vector(34, 34, 0);
	maxs.z = pev->vuser1.z;
	mins.z = m_flGround;

	CBaseEntity* pList[2];
	int count = UTIL_EntitiesInBox(pList, 2, mins, maxs, FL_CLIENT | FL_MONSTER);
	if ((pev->spawnflags & SF_MONSTERMAKER_FORCESPAWN) == 0 && count != 0)
	{
		// don't build a stack of monsters!
		return;
	}

	if (m_fSpawnDelay != 0.0f)
	{
		// If I have a target, fire now so things like env_warpball can play before the monster appears
		if (!FStringNull(pev->target))
		{
			// delay already overloaded for this entity, so can't call SUB_UseTargets()
			FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);
		}

		SetThink(&CMonsterMaker::MakeMonsterThink);
		SetNextThink(m_fSpawnDelay);
	}
	else
	{
		CBaseMonster* pMonst = MakeMonster();

		// If I have a target, fire! The spawned monster is the locus.
		if (!FStringNull(pev->target) && pMonst != nullptr)
		{
			FireTargets(STRING(pev->target), pMonst, this, USE_TOGGLE, 0);
		}
	}
}

//=========================================================
// MakeMonsterThink - deferred spawn after m_fSpawnDelay
//=========================================================
void CMonsterMaker::MakeMonsterThink()
{
	MakeMonster();
}

//=========================================================
// SetLocusFromActivator - stores the activator's position,
// angles and velocity into pev->vuser1/2/3 so they can be
// used as a dynamic spawn locus.
//=========================================================
void CMonsterMaker::SetLocusFromActivator(CBaseEntity* pActivator)
{
	if (pActivator)
	{
		pev->vuser1 = pActivator->pev->origin;
		pev->vuser2 = pActivator->pev->angles;
		pev->vuser3 = pActivator->pev->velocity;
	}
}

//=========================================================
// MakeMonster - creates the monster entity and returns it.
//=========================================================
CBaseMonster* CMonsterMaker::MakeMonster()
{
	edict_t* pent;
	entvars_t* pevCreate;

	pent = CREATE_NAMED_ENTITY(m_iszMonsterClassname);

	if (FNullEnt(pent))
	{
		LOG_INFO("NULL Ent in MonsterMaker!");
		return nullptr;
	}

	pevCreate = VARS(pent);
	pevCreate->origin = pev->vuser1;  // resolved dynamic position
	pevCreate->angles = pev->vuser2;  // resolved dynamic angles
	pevCreate->velocity = pev->vuser3; // resolved dynamic velocity

	SetBits(pevCreate->spawnflags, SF_MONSTER_FALL_TO_GROUND);

	if ((pev->spawnflags & SF_MONSTERMAKER_NO_WPN_DROP) != 0)
		SetBits(pevCreate->spawnflags, SF_MONSTER_NO_WPN_DROP);

	// Children hit monsterclip brushes
	if ((pev->spawnflags & SF_MONSTERMAKER_MONSTERCLIP) != 0)
		SetBits(pevCreate->spawnflags, SF_MONSTER_HITMONSTERCLIP);

	DispatchSpawn(ENT(pevCreate));
	pevCreate->owner = edict();

	// Copy custom monster behaviour from maker to child
	CBaseEntity* pEntity = CBaseEntity::Instance(pevCreate);
	CBaseMonster* pMonst = nullptr;
	if (pEntity != nullptr)
	{
		pMonst = pEntity->MyMonsterPointer();
	}
	if (pMonst != nullptr)
	{
		pMonst->m_iClass = this->m_iClass;
		pMonst->m_iPlayerReact = this->m_iPlayerReact;
		pMonst->m_iTriggerCondition = this->m_iTriggerCondition;
		pMonst->m_iszTriggerTarget = this->m_iszTriggerTarget;
	}

	if (!FStringNull(pev->netname))
	{
		// if I have a netname (overloaded), give the child monster that name as a targetname
		pevCreate->targetname = pev->netname;
	}

	m_cLiveChildren++; // count this monster
	m_cNumMonsters--;

	if (m_cNumMonsters == 0)
	{
		// Disable this forever.  Don't kill it because it still gets death notices
		SetThink(NULL);
		SetUse(NULL);
	}
	else if (m_iState != STATE_OFF) // LRC
	{
		// Reset think cycle for next monster (needed after spawn delay path)
		SetNextThink(m_flDelay);
		SetThink(&CMonsterMaker::MakerThink);
	}

	return pMonst;
}

//=========================================================
// CyclicUse - drops one monster from the monstermaker
// each time we call this.
//=========================================================
void CMonsterMaker::CyclicUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	SetLocusFromActivator(pActivator);
	TryMakeMonster();
}

//=========================================================
// ToggleUse - activates/deactivates the monster maker
//=========================================================
void CMonsterMaker::ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	SetLocusFromActivator(pActivator);

	if (!ShouldToggle(useType)) // LRC
		return;

	if (m_iState != STATE_OFF) // LRC
	{
		m_iState = STATE_OFF; // LRC
		SetThink(NULL);
	}
	else
	{
		m_iState = STATE_ON; // LRC
		SetThink(&CMonsterMaker::MakerThink);
	}

	SetNextThink(0);
}

//=========================================================
// MakerThink - creates a new monster every so often
//=========================================================
void CMonsterMaker::MakerThink()
{
	SetNextThink(m_flDelay);

	TryMakeMonster();
}


//=========================================================
//=========================================================
void CMonsterMaker::DeathNotice(entvars_t* pevChild)
{
	// ok, we've gotten the deathnotice from our child, now clear out its owner if we don't want it to fade.
	m_cLiveChildren--;

	if (!m_fFadeChildren)
	{
		pevChild->owner = NULL;
	}
}
