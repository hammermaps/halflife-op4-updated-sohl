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

===== subs.cpp ========================================================

  frequently used global functions

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "nodes.h"
#include "doors.h"
#include "movewith.h"
#include "alias.h"
#include "logger.h"

extern bool FEntIsVisible(entvars_t* pev, entvars_t* pevTarget);

// Landmark class
void CPointEntity::Spawn()
{
	pev->solid = SOLID_NOT;
	//	UTIL_SetSize(pev, g_vecZero, g_vecZero);
}


class CNullEntity : public CBaseEntity
{
public:
	void Spawn() override;
};


// Null Entity, remove on startup
void CNullEntity::Spawn()
{
	REMOVE_ENTITY(ENT(pev));
}
LINK_ENTITY_TO_CLASS(info_null, CNullEntity);
LINK_ENTITY_TO_CLASS(info_compile_parameters, CNullEntity);
LINK_ENTITY_TO_CLASS(info_texlights, CNullEntity);

class CBaseDMStart : public CPointEntity
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	bool IsTriggered(CBaseEntity* pEntity) override;

private:
};

// These are the new entry points to entities.
LINK_ENTITY_TO_CLASS(info_player_deathmatch, CBaseDMStart);
LINK_ENTITY_TO_CLASS(info_player_start, CPointEntity);
LINK_ENTITY_TO_CLASS(info_player_coop, CPointEntity);
LINK_ENTITY_TO_CLASS(info_landmark, CPointEntity);

bool CBaseDMStart::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "master"))
	{
		pev->netname = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

bool CBaseDMStart::IsTriggered(CBaseEntity* pEntity)
{
	bool master = UTIL_IsMasterTriggered(pev->netname, pEntity);

	return master;
}

// This updates global tables that need to know about entities being removed.
// Called by both UTIL_Remove (deferred FL_KILLME removal) and SUB_Remove
// (immediate REMOVE_ENTITY). MoveWith chain cleanup MUST happen here so
// that entities removed via UTIL_Remove don't leave dangling pointers in
// parent/child lists — which causes crashes when mw_debug is enabled.
void CBaseEntity::UpdateOnRemove()
{
	int i;

	if (FBitSet(pev->flags, FL_GRAPHED))
	{
		// this entity was a LinkEnt in the world node graph, so we must remove it from
		// the graph since we are removing it from the world.
		for (i = 0; i < WorldGraph.m_cLinks; i++)
		{
			if (WorldGraph.m_pLinkPool[i].m_pLinkEnt == pev)
			{
				// if this link has a link ent which is the same ent that is removing itself, remove it!
				WorldGraph.m_pLinkPool[i].m_pLinkEnt = NULL;
			}
		}
	}
	if (!FStringNull(pev->globalname))
		gGlobalState.EntitySetState(pev->globalname, GLOBAL_DEAD);

	// LRC - remove from all MoveWith FIFO queues
	MoveWith_RemoveEntityFromQueues(this);
	m_pAssistLink = NULL;

	// LRC - remove from parent's MoveWith child list so stale pointers
	// cannot be dereferenced when the parent later iterates its children
	if (m_pMoveWith)
	{
		CBaseEntity* pPrev = NULL;
		CBaseEntity* pCur = m_pMoveWith->m_pChildMoveWith;
		while (pCur != NULL)
		{
			if (pCur == this)
			{
				if (pPrev)
					pPrev->m_pSiblingMoveWith = m_pSiblingMoveWith;
				else
					m_pMoveWith->m_pChildMoveWith = m_pSiblingMoveWith;
				break;
			}
			pPrev = pCur;
			pCur = pCur->m_pSiblingMoveWith;
		}
		m_pMoveWith = NULL;
		m_pSiblingMoveWith = NULL;
	}

	// LRC - orphan any children that were moving with this entity
	if (m_pChildMoveWith)
	{
		CBaseEntity* pCur = m_pChildMoveWith;
		while (pCur != NULL)
		{
			CBaseEntity* pNext = pCur->m_pSiblingMoveWith;
			pCur->m_pMoveWith = NULL;
			pCur->m_pSiblingMoveWith = NULL;
			pCur = pNext;
		}
		m_pChildMoveWith = NULL;
	}
}

// Convenient way to delay removing oneself
void CBaseEntity::SUB_Remove()
{
	UpdateOnRemove(); // also handles MoveWith list clean
	if (pev->health > 0)
	{
		// this situation can screw up monsters who can't tell their entity pointers are invalid.
		pev->health = 0;
		LOG_DEBUG("SUB_Remove called on entity with health > 0");
	}

	REMOVE_ENTITY(ENT(pev));
}


// Convenient way to explicitly do nothing (passed to functions that require a method)
void CBaseEntity::SUB_DoNothing()
{
}


// Global Savedata for Delay
TYPEDESCRIPTION CBaseDelay::m_SaveData[] =
	{
		DEFINE_FIELD(CBaseDelay, m_flDelay, FIELD_FLOAT),
		DEFINE_FIELD(CBaseDelay, m_iszKillTarget, FIELD_STRING),
		DEFINE_FIELD(CBaseDelay, m_hActivator, FIELD_EHANDLE),  // LRC - moved here from CBaseToggle
};

IMPLEMENT_SAVERESTORE(CBaseDelay, CBaseEntity);

bool CBaseDelay::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "delay"))
	{
		m_flDelay = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "killtarget"))
	{
		m_iszKillTarget = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}


/*
==============================
SUB_UseTargets

If self.delay is set, a DelayedUse entity will be created that will actually
do the SUB_UseTargets after that many seconds have passed.

Removes all entities with a targetname that match self.killtarget,
and removes them, so some events can remove other triggers.

Search for (string)targetname in all entities that
match (string)self.target and call their .use function (if they have one)

==============================
*/
void CBaseEntity::SUB_UseTargets(CBaseEntity* pActivator, USE_TYPE useType, float value)
{
	//
	// fire targets
	//
	if (!FStringNull(pev->target))
	{
		FireTargets(STRING(pev->target), pActivator, this, useType, value);
	}
}


void FireTargets(const char* targetName, CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!targetName)
		return;

	// LRC - handle Spirit-style USE_TYPE prefixes on target names
	if (targetName[0] == '+')
	{
		targetName++;
		useType = USE_ON;
	}
	else if (targetName[0] == '-')
	{
		targetName++;
		useType = USE_OFF;
	}
	else if (targetName[0] == '!')
	{
		targetName++;
		useType = USE_KILL;
	}
	else if (targetName[0] == '>')
	{
		targetName++;
		useType = USE_SAME;
	}

	LOG_DEBUG("Firing: (%s)", targetName);

	// LRC - use UTIL_FindEntityByTargetname to support *alias and group.member references
	CBaseEntity* pTarget = UTIL_FindEntityByTargetname(nullptr, targetName);
	while (pTarget)
	{
		if ((pTarget->pev->flags & FL_KILLME) == 0) // Don't use dying ents
		{
			LOG_DEBUG("Found: %s, firing (%s)", STRING(pTarget->pev->classname), targetName);
			if (useType == USE_KILL)
			{
				UTIL_Remove(pTarget);
			}
			else
			{
				pTarget->Use(pActivator, pCaller, useType, value);
			}
		}
		pTarget = UTIL_FindEntityByTargetname(pTarget, targetName);
	}

	// LRC - allow aliases to reflect their new values after all targets have fired
	FlushAliases();
}

LINK_ENTITY_TO_CLASS(DelayedUse, CBaseDelay);


void CBaseDelay::SUB_UseTargets(CBaseEntity* pActivator, USE_TYPE useType, float value)
{
	//
	// exit immediatly if we don't have a target or kill target
	//
	if (FStringNull(pev->target) && FStringNull(m_iszKillTarget))
		return;

	// LRC - allow changing of usetype
	if (useType == USE_SAME)
	{
		// don't change the use type
	}
	else if (useType == USE_NOT)
	{
		// invert the current state
		useType = USE_TOGGLE;
	}

	//
	// check for a delay
	//
	if (m_flDelay != 0)
	{
		// create a temp object to fire at a later time
		CBaseDelay* pTemp = GetClassPtr((CBaseDelay*)NULL);
		pTemp->pev->classname = MAKE_STRING("DelayedUse");

		pTemp->SetNextThink(m_flDelay);

		pTemp->SetThink(&CBaseDelay::DelayThink);

		// Save the useType
		pTemp->pev->button = (int)useType;
		pTemp->m_iszKillTarget = m_iszKillTarget;
		pTemp->m_flDelay = 0; // prevent "recursion"
		pTemp->pev->target = pev->target;

		// LRC - now using m_hActivator
		pTemp->m_hActivator = pActivator;

		return;
	}

	//
	// kill the killtargets
	//
	// LRC - now just USE_KILLs its killtarget, for consistency.
	if (!FStringNull(m_iszKillTarget))
	{
		LOG_DEBUG("KillTarget: %s", STRING(m_iszKillTarget));
		FireTargets(STRING(m_iszKillTarget), pActivator, this, USE_KILL, value);
	}

	//
	// fire targets
	//
	if (!FStringNull(pev->target))
	{
		FireTargets(STRING(pev->target), pActivator, this, useType, value);
	}
}


/*
void CBaseDelay:: SUB_UseTargetsEntMethod()
{
	SUB_UseTargets(pev);
}
*/

/*
QuakeEd only writes a single float for angles (bad idea), so up and down are
just constant angles.
*/
void SetMovedir(entvars_t* pev)
{
	if (pev->angles == Vector(0, -1, 0))
	{
		pev->movedir = Vector(0, 0, 1);
	}
	else if (pev->angles == Vector(0, -2, 0))
	{
		pev->movedir = Vector(0, 0, -1);
	}
	else
	{
		UTIL_MakeVectors(pev->angles);
		pev->movedir = gpGlobals->v_forward;
	}

	pev->angles = g_vecZero;
}




void CBaseDelay::DelayThink()
{
	// LRC - now using m_hActivator
	CBaseEntity* pActivator = m_hActivator;

	// The use type is cached (and stashed) in pev->button
	SUB_UseTargets(pActivator, (USE_TYPE)pev->button, 0);
	REMOVE_ENTITY(ENT(pev));
}


// Global Savedata for Toggle
TYPEDESCRIPTION CBaseToggle::m_SaveData[] =
	{
		DEFINE_FIELD(CBaseToggle, m_toggle_state, FIELD_INTEGER),
		DEFINE_FIELD(CBaseToggle, m_flActivateFinished, FIELD_TIME),
		DEFINE_FIELD(CBaseToggle, m_flMoveDistance, FIELD_FLOAT),
		DEFINE_FIELD(CBaseToggle, m_flWait, FIELD_FLOAT),
		DEFINE_FIELD(CBaseToggle, m_flLip, FIELD_FLOAT),
		DEFINE_FIELD(CBaseToggle, m_flTWidth, FIELD_FLOAT),
		DEFINE_FIELD(CBaseToggle, m_flTLength, FIELD_FLOAT),
		DEFINE_FIELD(CBaseToggle, m_vecPosition1, FIELD_POSITION_VECTOR),
		DEFINE_FIELD(CBaseToggle, m_vecPosition2, FIELD_POSITION_VECTOR),
		DEFINE_FIELD(CBaseToggle, m_vecAngle1, FIELD_VECTOR), // UNDONE: Position could go through transition, but also angle?
		DEFINE_FIELD(CBaseToggle, m_vecAngle2, FIELD_VECTOR), // UNDONE: Position could go through transition, but also angle?
		DEFINE_FIELD(CBaseToggle, m_cTriggersLeft, FIELD_INTEGER),
		DEFINE_FIELD(CBaseToggle, m_flHeight, FIELD_FLOAT),
		DEFINE_FIELD(CBaseToggle, m_pfnCallWhenMoveDone, FIELD_FUNCTION),
		DEFINE_FIELD(CBaseToggle, m_vecFinalDest, FIELD_POSITION_VECTOR),
		DEFINE_FIELD(CBaseToggle, m_vecFinalAngle, FIELD_VECTOR),
		DEFINE_FIELD(CBaseToggle, m_sMaster, FIELD_STRING),
		DEFINE_FIELD(CBaseToggle, m_bitsDamageInflict, FIELD_INTEGER), // damage type inflicted
};
IMPLEMENT_SAVERESTORE(CBaseToggle, CBaseAnimating);


bool CBaseToggle::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "lip"))
	{
		m_flLip = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "master"))
	{
		m_sMaster = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "distance"))
	{
		m_flMoveDistance = atof(pkvd->szValue);
		return true;
	}

	return CBaseDelay::KeyValue(pkvd);
}

/*
=============
LinearMove

calculate pev->velocity and pev->nextthink to reach vecDest from
pev->origin traveling at flSpeed
===============
*/
void CBaseToggle::LinearMove(Vector vecDest, float flSpeed)
{
	ASSERTSZ(flSpeed != 0, "LinearMove:  no speed is defined!");
	//	ASSERTSZ(m_pfnCallWhenMoveDone != NULL, "LinearMove: no post-move function defined");

	m_vecFinalDest = vecDest;

	// Already there?
	if (vecDest == pev->origin)
	{
		LinearMoveDone();
		return;
	}

	// set destdelta to the vector needed to move
	Vector vecDestDelta = vecDest - pev->origin;

	// divide vector length by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / flSpeed;

	// set nextthink to trigger a call to LinearMoveDone when dest is reached
	SetNextThink(flTravelTime);
	SetThink(&CBaseToggle::LinearMoveDone);

	// scale the destdelta vector by the time spent traveling to get velocity
	// LRC - Use UTIL_SetVelocity to propagate to MoveWith children
	UTIL_SetVelocity(this, vecDestDelta / flTravelTime);
}


/*
============
After moving, set origin to exact final destination, call "move done" function
============
*/
void CBaseToggle::LinearMoveDone()
{
	Vector delta = m_vecFinalDest - pev->origin;
	float error = delta.Length();
	if (error > 0.03125)
	{
		LinearMove(m_vecFinalDest, 100);
		return;
	}

	UTIL_AssignOrigin(this, m_vecFinalDest);
	UTIL_SetVelocity(this, g_vecZero);
	DontThink();  // LRC
	if (m_pfnCallWhenMoveDone)
		(this->*m_pfnCallWhenMoveDone)();
}

bool CBaseToggle::IsLockedByMaster()
{
	return !FStringNull(m_sMaster) && !UTIL_IsMasterTriggered(m_sMaster, m_hActivator);
}

void CBaseToggle::PlaySentence(const char* pszSentence, float duration, float volume, float attenuation)
{
	ASSERT(pszSentence != nullptr);

	if (!pszSentence || !IsAllowedToSpeak())
	{
		return;
	}

	PlaySentenceCore(pszSentence, duration, volume, attenuation);
}

void CBaseToggle::PlaySentenceCore(const char* pszSentence, float duration, float volume, float attenuation)
{
	if (pszSentence[0] == '!')
		EMIT_SOUND_DYN(edict(), CHAN_VOICE, pszSentence, volume, attenuation, 0, PITCH_NORM);
	else
		SENTENCEG_PlayRndSz(edict(), pszSentence, volume, attenuation, 0, PITCH_NORM);
}

void CBaseToggle::PlayScriptedSentence(const char* pszSentence, float duration, float volume, float attenuation, bool bConcurrent, CBaseEntity* pListener)
{
	PlaySentence(pszSentence, duration, volume, attenuation);
}


void CBaseToggle::SentenceStop()
{
	EMIT_SOUND(edict(), CHAN_VOICE, "common/null.wav", 1.0, ATTN_IDLE);
}

/*
=============
AngularMove

calculate pev->velocity and pev->nextthink to reach vecDest from
pev->origin traveling at flSpeed
Just like LinearMove, but rotational.
===============
*/
void CBaseToggle::AngularMove(Vector vecDestAngle, float flSpeed)
{
	ASSERTSZ(flSpeed != 0, "AngularMove:  no speed is defined!");
	//	ASSERTSZ(m_pfnCallWhenMoveDone != NULL, "AngularMove: no post-move function defined");

	m_vecFinalAngle = vecDestAngle;

	// Already there?
	if (vecDestAngle == pev->angles)
	{
		AngularMoveDone();
		return;
	}

	// set destdelta to the vector needed to move
	Vector vecDestDelta = vecDestAngle - pev->angles;

	// divide by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / flSpeed;

	// set nextthink to trigger a call to AngularMoveDone when dest is reached
	SetNextThink(flTravelTime);
	SetThink(&CBaseToggle::AngularMoveDone);

	// scale the destdelta vector by the time spent traveling to get velocity
	// LRC - Use UTIL_SetAvelocity to propagate to MoveWith children
	UTIL_SetAvelocity(this, vecDestDelta / flTravelTime);
}


/*
============
After rotating, set angle to exact final angle, call "move done" function
============
*/
void CBaseToggle::AngularMoveDone()
{
	UTIL_SetAngles(this, m_vecFinalAngle);
	UTIL_SetAvelocity(this, g_vecZero);
	DontThink();
	if (m_pfnCallWhenMoveDone)
		(this->*m_pfnCallWhenMoveDone)();
}


float CBaseToggle::AxisValue(int flags, const Vector& angles)
{
	if (FBitSet(flags, SF_DOOR_ROTATE_Z))
		return angles.z;
	if (FBitSet(flags, SF_DOOR_ROTATE_X))
		return angles.x;

	return angles.y;
}


void CBaseToggle::AxisDir(entvars_t* pev)
{
	if (FBitSet(pev->spawnflags, SF_DOOR_ROTATE_Z))
		pev->movedir = Vector(0, 0, 1); // around z-axis
	else if (FBitSet(pev->spawnflags, SF_DOOR_ROTATE_X))
		pev->movedir = Vector(1, 0, 0); // around x-axis
	else
		pev->movedir = Vector(0, 1, 0); // around y-axis
}


float CBaseToggle::AxisDelta(int flags, const Vector& angle1, const Vector& angle2)
{
	if (FBitSet(flags, SF_DOOR_ROTATE_Z))
		return angle1.z - angle2.z;

	if (FBitSet(flags, SF_DOOR_ROTATE_X))
		return angle1.x - angle2.x;

	return angle1.y - angle2.y;
}


/*
=============
FEntIsVisible

returns true if the passed entity is visible to caller, even if not infront ()
=============
*/
bool FEntIsVisible(
	entvars_t* pev,
	entvars_t* pevTarget)
{
	Vector vecSpot1 = pev->origin + pev->view_ofs;
	Vector vecSpot2 = pevTarget->origin + pevTarget->view_ofs;
	TraceResult tr;

	UTIL_TraceLine(vecSpot1, vecSpot2, ignore_monsters, ENT(pev), &tr);

	if (0 != tr.fInOpen && 0 != tr.fInWater)
		return false; // sight line crossed contents

	if (tr.flFraction == 1)
		return true;

	return false;
}


//=========================================================
// LRC - info_movewith entity
// A point entity that can be parented to another entity
//=========================================================
class CInfoMoveWith : public CPointEntity
{
public:
	void Spawn() override
	{
		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NONE;
	}
};

LINK_ENTITY_TO_CLASS(info_movewith, CInfoMoveWith);
