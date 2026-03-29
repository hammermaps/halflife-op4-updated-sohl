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
// LRC - MoveWith system implementation

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "movewith.h"

bool g_doingDesired = false;

//=========================================================
// CheckDesiredList
// Process all entities that have requested a deferred think/action
//=========================================================
void CheckDesiredList()
{
	g_doingDesired = true;

	CBaseEntity* pWorld = CWorld::World;
	if (!pWorld)
	{
		g_doingDesired = false;
		return;
	}

	CBaseEntity* pCurrent = pWorld->m_pAssistLink;
	while (pCurrent)
	{
		CBaseEntity* pNext = pCurrent->m_pAssistLink;

		if (FBitSet(pCurrent->m_iLFlags, LF_DODESIRED))
		{
			ClearBits(pCurrent->m_iLFlags, LF_DODESIRED);

			if (FBitSet(pCurrent->m_iLFlags, LF_DESIRED_ACTION))
			{
				ClearBits(pCurrent->m_iLFlags, LF_DESIRED_ACTION);
				pCurrent->DesiredAction();
			}

			if (FBitSet(pCurrent->m_iLFlags, LF_DESIRED_THINK))
			{
				ClearBits(pCurrent->m_iLFlags, LF_DESIRED_THINK);
				pCurrent->Think();
			}
		}

		pCurrent = pNext;
	}

	g_doingDesired = false;
}


//=========================================================
// CheckAssistList
// Process all entities in the assist list each frame
// Applies pending post-assist velocities
//=========================================================
void CheckAssistList()
{
	CBaseEntity* pWorld = CWorld::World;
	if (!pWorld)
		return;

	CBaseEntity* pCurrent = pWorld->m_pAssistLink;
	while (pCurrent)
	{
		if (FBitSet(pCurrent->m_iLFlags, LF_POSTASSISTVEL))
		{
			ClearBits(pCurrent->m_iLFlags, LF_POSTASSISTVEL);
			pCurrent->pev->velocity = pCurrent->m_vecPostAssistVel;
		}

		if (FBitSet(pCurrent->m_iLFlags, LF_POSTASSISTAVEL))
		{
			ClearBits(pCurrent->m_iLFlags, LF_POSTASSISTAVEL);
			pCurrent->pev->avelocity = pCurrent->m_vecPostAssistAVel;
		}

		pCurrent = pCurrent->m_pAssistLink;
	}

	CheckDesiredList();
}


//=========================================================
// UTIL_DesiredAction
// Schedule a deferred DesiredAction call for an entity
//=========================================================
void UTIL_DesiredAction(CBaseEntity* pEnt)
{
	pEnt->m_iLFlags |= LF_DODESIRED | LF_DESIRED_ACTION;

	// add to assist list if not already there
	if (!(pEnt->m_iLFlags & LF_DOASSIST))
	{
		pEnt->m_iLFlags |= LF_DOASSIST;
		if (CWorld::World)
		{
			pEnt->m_pAssistLink = CWorld::World->m_pAssistLink;
			CWorld::World->m_pAssistLink = pEnt;
		}
	}
}


//=========================================================
// UTIL_DesiredThink
// Schedule a deferred Think call for an entity
//=========================================================
void UTIL_DesiredThink(CBaseEntity* pEnt)
{
	pEnt->m_iLFlags |= LF_DODESIRED | LF_DESIRED_THINK;

	// add to assist list if not already there
	if (!(pEnt->m_iLFlags & LF_DOASSIST))
	{
		pEnt->m_iLFlags |= LF_DOASSIST;
		if (CWorld::World)
		{
			pEnt->m_pAssistLink = CWorld::World->m_pAssistLink;
			CWorld::World->m_pAssistLink = pEnt;
		}
	}
}


//=========================================================
// UTIL_AssignOrigin
// Change the origin to the given position, and bring any
// MoveWith children along too
//=========================================================
void UTIL_AssignOrigin(CBaseEntity* pEnt, const Vector& vecOrigin, bool bInitiator)
{
	UTIL_SetOrigin(pEnt->pev, vecOrigin);

	if (!pEnt->m_pChildMoveWith)
		return;

	int loopbreaker = MAX_MOVEWITH_DEPTH;
	CBaseEntity* pChild = pEnt->m_pChildMoveWith;
	while (pChild)
	{
		if (loopbreaker-- <= 0)
		{
			ALERT(at_error, "MoveWith chain overflow in UTIL_AssignOrigin!\n");
			break;
		}
		Vector vecChildOrigin = vecOrigin + pChild->m_vecMoveWithOffset;
		UTIL_AssignOrigin(pChild, vecChildOrigin, false);
		pChild = pChild->m_pSiblingMoveWith;
	}
}


//=========================================================
// UTIL_SetVelocity
// Set velocity and propagate to MoveWith children
//=========================================================
void UTIL_SetVelocity(CBaseEntity* pEnt, const Vector& vecSet)
{
	pEnt->pev->velocity = vecSet;

	if (!pEnt->m_pChildMoveWith)
		return;

	int loopbreaker = MAX_MOVEWITH_DEPTH;
	CBaseEntity* pChild = pEnt->m_pChildMoveWith;
	while (pChild)
	{
		if (loopbreaker-- <= 0)
		{
			ALERT(at_error, "MoveWith chain overflow in UTIL_SetVelocity!\n");
			break;
		}

		// velocity is ignored on entities that aren't thinking!
		// ensure we have a think set, so the velocity takes effect
		if (vecSet != g_vecZero)
		{
			if (pChild->pev->movetype != MOVETYPE_PUSH)
			{
				if (pChild->m_pfnThink == NULL)
				{
					pChild->SetThink(&CBaseEntity::SUB_DoNothing);
					pChild->SetNextThink(0.01);
				}
			}
		}

		UTIL_SetVelocity(pChild, vecSet);
		pChild = pChild->m_pSiblingMoveWith;
	}
}


//=========================================================
// UTIL_SetAvelocity
// Set angular velocity and propagate to MoveWith children
//=========================================================
void UTIL_SetAvelocity(CBaseEntity* pEnt, const Vector& vecSet)
{
	pEnt->pev->avelocity = vecSet;

	if (!pEnt->m_pChildMoveWith)
		return;

	int loopbreaker = MAX_MOVEWITH_DEPTH;
	CBaseEntity* pChild = pEnt->m_pChildMoveWith;
	while (pChild)
	{
		if (loopbreaker-- <= 0)
		{
			ALERT(at_error, "MoveWith chain overflow in UTIL_SetAvelocity!\n");
			break;
		}
		UTIL_SetAvelocity(pChild, vecSet);
		pChild = pChild->m_pSiblingMoveWith;
	}
}


//=========================================================
// UTIL_SetAngles
// Set angles and propagate to MoveWith children
//=========================================================
void UTIL_SetAngles(CBaseEntity* pEnt, const Vector& vecSet)
{
	pEnt->pev->angles = vecSet;

	if (!pEnt->m_pChildMoveWith)
		return;

	int loopbreaker = MAX_MOVEWITH_DEPTH;
	CBaseEntity* pChild = pEnt->m_pChildMoveWith;
	while (pChild)
	{
		if (loopbreaker-- <= 0)
		{
			ALERT(at_error, "MoveWith chain overflow in UTIL_SetAngles!\n");
			break;
		}
		Vector vecChildAngles = vecSet + pChild->m_vecRotWithOffset;
		UTIL_SetAngles(pChild, vecChildAngles);
		pChild = pChild->m_pSiblingMoveWith;
	}
}
