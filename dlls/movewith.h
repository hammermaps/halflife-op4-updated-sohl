/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
// LRC - MoveWith system header

#pragma once

// LRC flags stored in m_iLFlags
#define LF_NOTEASY (1 << 0)
#define LF_NOTMEDIUM (1 << 1)
#define LF_NOTHARD (1 << 2)
#define LF_POSTASSISTVEL (1 << 3)
#define LF_POSTASSISTAVEL (1 << 4)
#define LF_DOASSIST (1 << 5)
#define LF_CORRECTSPEED (1 << 6)
#define LF_DODESIRED (1 << 7)
#define LF_DESIRED_THINK (1 << 8)
#define LF_DESIRED_POSTASSIST (1 << 9)
#define LF_DESIRED_INFO (1 << 10)
#define LF_DESIRED_ACTION (1 << 11)
#define LF_ALIASLIST (1 << 12)
// bits 13-14 reserved (were LF_IN_POSTASSIST_QUEUE / LF_IN_DESIRED_QUEUE — removed)

// Arbitrary limit to detect infinite loops in MoveWith chains
#define MAX_MOVEWITH_DEPTH 100

// Budget limits per frame for queue processing
#define MAX_POSTASSIST_PER_FRAME 512
#define MAX_DESIRED_PER_FRAME 512

class CBaseEntity;

// MoveWith FIFO queue processing (replaces old CheckAssistList/CheckDesiredList)
void MoveWith_ProcessFrameQueues();

// Reset all queue state (call on level start)
void MoveWith_ResetQueues();

// Remove an entity from all queues (call from SUB_Remove / UpdateOnRemove)
void MoveWith_RemoveEntityFromQueues(CBaseEntity* pEnt);

// Schedule deferred actions/thinks for an entity
void UTIL_DesiredAction(CBaseEntity* pEnt);
void UTIL_DesiredThink(CBaseEntity* pEnt);

// Schedule post-assist velocity/angular velocity for an entity
void UTIL_PostAssistVelocity(CBaseEntity* pEnt, const Vector& vel);
void UTIL_PostAssistAVelocity(CBaseEntity* pEnt, const Vector& avel);

// MoveWith utility functions
void UTIL_AssignOrigin(CBaseEntity* pEnt, const Vector& vecOrigin, bool bInitiator = true);
void UTIL_SetVelocity(CBaseEntity* pEnt, const Vector& vecSet);
void UTIL_SetAvelocity(CBaseEntity* pEnt, const Vector& vecSet);
void UTIL_SetAngles(CBaseEntity* pEnt, const Vector& vecSet);

// MoveWith in-game debugger
// Dumps full MoveWith hierarchy to client console; draws debug beams when sohl_mw_debug >= 2
void MoveWith_DebugDump(CBaseEntity* pPlayer);
void MoveWith_DrawDebugBeams();
