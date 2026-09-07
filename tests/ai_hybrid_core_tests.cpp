// Standalone host tests for dlls/ai_hybrid_core.{h,cpp} - the engine-free
// core of the Hybrid AI Core (see AGENTS_HYBRID_AI.md,
// docs/designs/hybrid-ai-core-phase-a.md).
//
// Deliberately minimal: no test framework, no engine headers, no DLL link.
// Compiled and run via scripts/run-ai-hybrid-core-tests.sh (also wired into
// `make check` in linux/Makefile). Covers only the pure functions that exist
// in Phase A; ScoreAttack/ChooseBestAction/hysteresis etc. get their own
// cases once Phase B adds them to the same core.

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

static void TestUpdateSnapshot()
{
	AIHybridState state;
	CHECK(state.currentAction == AI_ACTION_NONE);
	CHECK(state.previousAction == AI_ACTION_NONE);

	// Phase A: every tick resolves to NONE and rotates current into previous.
	state.currentAction = AI_ACTION_NONE;
	UpdateSnapshot(state);
	CHECK(state.currentAction == AI_ACTION_NONE);
	CHECK(state.previousAction == AI_ACTION_NONE);
}

int main()
{
	TestActionName();
	TestActionSupported();
	TestClamping();
	TestDecayConfidence();
	TestUpdateSnapshot();

	if (g_failures == 0)
	{
		std::printf("ai_hybrid_core_tests: all checks passed\n");
		return 0;
	}

	std::fprintf(stderr, "ai_hybrid_core_tests: %d check(s) failed\n", g_failures);
	return 1;
}
