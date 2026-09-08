# Design: Hybrid AI Core — Phase B (CHGrunt Attack/Cover/Search)

Generated on 2026-09-08 (direct implementation, no /office-hours session — user said "weiter mit Phase B" after Phase A was verified in-game)
Branch: ext-ki
Repo: hammermaps/halflife-op4-updated-sohl
Status: APPROVED
Supersedes: docs/designs/hybrid-ai-core-phase-a.md (extends it, doesn't replace it)

## Scope

Per `AGENTS_HYBRID_AI.md` §48 (Phase B acceptance) and §32 (CHGrunt reference implementation): direct sight updates memory; losing sight doesn't mean instant memory loss; search occurs and expires; attack/cover chosen through utility; original logic remains as fallback. Explicitly **not** in this phase: squad-shared memory (§C), flanking/suppression (§D), morale/personality tuning (§F) — those need capabilities CHGrunt isn't granted yet.

## Key decisions

1. **No `Vector`/position storage in the engine-free core.** The engine already tracks last-known-position via `CBaseMonster::m_vecEnemyLKP` (found by reading `dlls/monsters.cpp`). The core only needs to know *how sure* the NPC still is (a confidence float), not *where* — so `AIEnemyMemory` stays a `bool` + two `float`s, no engine math types, keeping the whole decision function (`DecideAction`) unit-testable exactly like Phase A's functions.
2. **`GetSchedule()` integration is deliberately narrow, not a top-level wrapper.** The spec's example (§3) checks `ShouldUseEnhancedAI()` before *everything*. `CHGrunt::GetSchedule()` is a ~250-line hand-tuned decision tree (repel-down-rope movement, grenade-danger cover-seeking, first-sight squad speech, no-ammo reload, light-damage flinch odds). Wrapping the whole thing would let the 3-action Phase B utility model (attack/cover/search) silently replace all of that variety and safety behavior. Instead, `ShouldUseHybridDecision()` explicitly excludes every one of those states (flying/repel, dangerous sound, non-combat state, dead/new enemy, no ammo, light damage) — the hybrid layer only ever competes for the "plain ongoing combat, nothing special just happened" case, which is exactly the tactical layer §3/§4 describe replacing. This duplicates a handful of condition checks already present at the top of the original function, but is explicit, auditable, and safe by construction rather than requiring a risky refactor of a long, delicate function.
3. **`SCHED_CHASE_ENEMY` reused for SEARCH**, not a new `SCHED_ENHANCED_SEARCH` (spec §16: "new schedules only where truly needed"). Its task list (`TASK_GET_PATH_TO_ENEMY` → `TASK_RUN_PATH`) already paths toward the enemy or its LKP via the engine's own `BuildRoute`, and its break conditions (`bits_COND_NEW_ENEMY`, both attack-possible conditions, `TASK_FAILED`, danger sound) naturally hand control back to `GetSchedule()` — and therefore back to the utility layer — the moment something changes.
4. **`ATTACK`/`COVER`/`SEARCH` resolution double-checks engine conditions before committing** (`bits_COND_CAN_RANGE_ATTACK1/2` for attack, `m_hEnemy != NULL` for cover/search) and returns `nullptr` (→ original fallback) otherwise — "never force movement toward an invalid state" (§59).
5. **Confidence/decision-timer state is still not saved** (per Phase A's same call, restated in `ai_hybrid_core.h`): losing it on load means one tick of "forgot the enemy," which is safe (falls back to original behavior, never crashes, never forces movement) and is re-established on the next sighting or decision tick.
6. **Two new tunables as CVars**, not hardcoded: `ai_hybrid_decision_interval` (default 0.25s, within spec §14's 0.15–0.35s combat range) and `ai_hybrid_confidence_halflife` (default 8s, within spec §43's "usable for search ~8–15s" guidance). The hysteresis switch threshold stays a compiled constant (`AIHybrid_SwitchThreshold()`, 5.0) — not enough evidence yet that it needs runtime tuning.

## What's implemented

- `dlls/ai_hybrid_core.h/.cpp`: `AIEnemyMemory` + `UpdateEnemyMemory()` (direct-sight-only confidence model), `AIConfidenceLevel`/`ClassifyConfidence()` with the named thresholds from spec §9, `AIUtilityContext` + `ScoreAttack`/`ScoreCover`/`ScoreSearch`, `AIDecision` + `DecideAction()` (scores all three, applies hysteresis, updates `AIHybridState`). All pure, all covered by `tests/ai_hybrid_core_tests.cpp` (replaces Phase A's now-superseded `UpdateSnapshot`/`TestUpdateSnapshot`).
- `dlls/ai_hybrid.h/.cpp`: `AIHybrid_DecisionInterval()`, `AIHybrid_ConfidenceHalfLife()`, `AIHybrid_SwitchThreshold()`; debug log now also prints `confidence`.
- `dlls/game.h/.cpp`: two new CVars registered alongside the existing block.
- `dlls/hgrunt.cpp`: `m_flNextAIDecision` throttles `DecideAction()` calls from `PrescheduleThink()` (runs regardless of `ai_hybrid`, cheaply, so debug output works without flipping the master switch — no behavior impact since `GetSchedule()` only *consumes* the result when enabled); `ShouldUseHybridDecision()` and `ResolveHybridSchedule()` implement the narrow `GetSchedule()` integration described above.

## Verified

- `scripts/run-ai-hybrid-core-tests.sh` — all checks pass (enemy memory, confidence classification, all three score functions, `DecideAction` basic selection + hysteresis).
- `scripts/build.sh` — builds clean.
- `scripts/diff-baseline.sh reference-builds/6a789e7f-release` — `client.so` and `vgui.so` byte-identical to the pre-Phase-A baseline; only `hl.so` differs, as expected (no client-side changes in either phase).
- Manual in-game (`ai_hybrid_debug 1`, `map ai_test_grunt`): confirmed working by the user for Phase A's dormant output; Phase B's live attack/cover/search behavior needs a manual playtest against a grunt with `ai_hybrid 1` — **not yet done**, see Open Questions.

## Open Questions

- Manual playtest of actual behavior (`ai_hybrid 1`) hasn't happened yet — `ai_test_grunt.map`'s single stationary grunt has no real combat scenario (no cover geometry, no way to lose line of sight) to meaningfully exercise COVER/SEARCH. **Resolved which map to use for this:** searched every `.bsp` already present in the test mod's `maps/` folder for the exact classname `monster_human_grunt` (the hostile HECU grunt `CHGrunt` handles — distinct from `monster_human_grunt_ally`, which barely appears with that exact name in Opposing Force's own `of*.bsp` maps since OpFor's grunts are mostly allies). The retail Half-Life 1 campaign maps bundled alongside OpFor's own maps have plenty: `c2a5f` ("Surface Tension") has 19, `c2a2b1` has 14, `c2a4e`/`c2a2e`/`c1a3a` have 10 each. Recommended: `c2a5f` — large-scale multi-grunt firefight with real cover geometry and sightline breaks, ideal for exercising ATTACK/COVER/SEARCH transitions. Test with:
  ```
  ai_hybrid 1
  ai_hybrid_debug 1
  map c2a5f
  ```
  Still not actually run as of this writing — the playtest itself remains open.
- `ScoreAttack`/`ScoreCover`/`ScoreSearch` weights are first-pass estimates (not derived from the spec's illustrative formulas 1:1, since those reference fields — danger, weaponSuitability, morale — that don't exist yet). Expect to retune after the playtest above.
- No squad-role awareness yet: a squad of hybrid-enabled grunts will each decide independently (no coordination), which is correct for Phase B but means "one grunt attacks while its squadmate also tries to attack the same angle" is possible — Phase C's squad blackboard addresses this.

## Next Steps (Phase C, not started)

Squad-shared enemy knowledge (lower confidence than direct sight, per spec §8) and squad roles/blackboard, per `AGENTS_HYBRID_AI.md` §19-20.
