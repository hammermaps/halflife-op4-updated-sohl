#include "extdll.h"
#include "util.h"

#include "ai_hybrid.h"

bool AIHybrid_Enabled()
{
	return CVAR_GET_FLOAT("ai_hybrid") != 0.0f;
}

void AIHybrid_MaybeLogDebug(const AIHybridState& state, const char* monsterLabel)
{
	if (CVAR_GET_FLOAT("ai_hybrid_debug") < 1.0f)
		return;

	ALERT(at_console, "[AI] %s current=%s previous=%s enabled=%d\n",
		monsterLabel,
		ActionName(state.currentAction),
		ActionName(state.previousAction),
		AIHybrid_Enabled() ? 1 : 0);
}
