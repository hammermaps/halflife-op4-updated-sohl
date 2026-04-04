// LRC - MoveWith system implementation

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "movewith.h"
#include "debug.h"
#include "logger.h"
#include "game.h"
#include "weapons.h"

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
		LOG_WARNING("MoveWith PostAssist queue budget exceeded (%d). Remaining deferred to next frame.", MAX_POSTASSIST_PER_FRAME);
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
		LOG_WARNING("MoveWith Desired queue budget exceeded (%d). Remaining deferred to next frame.", MAX_DESIRED_PER_FRAME);
		s_flLastDesiredWarnTime = gpGlobals->time;
	}

	// Draw debug beams when sohl_mw_debug >= 2
	MoveWith_DrawDebugBeams();
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

	DEV_LOG(mw_debug, "AssignOrigin: %s (%.1f %.1f %.1f)",
		STRING(pEnt->pev->classname), vecOrigin.x, vecOrigin.y, vecOrigin.z);

	int loopbreaker = MAX_MOVEWITH_DEPTH;
	CBaseEntity* pChild = pEnt->m_pChildMoveWith;
	while (pChild)
	{
		if (loopbreaker-- <= 0)
		{
			LOG_ERROR("MoveWith chain overflow in UTIL_AssignOrigin!");
			break;
		}
		Vector vecChildOrigin = vecOrigin + pChild->m_vecMoveWithOffset;
		DEV_LOG(mw_debug, "  child %s offset (%.1f %.1f %.1f) -> origin (%.1f %.1f %.1f)",
			STRING(pChild->pev->classname),
			pChild->m_vecMoveWithOffset.x, pChild->m_vecMoveWithOffset.y, pChild->m_vecMoveWithOffset.z,
			vecChildOrigin.x, vecChildOrigin.y, vecChildOrigin.z);
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

	DEV_LOG(mw_debug, "SetVelocity: %s vel (%.1f %.1f %.1f)",
		STRING(pEnt->pev->classname), vecSet.x, vecSet.y, vecSet.z);

	int loopbreaker = MAX_MOVEWITH_DEPTH;
	CBaseEntity* pChild = pEnt->m_pChildMoveWith;
	while (pChild)
	{
		if (loopbreaker-- <= 0)
		{
			LOG_ERROR("MoveWith chain overflow in UTIL_SetVelocity!");
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
					DEV_LOG(mw_debug, "  child %s has no think! Setting SUB_DoNothing to enable velocity",
						STRING(pChild->pev->classname));
					pChild->SetThink(&CBaseEntity::SUB_DoNothing);
					pChild->SetNextThink(0.01);
				}
			}
		}

		DEV_LOG(mw_debug, "  propagate vel to child %s", STRING(pChild->pev->classname));
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

	DEV_LOG(mw_debug, "SetAvelocity: %s avel (%.1f %.1f %.1f)",
		STRING(pEnt->pev->classname), vecSet.x, vecSet.y, vecSet.z);

	int loopbreaker = MAX_MOVEWITH_DEPTH;
	CBaseEntity* pChild = pEnt->m_pChildMoveWith;
	while (pChild)
	{
		if (loopbreaker-- <= 0)
		{
			LOG_ERROR("MoveWith chain overflow in UTIL_SetAvelocity!");
			break;
		}
		DEV_LOG(mw_debug, "  propagate avel to child %s", STRING(pChild->pev->classname));
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

	DEV_LOG(mw_debug, "SetAngles: %s angles (%.1f %.1f %.1f)",
		STRING(pEnt->pev->classname), vecSet.x, vecSet.y, vecSet.z);

	int loopbreaker = MAX_MOVEWITH_DEPTH;
	CBaseEntity* pChild = pEnt->m_pChildMoveWith;
	while (pChild)
	{
		if (loopbreaker-- <= 0)
		{
			LOG_ERROR("MoveWith chain overflow in UTIL_SetAngles!");
			break;
		}
		Vector vecChildAngles = vecSet + pChild->m_vecRotWithOffset;
		DEV_LOG(mw_debug, "  child %s rot-offset (%.1f %.1f %.1f) -> angles (%.1f %.1f %.1f)",
			STRING(pChild->pev->classname),
			pChild->m_vecRotWithOffset.x, pChild->m_vecRotWithOffset.y, pChild->m_vecRotWithOffset.z,
			vecChildAngles.x, vecChildAngles.y, vecChildAngles.z);
		UTIL_SetAngles(pChild, vecChildAngles);
		pChild = pChild->m_pSiblingMoveWith;
	}
}


//=========================================================
// MoveWith_DebugDump
// Print the full MoveWith hierarchy to the calling player's
// console.  Requires sv_cheats 1.
// Usage in-game: movewith_debug
//=========================================================
static void PrintChildTree(edict_t* pClient, CBaseEntity* pEnt, int depth)
{
	if (!pEnt || depth > MAX_MOVEWITH_DEPTH)
		return;

	// indent (cap visual indentation to keep output readable)
	static constexpr int MAX_INDENT = 32;
	char indent[MAX_INDENT * 2 + 1];
	int clampedDepth = (depth < MAX_INDENT) ? depth : MAX_INDENT;
	int i = 0;
	for (; i < clampedDepth * 2; i++)
		indent[i] = ' ';
	indent[i] = '\0';

	const char* name = pEnt->pev->targetname ? STRING(pEnt->pev->targetname) : "<no name>";
	const char* cls  = STRING(pEnt->pev->classname);
	const char* mdl  = pEnt->pev->model ? STRING(pEnt->pev->model) : "";

	CLIENT_PRINTF(pClient, print_console,
		UTIL_VarArgs("%s[%s] %s (%s) origin(%.1f %.1f %.1f) vel(%.1f %.1f %.1f) movetype %d\n",
			indent, cls, name, mdl,
			pEnt->pev->origin.x, pEnt->pev->origin.y, pEnt->pev->origin.z,
			pEnt->pev->velocity.x, pEnt->pev->velocity.y, pEnt->pev->velocity.z,
			pEnt->pev->movetype));

	if (depth > 0)
	{
		CLIENT_PRINTF(pClient, print_console,
			UTIL_VarArgs("%s  offset(%.1f %.1f %.1f) rot-offset(%.1f %.1f %.1f) lflags 0x%x\n",
				indent,
				pEnt->m_vecMoveWithOffset.x, pEnt->m_vecMoveWithOffset.y, pEnt->m_vecMoveWithOffset.z,
				pEnt->m_vecRotWithOffset.x, pEnt->m_vecRotWithOffset.y, pEnt->m_vecRotWithOffset.z,
				pEnt->m_iLFlags));

		// Check if offset matches actual position difference with parent
		if (pEnt->m_pMoveWith)
		{
			Vector expectedOrigin = pEnt->m_pMoveWith->pev->origin + pEnt->m_vecMoveWithOffset;
			Vector diff = pEnt->pev->origin - expectedOrigin;
			float drift = diff.Length();
			if (drift > 1.0f)
			{
				CLIENT_PRINTF(pClient, print_console,
					UTIL_VarArgs("%s  *** DRIFT %.1f units! expected(%.1f %.1f %.1f) actual(%.1f %.1f %.1f)\n",
						indent, drift,
						expectedOrigin.x, expectedOrigin.y, expectedOrigin.z,
						pEnt->pev->origin.x, pEnt->pev->origin.y, pEnt->pev->origin.z));
			}
		}
	}

	// recurse into children
	CBaseEntity* pChild = pEnt->m_pChildMoveWith;
	while (pChild)
	{
		PrintChildTree(pClient, pChild, depth + 1);
		pChild = pChild->m_pSiblingMoveWith;
	}
}

void MoveWith_DebugDump(CBaseEntity* pPlayer)
{
	if (!pPlayer)
		return;

	edict_t* pClient = pPlayer->edict();
	int totalRoots = 0;
	int totalChildren = 0;
	int totalOrphans = 0;

	CLIENT_PRINTF(pClient, print_console, "--- MoveWith Hierarchy Dump ---\n");

	// Walk all entities, find roots (have children but no parent)
	for (int i = 1; i < gpGlobals->maxEntities; i++)
	{
		edict_t* pEdict = INDEXENT(i);
		if (!pEdict || pEdict->free)
			continue;

		CBaseEntity* pEnt = CBaseEntity::Instance(pEdict);
		if (!pEnt)
			continue;

		// Root: has children but no parent
		if (pEnt->m_pChildMoveWith && !pEnt->m_pMoveWith)
		{
			totalRoots++;
			CLIENT_PRINTF(pClient, print_console, "\n");
			PrintChildTree(pClient, pEnt, 0);
		}
	}

	// Second pass: find orphans (have m_MoveWith set but no resolved m_pMoveWith)
	for (int i = 1; i < gpGlobals->maxEntities; i++)
	{
		edict_t* pEdict = INDEXENT(i);
		if (!pEdict || pEdict->free)
			continue;

		CBaseEntity* pEnt = CBaseEntity::Instance(pEdict);
		if (!pEnt)
			continue;

		if (pEnt->m_MoveWith && !pEnt->m_pMoveWith)
		{
			totalOrphans++;
			CLIENT_PRINTF(pClient, print_console,
				UTIL_VarArgs("\n*** ORPHAN: [%s] %s wanted movewith '%s' but parent not found!\n",
					STRING(pEnt->pev->classname),
					pEnt->pev->targetname ? STRING(pEnt->pev->targetname) : "<no name>",
					STRING(pEnt->m_MoveWith)));
		}

		if (pEnt->m_pMoveWith)
			totalChildren++;
	}

	CLIENT_PRINTF(pClient, print_console,
		UTIL_VarArgs("\n--- Summary: %d root(s), %d child(ren), %d orphan(s) ---\n",
			totalRoots, totalChildren, totalOrphans));

	// Queue stats
	CLIENT_PRINTF(pClient, print_console,
		UTIL_VarArgs("PostAssist queue: cur=%d next=%d | Desired queue: cur=%d next=%d\n",
			(int)s_PostAssistCur.size(), (int)s_PostAssistNext.size(),
			(int)s_DesiredCur.size(), (int)s_DesiredNext.size()));
}


//=========================================================
// MoveWith_DrawDebugBeams
// When sohl_mw_debug >= 2, draw temporary beams between
// each parent and its MoveWith children.
// Call once per server frame from MoveWith_ProcessFrameQueues.
//=========================================================
void MoveWith_DrawDebugBeams()
{
	if (mw_debug.value < 2)
		return;

	for (int i = 1; i < gpGlobals->maxEntities; i++)
	{
		edict_t* pEdict = INDEXENT(i);
		if (!pEdict || pEdict->free)
			continue;

		CBaseEntity* pEnt = CBaseEntity::Instance(pEdict);
		if (!pEnt || !pEnt->m_pChildMoveWith)
			continue;

		CBaseEntity* pChild = pEnt->m_pChildMoveWith;
		while (pChild)
		{
			// Green beam from parent to child
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMPOINTS);
			WRITE_COORD(pEnt->pev->origin.x);
			WRITE_COORD(pEnt->pev->origin.y);
			WRITE_COORD(pEnt->pev->origin.z);
			WRITE_COORD(pChild->pev->origin.x);
			WRITE_COORD(pChild->pev->origin.y);
			WRITE_COORD(pChild->pev->origin.z);
			WRITE_SHORT(g_sModelIndexLaser);
			WRITE_BYTE(0);   // frame start
			WRITE_BYTE(0);   // framerate
			WRITE_BYTE(1);   // life (0.1s)
			WRITE_BYTE(4);   // width
			WRITE_BYTE(0);   // noise
			WRITE_BYTE(0);   // r
			WRITE_BYTE(255); // g
			WRITE_BYTE(0);   // b
			WRITE_BYTE(200); // brightness
			WRITE_BYTE(0);   // speed
			MESSAGE_END();

			// Check for positional drift — red beam if child drifted > 1 unit
			Vector expectedOrigin = pEnt->pev->origin + pChild->m_vecMoveWithOffset;
			Vector diff = pChild->pev->origin - expectedOrigin;
			if (diff.Length() > 1.0f)
			{
				// Red beam from expected position to actual position
				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
				WRITE_BYTE(TE_BEAMPOINTS);
				WRITE_COORD(expectedOrigin.x);
				WRITE_COORD(expectedOrigin.y);
				WRITE_COORD(expectedOrigin.z);
				WRITE_COORD(pChild->pev->origin.x);
				WRITE_COORD(pChild->pev->origin.y);
				WRITE_COORD(pChild->pev->origin.z);
				WRITE_SHORT(g_sModelIndexLaser);
				WRITE_BYTE(0);   // frame start
				WRITE_BYTE(0);   // framerate
				WRITE_BYTE(1);   // life (0.1s)
				WRITE_BYTE(8);   // width (thicker for errors)
				WRITE_BYTE(0);   // noise
				WRITE_BYTE(255); // r
				WRITE_BYTE(0);   // g
				WRITE_BYTE(0);   // b
				WRITE_BYTE(255); // brightness
				WRITE_BYTE(0);   // speed
				MESSAGE_END();
			}

			pChild = pChild->m_pSiblingMoveWith;
		}
	}
}
