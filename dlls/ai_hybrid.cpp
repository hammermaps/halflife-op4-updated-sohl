#include "extdll.h"
#include "util.h"

#include "ai_hybrid.h"

#include <cstdio>
#include <cstring>

namespace
{
// Not a CVar in Phase B - see AIHybrid_SwitchThreshold()'s header comment.
constexpr float kSwitchThreshold = 5.0f;

// Lazily-opened, kept alive for the process lifetime (unlike dlls/stats.cpp's
// open/write/close-per-call pattern - Hybrid AI decisions happen far more
// often than stat snapshots, so reopening every call would be wasteful).
// Tracks the path it was opened with so a mid-session ai_hybrid_log_file
// change reopens the right file instead of silently keeping the old one.
FILE* g_activityLogFile = nullptr;
char g_activityLogPath[260] = "";

FILE* GetActivityLogFile()
{
	const char* path = CVAR_GET_STRING("ai_hybrid_log_file");
	if (!path || !path[0])
		path = "ai_hybrid_log.jsonl";

	if (g_activityLogFile && std::strncmp(path, g_activityLogPath, sizeof(g_activityLogPath)) != 0)
	{
		std::fclose(g_activityLogFile);
		g_activityLogFile = nullptr;
	}

	if (!g_activityLogFile)
	{
		g_activityLogFile = std::fopen(path, "a");
		if (g_activityLogFile)
		{
			std::strncpy(g_activityLogPath, path, sizeof(g_activityLogPath) - 1);
			g_activityLogPath[sizeof(g_activityLogPath) - 1] = '\0';
		}
	}

	return g_activityLogFile;
}
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

float AIHybrid_SquadReportRecency()
{
	const float value = CVAR_GET_FLOAT("ai_hybrid_squad_report_recency");
	return (value > 0.0f) ? value : 5.0f;
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

void AIHybrid_MaybeLogActivity(int entityIndex, const char* monsterLabel,
	const AIHybridState& state, const AIDecision& decision, const AIUtilityContext& context)
{
	if (CVAR_GET_FLOAT("ai_hybrid_log") < 1.0f)
		return;

	FILE* file = GetActivityLogFile();
	if (!file)
		return;

	// Hand-rolled JSON: every field is a number, bool, or a controlled
	// enum-name/classname string (never arbitrary/user-supplied text), so
	// there is nothing that needs escaping.
	std::fprintf(file,
		"{\"time\":%.3f,\"entity\":\"%s#%d\",\"action\":\"%s\",\"previous_action\":\"%s\","
		"\"score\":%.2f,\"confidence\":%.3f,\"enemy_visible\":%s,\"can_range_attack\":%s,"
		"\"health_ratio\":%.3f,\"ai_hybrid_enabled\":%s}\n",
		gpGlobals->time,
		monsterLabel, entityIndex,
		ActionName(decision.action),
		ActionName(state.previousAction),
		decision.score,
		state.enemyMemory.confidence,
		context.enemyVisible ? "true" : "false",
		context.canRangeAttack ? "true" : "false",
		context.healthRatio,
		AIHybrid_Enabled() ? "true" : "false");

	std::fflush(file); // survive a mid-session crash with the tail intact
}
