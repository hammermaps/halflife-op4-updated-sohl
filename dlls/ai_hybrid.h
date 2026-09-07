/*
 * Engine-aware adapter for the Hybrid AI Core (see AGENTS_HYBRID_AI.md and
 * docs/designs/hybrid-ai-core-phase-a.md).
 *
 * This is the only file in the ai_hybrid* subsystem allowed to include
 * HLSDK/engine headers. It owns the ai_hybrid* CVars and bounded debug
 * output; the actual decision core stays in ai_hybrid_core.h/.cpp so it can
 * be built and tested without any engine dependency.
 */
#pragma once

#include "ai_hybrid_core.h"

// True if the Hybrid AI master switch (`ai_hybrid` CVar) is enabled. Phase A
// callers must not change behavior based on this yet - it exists so later
// phases have a single place to check, and so the debug output below can
// note whether the subsystem is armed.
bool AIHybrid_Enabled();

// Prints `state`'s current/previous action for `monsterLabel` to the
// console, gated on `ai_hybrid_debug >= 1`. No-op otherwise (including when
// AIHybrid_Enabled() is false - Phase A logs unconditionally on the debug
// CVar so it can be verified even while the master switch stays off).
void AIHybrid_MaybeLogDebug(const AIHybridState& state, const char* monsterLabel);
