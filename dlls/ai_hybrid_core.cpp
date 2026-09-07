#include "ai_hybrid_core.h"

#include <cmath>

const char* ActionName(AITacticalAction action)
{
	switch (action)
	{
	case AI_ACTION_NONE:
		return "NONE";
	case AI_ACTION_IDLE:
		return "IDLE";
	case AI_ACTION_ALERT:
		return "ALERT";
	case AI_ACTION_ATTACK:
		return "ATTACK";
	case AI_ACTION_ADVANCE:
		return "ADVANCE";
	case AI_ACTION_COVER:
		return "COVER";
	case AI_ACTION_SEARCH:
		return "SEARCH";
	case AI_ACTION_INVESTIGATE:
		return "INVESTIGATE";
	case AI_ACTION_SUPPRESS:
		return "SUPPRESS";
	case AI_ACTION_FLANK_LEFT:
		return "FLANK_LEFT";
	case AI_ACTION_FLANK_RIGHT:
		return "FLANK_RIGHT";
	case AI_ACTION_RETREAT:
		return "RETREAT";
	case AI_ACTION_REPOSITION:
		return "REPOSITION";
	case AI_ACTION_FOLLOW:
		return "FOLLOW";
	case AI_ACTION_MOVE_ASIDE:
		return "MOVE_ASIDE";
	case AI_ACTION_COUNT:
		break;
	}
	return "UNKNOWN";
}

bool ActionSupported(AITacticalAction action, uint32_t capabilityMask)
{
	switch (action)
	{
	case AI_ACTION_NONE:
	case AI_ACTION_IDLE:
	case AI_ACTION_ALERT:
	case AI_ACTION_ATTACK:
	case AI_ACTION_ADVANCE:
	case AI_ACTION_REPOSITION:
	case AI_ACTION_FOLLOW:
		return true; // baseline actions, no capability flag gates them
	case AI_ACTION_SEARCH:
	case AI_ACTION_INVESTIGATE:
		return (capabilityMask & AI_CAP_SEARCH) != 0;
	case AI_ACTION_COVER:
		return (capabilityMask & AI_CAP_COVER) != 0;
	case AI_ACTION_FLANK_LEFT:
	case AI_ACTION_FLANK_RIGHT:
		return (capabilityMask & AI_CAP_FLANK) != 0;
	case AI_ACTION_SUPPRESS:
		return (capabilityMask & AI_CAP_SUPPRESSION) != 0;
	case AI_ACTION_RETREAT:
		return (capabilityMask & AI_CAP_RETREAT) != 0;
	case AI_ACTION_MOVE_ASIDE:
		return (capabilityMask & AI_CAP_MOVE_ASIDE) != 0;
	case AI_ACTION_COUNT:
		break;
	}
	return false;
}

float ClampScore(float value)
{
	if (!(value == value)) // NaN: the only float that isn't equal to itself
		return 0.0f;
	if (value < 0.0f)
		return 0.0f;
	if (value > 100.0f)
		return 100.0f;
	return value;
}

float Clamp01(float value)
{
	if (!(value == value)) // NaN
		return 0.0f;
	if (value < 0.0f)
		return 0.0f;
	if (value > 1.0f)
		return 1.0f;
	return value;
}

float DecayConfidence(float confidence, float elapsedSeconds, float halfLifeSeconds)
{
	if (elapsedSeconds <= 0.0f)
		return Clamp01(confidence);

	if (halfLifeSeconds <= 0.0f)
		return 0.0f;

	const float decayed = confidence * std::pow(0.5f, elapsedSeconds / halfLifeSeconds);
	return Clamp01(decayed);
}

void UpdateSnapshot(AIHybridState& state)
{
	// Phase A never decides anything; the dormant state always reports NONE.
	state.previousAction = state.currentAction;
	state.currentAction = AI_ACTION_NONE;
}
