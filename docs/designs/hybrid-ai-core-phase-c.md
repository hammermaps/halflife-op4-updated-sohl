# Design: Hybrid AI Core — Phase C (Squad-Shared Enemy Memory)

Generated on 2026-09-08 (direct implementation, user said "weiter" after Phase B + logger were playtest-confirmed)
Branch: ext-ki
Repo: hammermaps/halflife-op4-updated-sohl
Status: APPROVED
Supersedes: none (extends Phase B)

## Scope

Per `AGENTS_HYBRID_AI.md` §19 ("check how `CSquadMonster` currently manages leader/member... prefer storing the blackboard on the squad leader or in the existing squad structure... no global manager") and §49 (Phase C acceptance: one grunt sees the player, others receive *approximate* knowledge, not direct sight; roles don't duplicate unnecessarily; leader death handled).

**Investigated first, per the spec's own instruction:** `dlls/squadmonster.h/.cpp` already implements almost everything §19's `AISquadBlackboard` sketch asks for:
- `CSquadMonster::m_vecEnemyLKP` is already shared through the leader via `SquadPasteEnemyInfo()`/`SquadCopyEnemyInfo()`, called from `CheckEnemy()` every condition-gathering pass.
- `CSquadMonster::m_flLastEnemySightTime` is already documented in the header as "last time **anyone in the squad** saw the enemy" and is already updated by `CHGrunt::PrescheduleThink()`'s pre-existing (non-hybrid) code whenever `bits_COND_SEE_ENEMY` is set.
- `MySquadLeader()` already degrades gracefully to `this` when there's no leader (`InSquad()` false), and `Killed()` → `SquadRemove()` already disbands the squad cleanly when the leader dies (nulls every member's `m_hSquadLeader`), so **"leader death handled" needs no new code** — it falls out of existing, decades-old engine mechanics that `InSquad()`/`MySquadLeader()` already respect.
- Squad "roles" already exist as the existing slot system (`bits_SLOT_HGRUNT_ENGAGE1/2`, `OccupySlot`/`VacateSlot`) — per spec §20 ("prefer reusing existing squad slots"), this is not rebuilt. Formal utility-AI role assignment (PRIMARY/FLANK/SUPPRESS/RESERVE) is deferred to whichever phase adds the FLANK/SUPPRESS actions those roles are actually *for* — assigning a role with no corresponding action to execute would be dead weight now.

**So this phase's actual new work is narrow:** teach the Hybrid AI Core's confidence model to distinguish "I see the enemy myself" from "my squad still knows where they are, but I don't personally," per spec §8-9 ("squad information is never identical to direct sight"; "recent squad report: confidence = max(confidence, 0.7)").

## Key decisions

1. **`AIMemorySource` replaces the bare `bool enemyVisible` input to `UpdateEnemyMemory()`/`DecideAction()`.** Values: `AI_MEMORY_SOURCE_NONE`, `AI_MEMORY_SOURCE_DIRECT_SIGHT`, `AI_MEMORY_SOURCE_SQUAD_SHARED` (HEARD/INFERRED stay unimplemented — no soundent integration yet, deferred). Direct sight still sets confidence to exactly 1.0; squad-shared floors confidence at a new named constant `AI_CONFIDENCE_SQUAD_REPORT_FLOOR` (0.7, per spec §9) without exceeding whatever direct-sight-derived confidence is already higher. `AIUtilityContext` (visibility/attack/health) is untouched — memory source is a separate `DecideAction()` parameter, not mixed into utility-scoring inputs, since "can I see them" (for `ScoreAttack`'s gate) and "how did I learn about them" (for confidence) are different questions.
2. **Source computed entirely in `dlls/hgrunt.cpp`**, not the core: `AI_MEMORY_SOURCE_DIRECT_SIGHT` when `bits_COND_SEE_ENEMY`; else `AI_MEMORY_SOURCE_SQUAD_SHARED` when `InSquad()` and the squad leader's `m_flLastEnemySightTime` is within `ai_hybrid_squad_report_recency` seconds (new CVar, default 5s — matching the recency window the engine's own pre-existing `m_fEnemyEluded` logic already uses); else `AI_MEMORY_SOURCE_NONE`. This keeps `ai_hybrid_core.h/.cpp` free of any squad/engine concept, same layering as every prior phase.
3. **No new capability-gating logic for this.** `AI_CAP_SQUAD` is added to `CHGrunt`'s capability mask for documentation/future use, but the actual gate is `InSquad()` (engine/entity state the core can't see anyway) — consistent with how `COVER`/`SEARCH` are already gated by an `m_hEnemy != NULL` engine check in `ResolveHybridSchedule()`, not by a core-side capability check alone.
4. **No schedule/action changes.** A squad-shared-only sighting still only ever resolves to `AI_ACTION_SEARCH` (never `AI_ACTION_ATTACK` — `ScoreAttack` requires `context.enemyVisible`, i.e. direct sight, unchanged from Phase B) — matching the acceptance criterion "others receive approximate knowledge, not direct sight." `ResolveHybridSchedule(AI_ACTION_SEARCH)` already maps to `SCHED_CHASE_ENEMY`, which paths toward `m_vecEnemyLKP` — already kept fresh by the engine's own `SquadCopyEnemyInfo()`, so a squad-informed grunt genuinely moves toward the right place with zero new position-sharing code.

## What's implemented

- `dlls/ai_hybrid_core.h/.cpp`: `AIMemorySource` enum, `AI_CONFIDENCE_SQUAD_REPORT_FLOOR` constant, `UpdateEnemyMemory()` signature changed to take `AIMemorySource` (direct sight → 1.0; squad-shared → decay-then-floor at 0.7; none → decay only, no-op if never known). `DecideAction()` gains an `AIMemorySource memorySource` parameter, threaded straight through to `UpdateEnemyMemory()`. Tests updated for the new signature plus new squad-floor cases.
- `dlls/game.h/.cpp`: new CVar `ai_hybrid_squad_report_recency` (default `"5"`).
- `dlls/hgrunt.cpp`: computes `AIMemorySource` from `bits_COND_SEE_ENEMY` / `InSquad()` / `MySquadLeader()->m_flLastEnemySightTime` before calling `DecideAction()`; capability mask gains `AI_CAP_SQUAD`.

## Bug found and fixed while writing Phase C's tests

`UpdateEnemyMemory()` had a **pre-existing double-decay bug carried over from Phase B**, only exposed now because a new test called it twice in quick succession from different sources. The decay step computed `elapsed = now - memory.lastSeenTime`, but `lastSeenTime` was only ever advanced on a (re)sighting - not on every decay-only call. So a sequence of `AI_MEMORY_SOURCE_NONE` ticks kept measuring elapsed time from the *original* sighting while repeatedly re-decaying an *already-decayed* confidence value, compounding decay incorrectly (e.g. two 0.25s ticks after a sighting decayed confidence as if 0.5s **and then another 0.5s** had passed, not 0.5s total). This was live in Phase B's playtest too, just not obviously wrong at a glance. Fixed by advancing `lastSeenTime` on every call that computes a decay step, not just on (re)sightings — see the comment in `ai_hybrid_core.cpp`. Added a regression test (`TestUpdateEnemyMemory`'s second `UpdateEnemyMemory` call) asserting two sequential decay steps compose correctly instead of compounding.

## Verified

- `scripts/run-ai-hybrid-core-tests.sh` — updated + new cases (direct sight still forces 1.0 regardless of squad state; squad-shared floors at 0.7 without lowering an already-higher confidence; `AI_MEMORY_SOURCE_NONE` decays as before **and composes correctly across repeated calls, per the bug fix above**; a squad-shared-only context never scores `ATTACK` above 0).
- `scripts/build.sh` + `scripts/diff-baseline.sh reference-builds/6a789e7f-release` — `client.so`/`vgui.so` byte-identical; `hl.so` differs, as expected.
- Manual in-game playtest, via the new `ai_hybrid_log` activity log (see `docs/designs/hybrid-ai-core-activity-logger.md`) on `c2a5f` with `spserver.cfg` auto-enabling `ai_hybrid`/`ai_hybrid_debug`/`ai_hybrid_log`: **confirmed working, 2026-09-08.** Analyzed 16,301 log lines across 41 grunts over ~230s: zero `ATTACK` entries without `enemy_visible: true` (the safety invariant holds); 379 entries with confidence ≈ 0.70 while not directly visible (the squad-shared floor firing as designed); score ranges and action dwell times (ATTACK ~2.6s avg, SEARCH ~6.9s avg, COVER ~23.7s avg) look sane, no thrashing.

### Bug found via log analysis and fixed

`ScoreCover()` didn't require `memory.enemyKnown`, unlike `ScoreSearch()`. Since a cautious profile alone (`caution*30 - aggression*15 > 0`) scores above zero, every grunt showed a baseline `COVER` action in the log **before ever encountering an enemy** (9,395 of 10,227 `COVER` log lines were this false-positive baseline, confidence exactly 0). Harmless in-game — `ResolveHybridSchedule()`'s `m_hEnemy == NULL` check already blocked it from ever producing a real schedule — but misleading to read, and a genuine utility-AI modeling gap ("cover from an enemy" with no enemy known makes no sense). Fixed by adding the same `!memory.enemyKnown → return 0` guard `ScoreSearch()` already had; added a regression test and re-verified `client.so`/`vgui.so` stay byte-identical to baseline.

## Open Questions

None outstanding for this phase.
- Squad roles (PRIMARY/FLANK/SUPPRESS/RESERVE, spec §20) remain deferred until FLANK/SUPPRESS actions exist to assign them to.
