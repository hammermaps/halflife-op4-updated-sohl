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

//=========================================================
// FIFO Queue state — two-batch (current + next) for both
// PostAssist and Desired queues.
// Static in this TU; not saved/restored (rebuilt each frame).
//=========================================================

// PostAssist queue
static CBaseEntity* s_pPostAssistHeadCur = nullptr;
static CBaseEntity* s_pPostAssistTailCur = nullptr;
static CBaseEntity* s_pPostAssistHeadNext = nullptr;
static CBaseEntity* s_pPostAssistTailNext = nullptr;

// Desired queue
static CBaseEntity* s_pDesiredHeadCur = nullptr;
static CBaseEntity* s_pDesiredTailCur = nullptr;
static CBaseEntity* s_pDesiredHeadNext = nullptr;
static CBaseEntity* s_pDesiredTailNext = nullptr;

// Rate-limit budget warnings
static float s_flLastPostAssistWarnTime = 0;
static float s_flLastDesiredWarnTime = 0;

// True while processing the Desired queue (for reentrancy detection)
bool g_doingDesired = false;


//=========================================================
// MoveWith_ResetQueues
// Clear all queue state — call on level start (CWorld::Spawn)
//=========================================================
void MoveWith_ResetQueues()
{
	s_pPostAssistHeadCur = nullptr;
	s_pPostAssistTailCur = nullptr;
	s_pPostAssistHeadNext = nullptr;
	s_pPostAssistTailNext = nullptr;

	s_pDesiredHeadCur = nullptr;
	s_pDesiredTailCur = nullptr;
	s_pDesiredHeadNext = nullptr;
	s_pDesiredTailNext = nullptr;

	s_flLastPostAssistWarnTime = 0;
	s_flLastDesiredWarnTime = 0;

	g_doingDesired = false;
}


//=========================================================
// Internal: enqueue helpers (always append to tail = FIFO)
//=========================================================
static void EnqueuePostAssistNext(CBaseEntity* pEnt)
{
	if (FBitSet(pEnt->m_iLFlags, LF_IN_POSTASSIST_QUEUE))
		return; // duplicate guard

	SetBits(pEnt->m_iLFlags, LF_IN_POSTASSIST_QUEUE);
	pEnt->m_pPostAssistNext = nullptr;

	if (s_pPostAssistTailNext)
	{
		s_pPostAssistTailNext->m_pPostAssistNext = pEnt;
		s_pPostAssistTailNext = pEnt;
	}
	else
	{
		s_pPostAssistHeadNext = pEnt;
		s_pPostAssistTailNext = pEnt;
	}
}

static void EnqueueDesiredNext(CBaseEntity* pEnt)
{
	if (FBitSet(pEnt->m_iLFlags, LF_IN_DESIRED_QUEUE))
		return; // duplicate guard

	SetBits(pEnt->m_iLFlags, LF_IN_DESIRED_QUEUE);
	pEnt->m_pDesiredNext = nullptr;

	if (s_pDesiredTailNext)
	{
		s_pDesiredTailNext->m_pDesiredNext = pEnt;
		s_pDesiredTailNext = pEnt;
	}
	else
	{
		s_pDesiredHeadNext = pEnt;
		s_pDesiredTailNext = pEnt;
	}
}


//=========================================================
// UTIL_PostAssistVelocity
// Schedule post-assist velocity application for an entity
//=========================================================
void UTIL_PostAssistVelocity(CBaseEntity* pEnt, const Vector& vel)
{
	if (!pEnt)
		return;

	pEnt->m_vecPostAssistVel = vel;
	SetBits(pEnt->m_iLFlags, LF_POSTASSISTVEL);
	EnqueuePostAssistNext(pEnt);
}


//=========================================================
// UTIL_PostAssistAVelocity
// Schedule post-assist angular velocity application
//=========================================================
void UTIL_PostAssistAVelocity(CBaseEntity* pEnt, const Vector& avel)
{
	if (!pEnt)
		return;

	pEnt->m_vecPostAssistAVel = avel;
	SetBits(pEnt->m_iLFlags, LF_POSTASSISTAVEL);
	EnqueuePostAssistNext(pEnt);
}


//=========================================================
// UTIL_DesiredAction
// Schedule a deferred DesiredAction call for an entity
//=========================================================
void UTIL_DesiredAction(CBaseEntity* pEnt)
{
	if (!pEnt)
		return;

	SetBits(pEnt->m_iLFlags, LF_DODESIRED | LF_DESIRED_ACTION);
	EnqueueDesiredNext(pEnt);
}


//=========================================================
// UTIL_DesiredThink
// Schedule a deferred Think call for an entity
//=========================================================
void UTIL_DesiredThink(CBaseEntity* pEnt)
{
	if (!pEnt)
		return;

	SetBits(pEnt->m_iLFlags, LF_DODESIRED | LF_DESIRED_THINK);
	EnqueueDesiredNext(pEnt);
}


//=========================================================
// Helper: check if entity is still valid for queue processing
//=========================================================
static bool IsEntityValidForQueue(CBaseEntity* pEnt)
{
	if (!pEnt)
		return false;
	if (!pEnt->pev)
		return false;
	if (pEnt->pev->flags & FL_KILLME)
		return false;
	return true;
}


//=========================================================
// MoveWith_ProcessFrameQueues
// Called once per server frame (replaces CheckAssistList + CheckDesiredList)
//
// Two-phase approach:
//   1) Swap: Cur = Next, Next = empty
//   2) Process Cur (FIFO), new requests go into Next → next frame
//=========================================================
void MoveWith_ProcessFrameQueues()
{
	// ---- Phase 1: Swap PostAssist batches ----
	// If there are leftover entries in Cur (from budget cap last frame),
	// prepend them before Next to preserve FIFO across frames.
	if (s_pPostAssistHeadCur)
	{
		// Leftover Cur exists — append Next to the end of Cur
		if (s_pPostAssistHeadNext)
		{
			s_pPostAssistTailCur->m_pPostAssistNext = s_pPostAssistHeadNext;
			s_pPostAssistTailCur = s_pPostAssistTailNext;
		}
		// Cur stays as is (now Cur + old Next merged)
	}
	else
	{
		// No leftover — just promote Next to Cur
		s_pPostAssistHeadCur = s_pPostAssistHeadNext;
		s_pPostAssistTailCur = s_pPostAssistTailNext;
	}
	s_pPostAssistHeadNext = nullptr;
	s_pPostAssistTailNext = nullptr;

	// ---- Phase 2: Process PostAssist Cur (FIFO with budget) ----
	int postAssistCount = 0;
	while (s_pPostAssistHeadCur && postAssistCount < MAX_POSTASSIST_PER_FRAME)
	{
		CBaseEntity* pCurrent = s_pPostAssistHeadCur;
		s_pPostAssistHeadCur = pCurrent->m_pPostAssistNext;
		if (!s_pPostAssistHeadCur)
			s_pPostAssistTailCur = nullptr;

		pCurrent->m_pPostAssistNext = nullptr;
		ClearBits(pCurrent->m_iLFlags, LF_IN_POSTASSIST_QUEUE);

		if (!IsEntityValidForQueue(pCurrent))
			continue;

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

		postAssistCount++;
	}

	// Budget warning (rate-limited)
	if (s_pPostAssistHeadCur && gpGlobals->time - s_flLastPostAssistWarnTime > 5.0f)
	{
		ALERT(at_warning, "MoveWith PostAssist queue budget exceeded (%d). Remaining deferred to next frame.\n", MAX_POSTASSIST_PER_FRAME);
		s_flLastPostAssistWarnTime = gpGlobals->time;
	}

	// ---- Phase 3: Swap Desired batches ----
	if (s_pDesiredHeadCur)
	{
		if (s_pDesiredHeadNext)
		{
			s_pDesiredTailCur->m_pDesiredNext = s_pDesiredHeadNext;
			s_pDesiredTailCur = s_pDesiredTailNext;
		}
	}
	else
	{
		s_pDesiredHeadCur = s_pDesiredHeadNext;
		s_pDesiredTailCur = s_pDesiredTailNext;
	}
	s_pDesiredHeadNext = nullptr;
	s_pDesiredTailNext = nullptr;

	// ---- Phase 4: Process Desired Cur (FIFO with budget) ----
	g_doingDesired = true;
	int desiredCount = 0;
	while (s_pDesiredHeadCur && desiredCount < MAX_DESIRED_PER_FRAME)
	{
		CBaseEntity* pCurrent = s_pDesiredHeadCur;
		s_pDesiredHeadCur = pCurrent->m_pDesiredNext;
		if (!s_pDesiredHeadCur)
			s_pDesiredTailCur = nullptr;

		pCurrent->m_pDesiredNext = nullptr;
		ClearBits(pCurrent->m_iLFlags, LF_IN_DESIRED_QUEUE);

		if (!IsEntityValidForQueue(pCurrent))
			continue;

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

		desiredCount++;
	}
	g_doingDesired = false;

	// Budget warning (rate-limited)
	if (s_pDesiredHeadCur && gpGlobals->time - s_flLastDesiredWarnTime > 5.0f)
	{
		ALERT(at_warning, "MoveWith Desired queue budget exceeded (%d). Remaining deferred to next frame.\n", MAX_DESIRED_PER_FRAME);
		s_flLastDesiredWarnTime = gpGlobals->time;
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
