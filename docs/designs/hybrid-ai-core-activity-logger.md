# Design: Hybrid AI Core — Activity Logger

Generated on 2026-09-08 (direct planning, user asked to "plan a logger" before implementing)
Branch: ext-ki
Repo: hammermaps/halflife-op4-updated-sohl
Status: APPROVED — implemented 2026-09-08

## Problem Statement

Right now the only visibility into Hybrid AI Core decisions is `ai_hybrid_debug 1`'s console output (`dlls/ai_hybrid.cpp`, `AIHybrid_MaybeLogDebug`) — ephemeral, scrolls away, and only visible while watching the console live during a playtest. There's no durable record to look back at after a session, diff between two runs, or feed into any future automated/replay tooling (the "capture a snapshot, replay it outside the game" idea from the Phase A design doc's Cross-Model Perspective section).

## Scope (confirmed with user)

- **Hybrid AI Core only** — every `DecideAction()` call, not a general game-activity/event log. Damage, spawns, etc. are out of scope; if a broader log is wanted later, it's a separate subsystem.
- **JSON-Lines** format (one JSON object per line) — machine-parseable for later tooling, at the cost of being less pleasant to eyeball raw than plain text (the console `ai_hybrid_debug` output stays for that).

## Design

### New CVars (`dlls/game.h/.cpp`, alongside the existing `ai_hybrid*` block)

- `ai_hybrid_log` (float, default `"0"`) — master switch for file logging. Independent of `ai_hybrid` (behavior) and `ai_hybrid_debug` (console): you can log to file without enabling hybrid behavior or console spam, and vice versa.
- `ai_hybrid_log_file` (string, default `"ai_hybrid_log.jsonl"`) — output path, relative to the mod's working directory (same convention `dlls/stats.cpp`'s `UpdateStatsFile()` already uses in this codebase — proven to work, no new path-resolution logic needed).

### New adapter function (`dlls/ai_hybrid.h/.cpp` — engine-aware layer only; `ai_hybrid_core.h/.cpp` stays untouched)

```cpp
// Appends one JSON-Lines record of this decision to ai_hybrid_log_file,
// gated on ai_hybrid_log >= 1. No-op otherwise. Call once per DecideAction()
// call (i.e. from the same throttled block in PrescheduleThink() - not
// every server frame).
void AIHybrid_MaybeLogActivity(int entityIndex, const char* monsterLabel,
	const AIHybridState& state, const AIDecision& decision, const AIUtilityContext& context);
```

Implementation sketch:
- Lazily open the file (`fopen(ai_hybrid_log_file, "a")`) on first call and keep a static `FILE*` for the process lifetime (avoids reopen overhead per call, unlike `stats.cpp`'s open/write/close-per-call pattern — Hybrid AI decisions happen far more often than stat snapshots). `fflush()` after every write so a crash mid-session doesn't lose the whole log, only matches the cost already accepted for `ALERT()` console output.
- One line per call, fields: `time` (`gpGlobals->time`), `entity` (`STRING(pev->classname)` + entindex, e.g. `"CHGrunt#42"`), `action`/`previous_action` (via `ActionName()`), `score`, `confidence`, `enemy_visible`, `can_range_attack`, `health_ratio`, `ai_hybrid_enabled`.
- Minimal hand-rolled JSON serialization (no library — this codebase has none and pulling one in for a handful of scalar fields would be overkill); escaping isn't a concern since every field is a number, bool, or a controlled enum-name string.

### Call site (`dlls/hgrunt.cpp`)

`PrescheduleThink()`'s existing throttled block already computes everything needed; capture `DecideAction()`'s return value (currently discarded) and pass it straight to the new logger call, right next to the existing `AIHybrid_MaybeLogDebug()` line:

```cpp
const AIDecision decision = DecideAction(m_AIHybridState, hybridContext, AIProfile(), hybridCaps,
	gpGlobals->time, AIHybrid_ConfidenceHalfLife(), AIHybrid_SwitchThreshold());
AIHybrid_MaybeLogActivity(entindex(), "CHGrunt", m_AIHybridState, decision, hybridContext);
```

### Non-goals / deferred

- **Log rotation/size limits** — not handled in this pass; `ai_hybrid_log_file` append-forever is the same behavior `stats.txt` already has in this codebase. Revisit if file size becomes a real problem in practice.
- **Per-session unique filenames** (e.g. timestamped) — deferred; `ai_hybrid_log_file` can be set manually per session if needed (e.g. via an autoexec/launch script) until there's a concrete need to automate it.
- **Actual replay tooling** that reads this log back — out of scope here; this only produces the data.

## Verification

- `scripts/run-ai-hybrid-core-tests.sh` — all pass, unaffected (no core changes; `ai_hybrid_core.h/.cpp` untouched).
- Build + `scripts/diff-baseline.sh reference-builds/6a789e7f-release` — `client.so`/`vgui.so` byte-identical; only `hl.so` differs, as expected.
- JSON-Lines format verified standalone (a plain C program using the exact same `printf` format string, output round-tripped through `json.loads`) — confirmed each line is valid JSON.
- **Still open:** actual manual in-game verification (`ai_hybrid_log 1; ai_hybrid 1; map c2a5f`, then inspecting the generated `.jsonl` for one line per decision tick per grunt) hasn't happened yet — bundled with Phase B's own still-open manual playtest (see `docs/designs/hybrid-ai-core-phase-b.md` Open Questions).
