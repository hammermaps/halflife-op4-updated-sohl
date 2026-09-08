#include "extdll.h"
#include "util.h"

#include "ai_hybrid.h"

namespace
{
// Not a CVar in Phase B - see AIHybrid_SwitchThreshold()'s header comment.
constexpr float kSwitchThreshold = 5.0f;
} // namespace

bool AIHybrid_Enabled()
{
	return CVAR_GET_FLOAT("ai_hybrid") != 0.0f;
}

float AIHybrid_DecisionInterval()
{
	const float value = CVAR_GET_FLOAT("ai_hybrid_decision_interval");
	return (value > 0.0f) ? value : 0.25f;
}

float AIHybrid_ConfidenceHalfLife()
{
	const float value = CVAR_GET_FLOAT("ai_hybrid_confidence_halflife");
	return (value > 0.0f) ? value : 8.0f;
}

float AIHybrid_SwitchThreshold()
{
	return kSwitchThreshold;
}

void AIHybrid_MaybeLogDebug(const AIHybridState& state, const char* monsterLabel)
{
	if (CVAR_GET_FLOAT("ai_hybrid_debug") < 1.0f)
		return;

	ALERT(at_console, "[AI] %s current=%s previous=%s confidence=%.2f enabled=%d\n",
		monsterLabel,
		ActionName(state.currentAction),
		ActionName(state.previousAction),
		state.enemyMemory.confidence,
		AIHybrid_Enabled() ? 1 : 0);
}
