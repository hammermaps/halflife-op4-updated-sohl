/*
 * Engine-aware adapter for the Hybrid AI Core (see AGENTS_HYBRID_AI.md,
 * docs/designs/hybrid-ai-core-phase-a.md, -phase-b.md and -phase-c.md).
 *
 * This is the only file in the ai_hybrid* subsystem allowed to include
 * HLSDK/engine headers. It owns the ai_hybrid* CVars and bounded debug
 * output; the actual decision core stays in ai_hybrid_core.h/.cpp so it can
 * be built and tested without any engine dependency.
 */
#pragma once

#include "ai_hybrid_core.h"

// True if the Hybrid AI master switch (`ai_hybrid` CVar) is enabled. When
// false, callers must not deviate from original behavior.
bool AIHybrid_Enabled();

// How often (in seconds) a monster should re-run DecideAction() rather than
// reusing last tick's result - `ai_hybrid_decision_interval` CVar (default
// 0.25s, within the combat range AGENTS_HYBRID_AI.md section 14 suggests).
float AIHybrid_DecisionInterval();

// Confidence half-life in seconds for DecayConfidence() - `ai_hybrid_confidence_halflife`
// CVar (default 8s, matching the "usable for search ~8-15s" guidance in
// AGENTS_HYBRID_AI.md section 43).
float AIHybrid_ConfidenceHalfLife();

// Hysteresis switch threshold passed to DecideAction() - not a CVar yet
// (Phase B keeps it a fixed constant; expose it later if tuning needs arise).
float AIHybrid_SwitchThreshold();

// How recent (in seconds) a squad's last enemy sighting must be, per
// CSquadMonster::m_flLastEnemySightTime, for a squad member without direct
// sight to still count as squad-informed (AIMemorySource::AI_MEMORY_SOURCE_SQUAD_SHARED)
// rather than knowing nothing - `ai_hybrid_squad_report_recency` CVar
// (default 5s, matching the recency window the engine's own pre-existing
// m_fEnemyEluded logic already uses).
float AIHybrid_SquadReportRecency();

// Prints `state`'s current/previous action and enemy-memory confidence for
// `monsterLabel` to the console, gated on `ai_hybrid_debug >= 1`. No-op
// otherwise (including when AIHybrid_Enabled() is false, so the debug output
// can be checked even while the master switch stays off).
void AIHybrid_MaybeLogDebug(const AIHybridState& state, const char* monsterLabel);

// Appends one JSON-Lines record of this decision to the `ai_hybrid_log_file`
// CVar's path, gated on `ai_hybrid_log >= 1` (independent of `ai_hybrid` and
// `ai_hybrid_debug` - see docs/designs/hybrid-ai-core-activity-logger.md).
// No-op otherwise. Call once per DecideAction() call, not every server
// frame - see the throttled call site in CHGrunt::PrescheduleThink().
void AIHybrid_MaybeLogActivity(int entityIndex, const char* monsterLabel,
	const AIHybridState& state, const AIDecision& decision, const AIUtilityContext& context);
