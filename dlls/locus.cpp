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

// LRC - Locus system implementation
// Resolves $locus references in entity targeting strings.

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "locus.h"

//=========================================================
// CalcLocus_Position
// Resolves a locus position string to an actual position.
// If szText is empty or null, returns pLocus origin.
//=========================================================
Vector CalcLocus_Position(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText)
{
	if (!szText || !*szText)
	{
		if (pLocus)
			return pLocus->pev->origin;
		return g_vecZero;
	}

	// Try to find an entity with this targetname
	CBaseEntity* pCalc = UTIL_FindEntityByTargetname(nullptr, szText);
	if (pCalc)
	{
		return pCalc->CalcPosition(pLocus);
	}

	// Fallback to locus origin
	if (pLocus)
		return pLocus->pev->origin;

	return g_vecZero;
}

//=========================================================
// CalcLocus_Velocity
// Resolves a locus velocity string to an actual velocity.
// If szText is empty or null, returns pLocus velocity.
//=========================================================
Vector CalcLocus_Velocity(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText)
{
	if (!szText || !*szText)
	{
		if (pLocus)
			return pLocus->pev->velocity;
		return g_vecZero;
	}

	// Try to find an entity with this targetname
	CBaseEntity* pCalc = UTIL_FindEntityByTargetname(nullptr, szText);
	if (pCalc)
	{
		return pCalc->CalcVelocity(pLocus);
	}

	// Fallback to locus velocity
	if (pLocus)
		return pLocus->pev->velocity;

	return g_vecZero;
}

//=========================================================
// CalcLocus_Ratio
// Resolves a locus ratio string to a scalar value.
// If szText is empty or null, returns 0.
//=========================================================
float CalcLocus_Ratio(CBaseEntity* pLocus, const char* szText)
{
	if (!szText || !*szText)
		return 0;

	// Try to parse as a number
	char* pEnd;
	float fResult = strtof(szText, &pEnd);
	if (pEnd != szText && *pEnd == '\0')
		return fResult;

	// Try to find an entity with this targetname
	CBaseEntity* pCalc = UTIL_FindEntityByTargetname(nullptr, szText);
	if (pCalc)
	{
		return pCalc->CalcRatio(pLocus);
	}

	return 0;
}

//=========================================================
// Locus point entities
//=========================================================

//=========================================================
// calc_position - returns its own position
//=========================================================
class CCalcPosition : public CPointEntity
{
public:
	Vector CalcPosition(CBaseEntity* pLocus) override;
};

LINK_ENTITY_TO_CLASS(calc_position, CCalcPosition);

Vector CCalcPosition::CalcPosition(CBaseEntity* pLocus)
{
	return pev->origin;
}

//=========================================================
// calc_velocity - returns its own velocity
//=========================================================
class CCalcVelocity : public CPointEntity
{
public:
	Vector CalcVelocity(CBaseEntity* pLocus) override;
};

LINK_ENTITY_TO_CLASS(calc_velocity, CCalcVelocity);

Vector CCalcVelocity::CalcVelocity(CBaseEntity* pLocus)
{
	return pev->velocity;
}

//=========================================================
// calc_subvelocity - returns velocity with optional axis discard
// SoHL 1.5 - Spawnflags to discard individual velocity axes
//=========================================================
#define SF_CALCVELOCITY_DISCARDX (1 << 5)
#define SF_CALCVELOCITY_DISCARDY (1 << 6)
#define SF_CALCVELOCITY_DISCARDZ (1 << 7)

class CCalcSubVelocity : public CPointEntity
{
public:
	Vector CalcVelocity(CBaseEntity* pLocus) override;
};

LINK_ENTITY_TO_CLASS(calc_subvelocity, CCalcSubVelocity);

Vector CCalcSubVelocity::CalcVelocity(CBaseEntity* pLocus)
{
	Vector vecResult = pev->velocity;

	if (FBitSet(pev->spawnflags, SF_CALCVELOCITY_DISCARDX))
		vecResult.x = 0;
	if (FBitSet(pev->spawnflags, SF_CALCVELOCITY_DISCARDY))
		vecResult.y = 0;
	if (FBitSet(pev->spawnflags, SF_CALCVELOCITY_DISCARDZ))
		vecResult.z = 0;

	return vecResult;
}

//=========================================================
// calc_ratio - returns its stored ratio value
//=========================================================
class CCalcRatio : public CPointEntity
{
public:
	float CalcRatio(CBaseEntity* pLocus) override;
};

LINK_ENTITY_TO_CLASS(calc_ratio, CCalcRatio);

float CCalcRatio::CalcRatio(CBaseEntity* pLocus)
{
	return pev->frags;  // Use frags as ratio storage
}
