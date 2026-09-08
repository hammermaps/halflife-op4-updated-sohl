/*
 * Engine-free core for the Hybrid AI decision layer (see AGENTS_HYBRID_AI.md,
 * docs/designs/hybrid-ai-core-phase-a.md, -phase-b.md and -phase-c.md).
 *
 * Phase C teaches the confidence model to distinguish direct sight from
 * squad-shared knowledge (sections 8-9, 19) - HEARD/INFERRED sources still
 * stay unimplemented (no soundent integration yet). Squad *position*
 * sharing and role/slot assignment are NOT reimplemented here:
 * CSquadMonster already shares m_vecEnemyLKP and "anyone in the squad has
 * seen the enemy" timing through the leader, and squad slots already exist
 * for role-like behavior - see docs/designs/hybrid-ai-core-phase-c.md for
 * what was actually found there before writing this phase.
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

// --- Enemy memory (AGENTS_HYBRID_AI.md sections 8-9) ---
//
// Position itself is NOT stored here: the engine already tracks it via
// CBaseMonster::m_vecEnemyLKP (and, for squads, CSquadMonster already
// shares it through the leader), so the hybrid core only needs to know
// *how sure* the NPC still is, not *where*.

enum AIConfidenceLevel
{
	AI_CONFIDENCE_LOST,       // <= 0
	AI_CONFIDENCE_WEAK,       // < 0.20
	AI_CONFIDENCE_UNCERTAIN,  // < 0.45
	AI_CONFIDENCE_PROBABLE,   // < 0.75
	AI_CONFIDENCE_CONFIRMED,  // >= 0.75
};

// Named thresholds from AGENTS_HYBRID_AI.md section 9, so callers never
// hardcode the raw numbers.
inline constexpr float AI_CONFIDENCE_CONFIRMED_THRESHOLD = 0.75f;
inline constexpr float AI_CONFIDENCE_PROBABLE_THRESHOLD = 0.45f;
inline constexpr float AI_CONFIDENCE_UNCERTAIN_THRESHOLD = 0.20f;

// "Recent squad report: confidence = max(confidence, 0.7)" (section 9) -
// squad-shared knowledge is never as strong as seeing the enemy yourself,
// but it doesn't dip below this floor either while it's still being shared.
inline constexpr float AI_CONFIDENCE_SQUAD_REPORT_FLOOR = 0.7f;

AIConfidenceLevel ClassifyConfidence(float confidence);

// Where this tick's enemy-memory update came from (section 8: "the AI must
// distinguish DIRECT_SIGHT / HEARD / SQUAD_SHARED / INFERRED... squad
// information is never identical to direct sight"). HEARD/INFERRED aren't
// implemented yet - no soundent integration in this phase.
enum AIMemorySource
{
	AI_MEMORY_SOURCE_NONE = 0,      // nothing new this tick - only decay applies
	AI_MEMORY_SOURCE_DIRECT_SIGHT,  // strongest: confidence set to exactly 1.0
	AI_MEMORY_SOURCE_SQUAD_SHARED,  // confidence floored at AI_CONFIDENCE_SQUAD_REPORT_FLOOR
};

struct AIEnemyMemory
{
	bool enemyKnown = false;   // true once any sighting has ever occurred
	float confidence = 0.0f;   // [0, 1], see AIConfidenceLevel
	// Decay anchor: the `now` value as of the last UpdateEnemyMemory() call
	// that touched this memory (any source, including AI_MEMORY_SOURCE_NONE
	// while merely decaying) - NOT literally "last time seen". Advances every
	// call so repeated decay steps compose correctly instead of double-
	// decaying the same elapsed window. Caller's time unit (e.g.
	// gpGlobals->time); -1 = never updated.
	float lastSeenTime = -1.0f;
};

// Advances `memory` by one decision tick given `source`. Direct sight jumps
// confidence to 1.0 immediately (strongest source, per section 9).
// Squad-shared decays confidence as usual first, then floors it at
// AI_CONFIDENCE_SQUAD_REPORT_FLOOR - it can raise a lower confidence but
// never lowers one that's already higher (e.g. from a very recent direct
// sighting). AI_MEMORY_SOURCE_NONE only decays existing knowledge; if
// nothing has ever been known, it's a no-op. `now` must be non-decreasing
// between calls (matches gpGlobals->time).
void UpdateEnemyMemory(AIEnemyMemory& memory, AIMemorySource source, float now, float confidenceHalfLifeSeconds);

// --- Utility scoring (Phase B: ATTACK/COVER/SEARCH only) ---
//
// Inputs deliberately stay minimal - only what Phase B's engine adapter can
// cheaply provide from existing HLSDK conditions (hgrunt.cpp), not the full
// AIUtilityContext envisioned by the spec (danger, squad state, grenade
// awareness, ... all deferred to the phases that actually use them).

struct AIUtilityContext
{
	bool enemyVisible = false;    // HasConditions(bits_COND_SEE_ENEMY)
	bool canRangeAttack = false;  // HasConditions(bits_COND_CAN_RANGE_ATTACK1/2)
	float healthRatio = 1.0f;     // pev->health / pev->max_health, clamped [0,1]
};

// Each Score* function is pure and independently testable. All return a
// value in [0, 100] (via ClampScore) or exactly 0 when the action plainly
// doesn't apply (e.g. attacking an enemy that isn't visible).
float ScoreAttack(const AIUtilityContext& context, const AIEnemyMemory& memory, const AIProfile& profile);
float ScoreCover(const AIUtilityContext& context, const AIEnemyMemory& memory, const AIProfile& profile);
float ScoreSearch(const AIUtilityContext& context, const AIEnemyMemory& memory, const AIProfile& profile);

struct AIDecision
{
	AITacticalAction action = AI_ACTION_NONE;
	float score = 0.0f;
};

// Per-NPC, per-tick tactical state. Phase B state.enemyMemory carries real
// data; currentAction/previousAction now reflect DecideAction()'s actual
// choice instead of always being NONE. Deliberately still not part of
// m_SaveData (see AGENTS_HYBRID_AI.md section 38): losing this on load just
// means one tick of "forgot the enemy" until the next sighting/decision
// re-establishes it, which is safe by construction (never forces movement,
// never crashes) - revisit if that proves noticeable in practice.
struct AIHybridState
{
	AITacticalAction currentAction = AI_ACTION_NONE;
	AITacticalAction previousAction = AI_ACTION_NONE;
	AIEnemyMemory enemyMemory;
};

// Updates `state.enemyMemory` from `memorySource` (see AIMemorySource -
// independent of `context.enemyVisible`, which only gates ScoreAttack: a
// squad-shared-only sighting must never be enough to attack), scores every
// action `capabilityMask` supports, and picks a winner with hysteresis: the
// previous action is kept unless a different action beats its score by more
// than `switchThreshold` (AGENTS_HYBRID_AI.md section 13 - prevents rapid
// flapping between near-tied scores). Updates state.currentAction /
// state.previousAction and returns the decision.
AIDecision DecideAction(AIHybridState& state, const AIUtilityContext& context, const AIProfile& profile,
	uint32_t capabilityMask, AIMemorySource memorySource, float now, float confidenceHalfLifeSeconds,
	float switchThreshold);
