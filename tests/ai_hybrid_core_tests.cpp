// Standalone host tests for dlls/ai_hybrid_core.{h,cpp} - the engine-free
// core of the Hybrid AI Core (see AGENTS_HYBRID_AI.md,
// docs/designs/hybrid-ai-core-phase-a.md and -phase-b.md).
//
// Deliberately minimal: no test framework, no engine headers, no DLL link.
// Compiled and run via scripts/run-ai-hybrid-core-tests.sh (also wired into
// `make check` in linux/Makefile).

#include "../dlls/ai_hybrid_core.h"

#include <cmath>
#include <cstdio>
#include <string_view>

static int g_failures = 0;

#define CHECK(expr)                                                     \
	do                                                                    \
	{                                                                     \
		if (!(expr))                                                       \
		{                                                                   \
			std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
			++g_failures;                                                    \
		}                                                                   \
	} while (0)

static bool NearlyEqual(float a, float b, float epsilon = 0.0001f)
{
	return std::fabs(a - b) <= epsilon;
}

static void TestActionName()
{
	CHECK(std::string_view(ActionName(AI_ACTION_NONE)) == "NONE");
	CHECK(std::string_view(ActionName(AI_ACTION_FLANK_LEFT)) == "FLANK_LEFT");
	CHECK(std::string_view(ActionName(AI_ACTION_MOVE_ASIDE)) == "MOVE_ASIDE");
	// Out-of-range value must not crash and must not return nullptr.
	CHECK(ActionName(static_cast<AITacticalAction>(9999)) != nullptr);
}

static void TestActionSupported()
{
	// Baseline actions: supported regardless of capability mask, including 0.
	CHECK(ActionSupported(AI_ACTION_NONE, 0));
	CHECK(ActionSupported(AI_ACTION_ATTACK, 0));
	CHECK(ActionSupported(AI_ACTION_FOLLOW, 0));

	// Capability-gated actions: false without the flag, true with it.
	CHECK(!ActionSupported(AI_ACTION_COVER, 0));
	CHECK(ActionSupported(AI_ACTION_COVER, AI_CAP_COVER));
	CHECK(!ActionSupported(AI_ACTION_FLANK_LEFT, AI_CAP_COVER));
	CHECK(ActionSupported(AI_ACTION_FLANK_RIGHT, AI_CAP_FLANK));
	CHECK(ActionSupported(AI_ACTION_SUPPRESS, AI_CAP_SUPPRESSION));
	CHECK(ActionSupported(AI_ACTION_RETREAT, AI_CAP_RETREAT));
	CHECK(ActionSupported(AI_ACTION_MOVE_ASIDE, AI_CAP_MOVE_ASIDE));
	CHECK(ActionSupported(AI_ACTION_SEARCH, AI_CAP_SEARCH));
	CHECK(ActionSupported(AI_ACTION_INVESTIGATE, AI_CAP_SEARCH));

	// Unrelated flags don't leak into unrelated actions.
	CHECK(!ActionSupported(AI_ACTION_RETREAT, AI_CAP_FLANK | AI_CAP_COVER));
}

static void TestClamping()
{
	CHECK(NearlyEqual(ClampScore(-5.0f), 0.0f));
	CHECK(NearlyEqual(ClampScore(150.0f), 100.0f));
	CHECK(NearlyEqual(ClampScore(42.0f), 42.0f));
	CHECK(NearlyEqual(ClampScore(std::nanf("")), 0.0f));

	CHECK(NearlyEqual(Clamp01(-0.5f), 0.0f));
	CHECK(NearlyEqual(Clamp01(1.5f), 1.0f));
	CHECK(NearlyEqual(Clamp01(0.3f), 0.3f));
	CHECK(NearlyEqual(Clamp01(std::nanf("")), 0.0f));
}

static void TestDecayConfidence()
{
	// No time passed: confidence unchanged (never increases via this path).
	CHECK(NearlyEqual(DecayConfidence(0.8f, 0.0f, 10.0f), 0.8f));
	CHECK(NearlyEqual(DecayConfidence(0.8f, -5.0f, 10.0f), 0.8f));

	// Exactly one half-life: confidence halves.
	CHECK(NearlyEqual(DecayConfidence(1.0f, 10.0f, 10.0f), 0.5f, 0.001f));

	// Two half-lives: quarters.
	CHECK(NearlyEqual(DecayConfidence(1.0f, 20.0f, 10.0f), 0.25f, 0.001f));

	// Invalid half-life with elapsed time: instant full decay.
	CHECK(NearlyEqual(DecayConfidence(1.0f, 5.0f, 0.0f), 0.0f));
	CHECK(NearlyEqual(DecayConfidence(1.0f, 5.0f, -1.0f), 0.0f));

	// Result always stays clamped to [0, 1] even for out-of-range input.
	CHECK(NearlyEqual(DecayConfidence(2.0f, 0.0f, 10.0f), 1.0f));
}

static void TestClassifyConfidence()
{
	CHECK(ClassifyConfidence(0.0f) == AI_CONFIDENCE_LOST);
	CHECK(ClassifyConfidence(-1.0f) == AI_CONFIDENCE_LOST);
	CHECK(ClassifyConfidence(0.1f) == AI_CONFIDENCE_WEAK);
	CHECK(ClassifyConfidence(0.2f) == AI_CONFIDENCE_UNCERTAIN);
	CHECK(ClassifyConfidence(0.44f) == AI_CONFIDENCE_UNCERTAIN);
	CHECK(ClassifyConfidence(0.45f) == AI_CONFIDENCE_PROBABLE);
	CHECK(ClassifyConfidence(0.74f) == AI_CONFIDENCE_PROBABLE);
	CHECK(ClassifyConfidence(0.75f) == AI_CONFIDENCE_CONFIRMED);
	CHECK(ClassifyConfidence(1.0f) == AI_CONFIDENCE_CONFIRMED);
}

static void TestUpdateEnemyMemory()
{
	AIEnemyMemory memory;
	CHECK(!memory.enemyKnown);

	// Direct sight: confidence jumps to 1.0 immediately, regardless of prior state.
	UpdateEnemyMemory(memory, /*enemyVisible=*/true, /*now=*/10.0f, /*halfLife=*/5.0f);
	CHECK(memory.enemyKnown);
	CHECK(NearlyEqual(memory.confidence, 1.0f));
	CHECK(NearlyEqual(memory.lastSeenTime, 10.0f));

	// Losing sight does not instantly zero confidence - it decays over time.
	UpdateEnemyMemory(memory, /*enemyVisible=*/false, /*now=*/15.0f, /*halfLife=*/5.0f);
	CHECK(NearlyEqual(memory.confidence, 0.5f, 0.001f)); // one half-life elapsed
	CHECK(NearlyEqual(memory.lastSeenTime, 10.0f)); // unchanged while not visible

	// Never having seen an enemy: nothing to decay, stays at defaults.
	AIEnemyMemory neverSeen;
	UpdateEnemyMemory(neverSeen, /*enemyVisible=*/false, /*now=*/100.0f, /*halfLife=*/5.0f);
	CHECK(!neverSeen.enemyKnown);
	CHECK(NearlyEqual(neverSeen.confidence, 0.0f));
}

static void TestScoreFunctions()
{
	AIProfile profile;
	AIEnemyMemory noMemory;
	AIEnemyMemory knownMemory;
	knownMemory.enemyKnown = true;
	knownMemory.confidence = 0.8f;

	// ScoreAttack requires both visibility and a valid attack condition.
	AIUtilityContext ctx;
	CHECK(NearlyEqual(ScoreAttack(ctx, noMemory, profile), 0.0f)); // neither set
	ctx.enemyVisible = true;
	CHECK(NearlyEqual(ScoreAttack(ctx, noMemory, profile), 0.0f)); // can't range attack
	ctx.canRangeAttack = true;
	CHECK(ScoreAttack(ctx, noMemory, profile) > 0.0f);
	// Low health should pull the attack score down relative to full health.
	AIUtilityContext hurtCtx = ctx;
	hurtCtx.healthRatio = 0.1f;
	CHECK(ScoreAttack(hurtCtx, noMemory, profile) < ScoreAttack(ctx, noMemory, profile));

	// ScoreCover rises with damage taken (lower healthRatio).
	AIUtilityContext coverCtx;
	coverCtx.healthRatio = 1.0f;
	AIUtilityContext hurtCoverCtx;
	hurtCoverCtx.healthRatio = 0.2f;
	CHECK(ScoreCover(hurtCoverCtx, noMemory, profile) > ScoreCover(coverCtx, noMemory, profile));

	// ScoreSearch: 0 while visible or never seen; positive once known and hidden.
	AIUtilityContext visibleCtx;
	visibleCtx.enemyVisible = true;
	CHECK(NearlyEqual(ScoreSearch(visibleCtx, knownMemory, profile), 0.0f));
	AIUtilityContext hiddenCtx;
	hiddenCtx.enemyVisible = false;
	CHECK(NearlyEqual(ScoreSearch(hiddenCtx, noMemory, profile), 0.0f)); // never seen anything
	CHECK(ScoreSearch(hiddenCtx, knownMemory, profile) > 0.0f); // known + hidden
}

static void TestDecideActionBasicSelection()
{
	AIProfile profile;
	const uint32_t caps = AI_CAP_MEMORY | AI_CAP_COVER | AI_CAP_SEARCH;

	// Visible enemy + can attack -> ATTACK should win over COVER/SEARCH at full health.
	AIHybridState state;
	AIUtilityContext ctx;
	ctx.enemyVisible = true;
	ctx.canRangeAttack = true;
	ctx.healthRatio = 1.0f;
	AIDecision decision = DecideAction(state, ctx, profile, caps, /*now=*/0.0f, /*halfLife=*/5.0f, /*switchThreshold=*/5.0f);
	CHECK(decision.action == AI_ACTION_ATTACK);
	CHECK(state.currentAction == AI_ACTION_ATTACK);
	CHECK(state.previousAction == AI_ACTION_NONE);

	// Enemy no longer visible, but recently seen -> SEARCH should win (confidence still high).
	AIUtilityContext lostCtx;
	lostCtx.enemyVisible = false;
	decision = DecideAction(state, lostCtx, profile, caps, /*now=*/0.5f, /*halfLife=*/5.0f, /*switchThreshold=*/5.0f);
	CHECK(decision.action == AI_ACTION_SEARCH);

	// Capability gate: without AI_CAP_SEARCH/AI_CAP_COVER, losing sight can only
	// resolve to NONE (ATTACK requires visibility, which is false here).
	AIHybridState gatedState;
	AIUtilityContext gatedCtx;
	gatedCtx.enemyVisible = true;
	gatedCtx.canRangeAttack = true;
	DecideAction(gatedState, gatedCtx, profile, /*caps=*/0, 0.0f, 5.0f, 5.0f);
	gatedCtx.enemyVisible = false;
	decision = DecideAction(gatedState, gatedCtx, profile, /*caps=*/0, 1.0f, 5.0f, 5.0f);
	CHECK(decision.action == AI_ACTION_NONE);
}

static void TestDecideActionHysteresis()
{
	AIProfile profile;
	const uint32_t caps = AI_CAP_MEMORY | AI_CAP_COVER | AI_CAP_SEARCH;

	AIHybridState state;
	state.currentAction = AI_ACTION_COVER;
	state.enemyMemory.enemyKnown = true;
	state.enemyMemory.confidence = 0.5f;

	// A marginally higher-scoring action (within switchThreshold) should NOT
	// displace the current action - this is what keeps behavior from
	// flapping every tick between near-tied scores.
	AIUtilityContext ctx;
	ctx.enemyVisible = true;
	ctx.canRangeAttack = true;
	ctx.healthRatio = 1.0f;
	AIDecision decision = DecideAction(state, ctx, profile, caps, 0.0f, 5.0f, /*switchThreshold=*/1000.0f);
	CHECK(decision.action == AI_ACTION_COVER); // huge threshold: nothing can dislodge it
}

int main()
{
	TestActionName();
	TestActionSupported();
	TestClamping();
	TestDecayConfidence();
	TestClassifyConfidence();
	TestUpdateEnemyMemory();
	TestScoreFunctions();
	TestDecideActionBasicSelection();
	TestDecideActionHysteresis();

	if (g_failures == 0)
	{
		std::printf("ai_hybrid_core_tests: all checks passed\n");
		return 0;
	}

	std::fprintf(stderr, "ai_hybrid_core_tests: %d check(s) failed\n", g_failures);
	return 1;
}
