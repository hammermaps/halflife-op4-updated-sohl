/*
 * Engine-free core for the Hybrid AI decision layer (see AGENTS_HYBRID_AI.md
 * and docs/designs/hybrid-ai-core-phase-a.md).
 *
 * Phase A: framework scaffolding only. There is no tactical decision logic
 * here yet - UpdateSnapshot() always reports AI_ACTION_NONE. This file
 * defines the shared vocabulary and pure helper functions that the
 * engine-aware adapter (ai_hybrid.h/.cpp) and, from Phase B on, the real
 * scoring logic will build on.
 *
 * Nothing in this file may include HLSDK/engine headers (extdll.h, cbase.h,
 * util.h, ...) or use engine types (Vector, EHANDLE, entvars_t, ...). It
 * must stay compilable and testable as a standalone host binary - see
 * tests/ai_hybrid_core_tests.cpp, which compiles only this .cpp and links
 * nothing else.
 */
#pragma once

#include <cstdint>

enum AITacticalAction
{
	AI_ACTION_NONE = 0,

	AI_ACTION_IDLE,
	AI_ACTION_ALERT,
	AI_ACTION_ATTACK,
	AI_ACTION_ADVANCE,
	AI_ACTION_COVER,
	AI_ACTION_SEARCH,
	AI_ACTION_INVESTIGATE,
	AI_ACTION_SUPPRESS,
	AI_ACTION_FLANK_LEFT,
	AI_ACTION_FLANK_RIGHT,
	AI_ACTION_RETREAT,
	AI_ACTION_REPOSITION,
	AI_ACTION_FOLLOW,
	AI_ACTION_MOVE_ASIDE,

	AI_ACTION_COUNT
};

// Stable, human-readable name for debug output and future replay logs.
// Never returns nullptr, even for an out-of-range value.
const char* ActionName(AITacticalAction action);

enum AIHybridCapabilityFlags : uint32_t
{
	AI_CAP_MEMORY = 1u << 0,
	AI_CAP_SEARCH = 1u << 1,
	AI_CAP_COVER = 1u << 2,
	AI_CAP_SQUAD = 1u << 3,
	AI_CAP_FLANK = 1u << 4,
	AI_CAP_SUPPRESSION = 1u << 5,
	AI_CAP_RETREAT = 1u << 6,
	AI_CAP_FORMATION = 1u << 7,
	AI_CAP_MOVE_ASIDE = 1u << 8,
	AI_CAP_LOCAL_AVOID = 1u << 9,
	AI_CAP_MORALE = 1u << 10,
	AI_CAP_PERSONALITY = 1u << 11,
};

// True if an NPC with the given capability mask is allowed to select
// `action`. Actions with no corresponding capability flag (ATTACK, ADVANCE,
// REPOSITION, FOLLOW, plus the inert NONE/IDLE/ALERT) are baseline and
// always supported; everything else requires its matching AI_CAP_* bit.
bool ActionSupported(AITacticalAction action, uint32_t capabilityMask);

// Per-NPC-archetype tuning (see AGENTS_HYBRID_AI.md section 24). Not
// consumed by any decision logic in Phase A; exists so later phases and
// save/restore have a stable type to construct against from the start.
struct AIProfile
{
	float aggression = 0.5f;
	float caution = 0.5f;
	float bravery = 0.5f;
	float teamwork = 0.5f;
	float mobility = 0.5f;
};

// Clamps into [0, 100], the utility-score range used from Phase B onward.
// NaN maps to 0 - a NaN score must never win a comparison.
float ClampScore(float value);

// Clamps into [0, 1]. NaN maps to 0.
float Clamp01(float value);

// Exponential confidence decay: `confidence` halves every `halfLifeSeconds`
// of `elapsedSeconds` that pass. Used by the enemy-memory model from Phase B
// onward; exposed now so it has test coverage from the start.
// elapsedSeconds <= 0 returns `confidence` unchanged (no time passed, so no
// decay - never let a caller accidentally increase confidence this way).
// halfLifeSeconds <= 0 with elapsedSeconds > 0 returns 0 (an invalid/zero
// half-life means "decays instantly").
float DecayConfidence(float confidence, float elapsedSeconds, float halfLifeSeconds);

// Minimal, inert per-NPC state. Phase A only ever holds AI_ACTION_NONE in
// both fields - this struct exists so a monster class has a stable member
// to zero-initialize and observe from PrescheduleThink(), instead of
// hand-waving "observe" as a no-op comment. Deliberately not part of
// m_SaveData yet (see AGENTS_HYBRID_AI.md section 38): nothing here is
// meaningful yet, so it needs no save/restore support until Phase B gives
// it real state.
struct AIHybridState
{
	AITacticalAction currentAction = AI_ACTION_NONE;
	AITacticalAction previousAction = AI_ACTION_NONE;
};

// Advances `state` by one decision tick. Phase A performs no decision-making
// - this only rotates current into previous and always sets current back to
// AI_ACTION_NONE, so the call site in PrescheduleThink() has real plumbing
// to invoke.
void UpdateSnapshot(AIHybridState& state);
