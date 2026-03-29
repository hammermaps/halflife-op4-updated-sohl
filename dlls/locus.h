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

// LRC - Locus system
// Provides position/velocity/ratio calculation from entity references.

#pragma once

class CBaseEntity;

// Locus utility functions
// These resolve entity reference strings (which may contain $locus expressions)
// into actual positions, velocities, or scalar ratios.

Vector CalcLocus_Position(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText);
Vector CalcLocus_Velocity(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText);
float CalcLocus_Ratio(CBaseEntity* pLocus, const char* szText);
