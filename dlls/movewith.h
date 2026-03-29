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

// Arbitrary limit to detect infinite loops in MoveWith chains
#define MAX_MOVEWITH_DEPTH 100

class CBaseEntity;

// MoveWith assist/desired list processing
void CheckDesiredList();
void CheckAssistList();

// Schedule deferred actions/thinks for an entity
void UTIL_DesiredAction(CBaseEntity* pEnt);
void UTIL_DesiredThink(CBaseEntity* pEnt);

// MoveWith utility functions
void UTIL_AssignOrigin(CBaseEntity* pEnt, const Vector& vecOrigin, bool bInitiator = true);
void UTIL_SetVelocity(CBaseEntity* pEnt, const Vector& vecSet);
void UTIL_SetAvelocity(CBaseEntity* pEnt, const Vector& vecSet);
void UTIL_SetAngles(CBaseEntity* pEnt, const Vector& vecSet);
