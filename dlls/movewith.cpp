// LRC - MoveWith system implementation

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "movewith.h"

#include <algorithm>
#include <vector>

//=========================================================
// Non-intrusive FIFO queue state — two-batch (current + next)
// for both PostAssist and Desired queues.
// Static in this TU; not saved/restored (rebuilt each frame).
//=========================================================

static std::vector<CBaseEntity*> s_PostAssistCur;
static std::vector<CBaseEntity*> s_PostAssistNext;
static std::vector<CBaseEntity*> s_DesiredCur;
static std::vector<CBaseEntity*> s_DesiredNext;

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
	s_PostAssistCur.clear();
	s_PostAssistNext.clear();
	s_DesiredCur.clear();
	s_DesiredNext.clear();

	s_flLastPostAssistWarnTime = 0;
	s_flLastDesiredWarnTime = 0;

	g_doingDesired = false;
}


//=========================================================
// MoveWith_RemoveEntityFromQueues
// Remove an entity from all queues — call from SUB_Remove
// to prevent processing of freed entities.
//=========================================================
void MoveWith_RemoveEntityFromQueues(CBaseEntity* pEnt)
{
	if (!pEnt)
		return;

	// For the "Cur" queues (which may be actively iterated by index inside
	// MoveWith_ProcessFrameQueues when this is called from DesiredAction/Think),
	// we must nullify rather than erase to avoid shifting indices mid-iteration.
	// Null entries are compacted at the end of each processing pass via std::remove.
	//
	// For the "Next" queues (never iterated during processing), we can erase
	// immediately to free the slot.

	for (auto& p : s_PostAssistCur)
		if (p == pEnt) p = nullptr;
	for (auto& p : s_DesiredCur)
		if (p == pEnt) p = nullptr;

	s_PostAssistNext.erase(
		std::remove(s_PostAssistNext.begin(), s_PostAssistNext.end(), pEnt),
		s_PostAssistNext.end());
	s_DesiredNext.erase(
		std::remove(s_DesiredNext.begin(), s_DesiredNext.end(), pEnt),
		s_DesiredNext.end());

	ClearBits(pEnt->m_iLFlags, LF_DOASSIST | LF_DODESIRED);
}


//=========================================================
// Internal: enqueue helpers — append to next batch (FIFO)
//=========================================================
static void EnqueuePostAssistNext(CBaseEntity* pEnt)
{
	// Duplicate guard: check if already in either queue
	for (CBaseEntity* p : s_PostAssistNext)
		if (p == pEnt)
			return;
	for (CBaseEntity* p : s_PostAssistCur)
		if (p == pEnt)
			return;

	s_PostAssistNext.push_back(pEnt);
}

static void EnqueueDesiredNext(CBaseEntity* pEnt)
{
	// Duplicate guard: check if already in either queue
	for (CBaseEntity* p : s_DesiredNext)
		if (p == pEnt)
			return;
	for (CBaseEntity* p : s_DesiredCur)
		if (p == pEnt)
			return;

	s_DesiredNext.push_back(pEnt);
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
	// Append any remaining Cur entries (budget leftovers) before Next.
	if (!s_PostAssistCur.empty())
	{
		// Prepend leftover Cur to Next, then swap
		s_PostAssistNext.insert(s_PostAssistNext.begin(),
			s_PostAssistCur.begin(), s_PostAssistCur.end());
	}
	s_PostAssistCur.clear();
	std::swap(s_PostAssistCur, s_PostAssistNext);

	// ---- Phase 2: Process PostAssist Cur (FIFO with budget) ----
	int postAssistCount = 0;
	for (int i = 0; i < (int)s_PostAssistCur.size() && postAssistCount < MAX_POSTASSIST_PER_FRAME; i++)
	{
		CBaseEntity* pCurrent = s_PostAssistCur[i];
		s_PostAssistCur[i] = nullptr; // clear as we process

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

	// Remove processed (null) entries, keep unprocessed (budget overflow)
	s_PostAssistCur.erase(
		std::remove(s_PostAssistCur.begin(), s_PostAssistCur.end(), nullptr),
		s_PostAssistCur.end());

	// Budget warning (rate-limited)
	if (!s_PostAssistCur.empty() && gpGlobals->time - s_flLastPostAssistWarnTime > 5.0f)
	{
		ALERT(at_warning, "MoveWith PostAssist queue budget exceeded (%d). Remaining deferred to next frame.\n", MAX_POSTASSIST_PER_FRAME);
		s_flLastPostAssistWarnTime = gpGlobals->time;
	}

	// ---- Phase 3: Swap Desired batches ----
	if (!s_DesiredCur.empty())
	{
		s_DesiredNext.insert(s_DesiredNext.begin(),
			s_DesiredCur.begin(), s_DesiredCur.end());
	}
	s_DesiredCur.clear();
	std::swap(s_DesiredCur, s_DesiredNext);

	// ---- Phase 4: Process Desired Cur (FIFO with budget) ----
	g_doingDesired = true;
	int desiredCount = 0;
	for (int i = 0; i < (int)s_DesiredCur.size() && desiredCount < MAX_DESIRED_PER_FRAME; i++)
	{
		CBaseEntity* pCurrent = s_DesiredCur[i];
		s_DesiredCur[i] = nullptr; // clear as we process

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

	// Remove processed (null) entries, keep unprocessed (budget overflow)
	s_DesiredCur.erase(
		std::remove(s_DesiredCur.begin(), s_DesiredCur.end(), nullptr),
		s_DesiredCur.end());

	// Budget warning (rate-limited)
	if (!s_DesiredCur.empty() && gpGlobals->time - s_flLastDesiredWarnTime > 5.0f)
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
