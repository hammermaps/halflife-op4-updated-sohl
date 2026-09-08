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

AIConfidenceLevel ClassifyConfidence(float confidence)
{
	if (confidence >= AI_CONFIDENCE_CONFIRMED_THRESHOLD)
		return AI_CONFIDENCE_CONFIRMED;
	if (confidence >= AI_CONFIDENCE_PROBABLE_THRESHOLD)
		return AI_CONFIDENCE_PROBABLE;
	if (confidence >= AI_CONFIDENCE_UNCERTAIN_THRESHOLD)
		return AI_CONFIDENCE_UNCERTAIN;
	if (confidence > 0.0f)
		return AI_CONFIDENCE_WEAK;
	return AI_CONFIDENCE_LOST;
}

void UpdateEnemyMemory(AIEnemyMemory& memory, bool enemyVisible, float now, float confidenceHalfLifeSeconds)
{
	if (enemyVisible)
	{
		memory.enemyKnown = true;
		memory.confidence = 1.0f;
		memory.lastSeenTime = now;
		return;
	}

	if (!memory.enemyKnown)
		return; // never seen anything yet - nothing to decay

	const float elapsed = (memory.lastSeenTime < 0.0f) ? 0.0f : (now - memory.lastSeenTime);
	memory.confidence = DecayConfidence(memory.confidence, elapsed, confidenceHalfLifeSeconds);
}

float ScoreAttack(const AIUtilityContext& context, const AIEnemyMemory& memory, const AIProfile& profile)
{
	if (!context.enemyVisible || !context.canRangeAttack)
		return 0.0f;

	float score = 50.0f;
	score += profile.aggression * 30.0f;
	score += memory.confidence * 20.0f;
	score -= (1.0f - context.healthRatio) * 40.0f;
	return ClampScore(score);
}

float ScoreCover(const AIUtilityContext& context, const AIEnemyMemory& memory, const AIProfile& profile)
{
	const float danger = 1.0f - context.healthRatio;
	float score = danger * 50.0f;
	score += profile.caution * 30.0f;
	if (context.enemyVisible)
		score += 10.0f;
	score += memory.confidence * 10.0f;
	score -= profile.aggression * 15.0f;
	return ClampScore(score);
}

float ScoreSearch(const AIUtilityContext& context, const AIEnemyMemory& memory, const AIProfile& profile)
{
	if (context.enemyVisible || !memory.enemyKnown)
		return 0.0f;

	float score = memory.confidence * 60.0f;
	score += profile.aggression * 10.0f;
	return ClampScore(score);
}

AIDecision DecideAction(AIHybridState& state, const AIUtilityContext& context, const AIProfile& profile,
	uint32_t capabilityMask, float now, float confidenceHalfLifeSeconds, float switchThreshold)
{
	UpdateEnemyMemory(state.enemyMemory, context.enemyVisible, now, confidenceHalfLifeSeconds);

	struct Candidate
	{
		AITacticalAction action;
		float score;
	};

	const Candidate candidates[] = {
		{AI_ACTION_ATTACK, ActionSupported(AI_ACTION_ATTACK, capabilityMask)
							   ? ScoreAttack(context, state.enemyMemory, profile)
							   : 0.0f},
		{AI_ACTION_COVER, ActionSupported(AI_ACTION_COVER, capabilityMask)
							  ? ScoreCover(context, state.enemyMemory, profile)
							  : 0.0f},
		{AI_ACTION_SEARCH, ActionSupported(AI_ACTION_SEARCH, capabilityMask)
							   ? ScoreSearch(context, state.enemyMemory, profile)
							   : 0.0f},
	};

	AITacticalAction bestAction = AI_ACTION_NONE;
	float bestScore = 0.0f;
	for (const Candidate& candidate : candidates)
	{
		if (candidate.score > bestScore)
		{
			bestScore = candidate.score;
			bestAction = candidate.action;
		}
	}

	// Hysteresis: don't abandon the previous action for a new one that only
	// marginally beats it - avoids flapping between near-tied scores.
	const AITacticalAction previous = state.currentAction;
	if (previous != AI_ACTION_NONE && previous != bestAction)
	{
		float previousScore = 0.0f;
		for (const Candidate& candidate : candidates)
		{
			if (candidate.action == previous)
			{
				previousScore = candidate.score;
				break;
			}
		}
		if (previousScore > 0.0f && bestScore <= previousScore + switchThreshold)
		{
			bestAction = previous;
			bestScore = previousScore;
		}
	}

	state.previousAction = state.currentAction;
	state.currentAction = bestAction;
	return {bestAction, bestScore};
}
