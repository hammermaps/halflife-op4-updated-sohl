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
// Based on Spirit of Half-Life 1.2 by Laurie Cheers.

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "locus.h"
#include "effects.h"
#include "alias.h"
#include "movewith.h"

//=========================================================
// CalcLocus_Position
// Resolves a locus position string to an actual position.
// Can be a numeric vector "x y z" or a calc_position entity name.
//=========================================================
Vector CalcLocus_Position(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText)
{
	if (szText == nullptr || szText[0] == '\0')
	{
		if (pLocus)
			return pLocus->pev->origin;
		return g_vecZero;
	}

	if ((*szText >= '0' && *szText <= '9') || *szText == '-')
	{
		Vector tmp;
		UTIL_StringToRandomVector((float*)tmp, szText);
		return tmp;
	}

	CBaseEntity* pCalc = UTIL_FindEntityByTargetname(nullptr, szText);

	if (pCalc != nullptr)
	{
		return pCalc->CalcPosition(pLocus);
	}

	ALERT(at_error, "%s \"%s\" has bad or missing calc_position value \"%s\"\n",
		STRING(pEntity->pev->classname), STRING(pEntity->pev->targetname), szText);
	return g_vecZero;
}

//=========================================================
// CalcLocus_Velocity
// Resolves a locus velocity string to an actual velocity.
// Can be a numeric vector "x y z" or a calc_velocity entity name.
//=========================================================
Vector CalcLocus_Velocity(CBaseEntity* pEntity, CBaseEntity* pLocus, const char* szText)
{
	if (szText == nullptr || szText[0] == '\0')
	{
		if (pLocus)
			return pLocus->pev->velocity;
		return g_vecZero;
	}

	if ((*szText >= '0' && *szText <= '9') || *szText == '-')
	{
		Vector tmp;
		UTIL_StringToRandomVector((float*)tmp, szText);
		return tmp;
	}

	CBaseEntity* pCalc = UTIL_FindEntityByTargetname(nullptr, szText);

	if (pCalc != nullptr)
		return pCalc->CalcVelocity(pLocus);

	ALERT(at_error, "%s \"%s\" has bad or missing calc_velocity value \"%s\"\n",
		STRING(pEntity->pev->classname), STRING(pEntity->pev->targetname), szText);
	return g_vecZero;
}

//=========================================================
// CalcLocus_Ratio
// Resolves a locus ratio string to a scalar value.
// Can be a numeric literal or a calc_ratio entity name.
//=========================================================
float CalcLocus_Ratio(CBaseEntity* pLocus, const char* szText)
{
	if (szText == nullptr || szText[0] == '\0')
		return 0;

	if ((*szText >= '0' && *szText <= '9') || *szText == '-')
	{
		char* pEnd = nullptr;
		float fResult = strtof(szText, &pEnd);
		if (pEnd != szText && (*pEnd == '\0' || *pEnd == ' '))
			return fResult;
		// Fall through to entity lookup if not a clean numeric parse
	}

	CBaseEntity* pCalc = UTIL_FindEntityByTargetname(nullptr, szText);

	if (pCalc != nullptr)
		return pCalc->CalcRatio(pLocus);

	ALERT(at_error, "Bad or missing calc_ratio entity \"%s\"\n", szText);
	return 0;
}


//=============================================
// locus_beam - Creates a temporary beam between two locus points
//=============================================

#define BEAM_FSINE 0x10
#define BEAM_FSOLID 0x20
#define BEAM_FSHADEIN 0x40
#define BEAM_FSHADEOUT 0x80

#define SF_LBEAM_SHADEIN 128
#define SF_LBEAM_SHADEOUT 256
#define SF_LBEAM_SOLID 512
#define SF_LBEAM_SINE 1024

class CLocusBeam : public CPointEntity
{
public:
	void Spawn() override;
	void Precache() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	bool KeyValue(KeyValueData* pkvd) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	int m_iszSprite;
	int m_iszTargetName;
	int m_iszStart;
	int m_iszEnd;
	int m_iWidth;
	int m_iDistortion;
	float m_fFrame;
	int m_iScrollRate;
	float m_fDuration;
	float m_fDamage;
	int m_iDamageType;
	int m_iFlags;
};

TYPEDESCRIPTION CLocusBeam::m_SaveData[] =
{
	DEFINE_FIELD(CLocusBeam, m_iszSprite, FIELD_STRING),
	DEFINE_FIELD(CLocusBeam, m_iszTargetName, FIELD_STRING),
	DEFINE_FIELD(CLocusBeam, m_iszStart, FIELD_STRING),
	DEFINE_FIELD(CLocusBeam, m_iszEnd, FIELD_STRING),
	DEFINE_FIELD(CLocusBeam, m_iWidth, FIELD_INTEGER),
	DEFINE_FIELD(CLocusBeam, m_iDistortion, FIELD_INTEGER),
	DEFINE_FIELD(CLocusBeam, m_fFrame, FIELD_FLOAT),
	DEFINE_FIELD(CLocusBeam, m_iScrollRate, FIELD_INTEGER),
	DEFINE_FIELD(CLocusBeam, m_fDuration, FIELD_FLOAT),
	DEFINE_FIELD(CLocusBeam, m_fDamage, FIELD_FLOAT),
	DEFINE_FIELD(CLocusBeam, m_iDamageType, FIELD_INTEGER),
	DEFINE_FIELD(CLocusBeam, m_iFlags, FIELD_INTEGER),
};

LINK_ENTITY_TO_CLASS(locus_beam, CLocusBeam);
IMPLEMENT_SAVERESTORE(CLocusBeam, CPointEntity);

bool CLocusBeam::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszSprite"))
	{
		m_iszSprite = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszTargetName"))
	{
		m_iszTargetName = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszStart"))
	{
		m_iszStart = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszEnd"))
	{
		m_iszEnd = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iWidth"))
	{
		m_iWidth = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iDistortion"))
	{
		m_iDistortion = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fFrame"))
	{
		m_fFrame = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iScrollRate"))
	{
		m_iScrollRate = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fDuration"))
	{
		m_fDuration = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fDamage"))
	{
		m_fDamage = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iDamageType"))
	{
		m_iDamageType = atoi(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

void CLocusBeam::Precache()
{
	if (!m_iszSprite)
	{
		ALERT(at_error, "locus_beam with no sprite specified\n");
		return;
	}

	const char* pszSprite = STRING(m_iszSprite);
	if (pszSprite == nullptr || pszSprite[0] == '\0')
	{
		ALERT(at_error, "locus_beam with empty sprite specified\n");
		return;
	}

	PRECACHE_MODEL(pszSprite);
}

void CLocusBeam::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity* pStartEnt;
	CBaseEntity* pEndEnt;
	Vector vecStartPos;
	Vector vecEndPos;
	CBeam* pBeam;

	switch (pev->impulse)
	{
	case 0: // ents
		pStartEnt = UTIL_FindEntityByTargetname(nullptr, STRING(m_iszStart));
		pEndEnt = UTIL_FindEntityByTargetname(nullptr, STRING(m_iszEnd));

		if (pStartEnt == nullptr || pEndEnt == nullptr)
			return;
		pBeam = CBeam::BeamCreate(STRING(m_iszSprite), m_iWidth);
		pBeam->EntsInit(pStartEnt->entindex(), pEndEnt->entindex());
		break;

	case 1: // pointent
		vecStartPos = CalcLocus_Position(this, pActivator, STRING(m_iszStart));
		pEndEnt = UTIL_FindEntityByTargetname(nullptr, STRING(m_iszEnd));

		if (pEndEnt == nullptr)
			return;
		pBeam = CBeam::BeamCreate(STRING(m_iszSprite), m_iWidth);
		pBeam->PointEntInit(vecStartPos, pEndEnt->entindex());
		break;

	case 2: // points
		vecStartPos = CalcLocus_Position(this, pActivator, STRING(m_iszStart));
		vecEndPos = CalcLocus_Position(this, pActivator, STRING(m_iszEnd));

		pBeam = CBeam::BeamCreate(STRING(m_iszSprite), m_iWidth);
		pBeam->PointsInit(vecStartPos, vecEndPos);
		break;

	case 3: // point & offset
		vecStartPos = CalcLocus_Position(this, pActivator, STRING(m_iszStart));
		vecEndPos = CalcLocus_Velocity(this, pActivator, STRING(m_iszEnd));

		pBeam = CBeam::BeamCreate(STRING(m_iszSprite), m_iWidth);
		pBeam->PointsInit(vecStartPos, vecStartPos + vecEndPos);
		break;

	default:
		return;
	}

	pBeam->SetColor(pev->rendercolor.x, pev->rendercolor.y, pev->rendercolor.z);
	pBeam->SetBrightness(pev->renderamt);
	pBeam->SetNoise(m_iDistortion);
	pBeam->SetFrame(m_fFrame);
	pBeam->SetScrollRate(m_iScrollRate);
	pBeam->SetFlags(m_iFlags);
	pBeam->pev->dmg = m_fDamage;
	pBeam->pev->frags = m_iDamageType;
	pBeam->pev->spawnflags |= pev->spawnflags & (SF_BEAM_RING |
		SF_BEAM_SPARKSTART | SF_BEAM_SPARKEND | SF_BEAM_DECALS);

	if (m_fDuration)
	{
		pBeam->SetThink(&CBeam::SUB_Remove);
		pBeam->SetNextThink(m_fDuration);
	}

	pBeam->pev->targetname = m_iszTargetName;

	if (pev->target)
	{
		FireTargets(STRING(pev->target), pBeam, this, USE_TOGGLE, 0);
	}
}

void CLocusBeam::Spawn()
{
	Precache();
	m_iFlags = 0;
	if (pev->spawnflags & SF_LBEAM_SHADEIN)
		m_iFlags |= BEAM_FSHADEIN;
	if (pev->spawnflags & SF_LBEAM_SHADEOUT)
		m_iFlags |= BEAM_FSHADEOUT;
	if (pev->spawnflags & SF_LBEAM_SINE)
		m_iFlags |= BEAM_FSINE;
	if (pev->spawnflags & SF_LBEAM_SOLID)
		m_iFlags |= BEAM_FSOLID;
}


//=============================================
// calc_position - Returns a position derived from another entity
//=============================================

class CCalcPosition : public CPointEntity
{
public:
	Vector CalcPosition(CBaseEntity* pLocus) override;
};

LINK_ENTITY_TO_CLASS(calc_position, CCalcPosition);

Vector CCalcPosition::CalcPosition(CBaseEntity* pLocus)
{
	CBaseEntity* pSubject = UTIL_FindEntityByTargetname(nullptr, STRING(pev->netname));
	if (pSubject == nullptr)
		pSubject = pLocus;
	if (pSubject == nullptr)
		return g_vecZero;

	Vector vecOffset = CalcLocus_Velocity(this, pLocus, STRING(pev->message));

	Vector vecPosition;
	Vector vecJunk;

	switch (pev->impulse)
	{
	case 1: // eyes
		return vecOffset + pSubject->EyePosition();
	case 2: // top
		return vecOffset + pSubject->pev->origin + Vector(
			(pSubject->pev->mins.x + pSubject->pev->maxs.x) / 2,
			(pSubject->pev->mins.y + pSubject->pev->maxs.y) / 2,
			pSubject->pev->maxs.z);
	case 3: // centre
		return vecOffset + pSubject->pev->origin + Vector(
			(pSubject->pev->mins.x + pSubject->pev->maxs.x) / 2,
			(pSubject->pev->mins.y + pSubject->pev->maxs.y) / 2,
			(pSubject->pev->mins.z + pSubject->pev->maxs.z) / 2);
	case 4: // bottom
		return vecOffset + pSubject->pev->origin + Vector(
			(pSubject->pev->mins.x + pSubject->pev->maxs.x) / 2,
			(pSubject->pev->mins.y + pSubject->pev->maxs.y) / 2,
			pSubject->pev->mins.z);
	case 5:
		((CBaseAnimating*)pSubject)->GetAttachment(0, vecPosition, vecJunk);
		return vecOffset + vecPosition;
	case 6:
		((CBaseAnimating*)pSubject)->GetAttachment(1, vecPosition, vecJunk);
		return vecOffset + vecPosition;
	case 7:
		((CBaseAnimating*)pSubject)->GetAttachment(2, vecPosition, vecJunk);
		return vecOffset + vecPosition;
	case 8:
		((CBaseAnimating*)pSubject)->GetAttachment(3, vecPosition, vecJunk);
		return vecOffset + vecPosition;
	case 9: // random point in bbox
		return vecOffset + pSubject->pev->origin + Vector(
			RANDOM_FLOAT(pSubject->pev->mins.x, pSubject->pev->maxs.x),
			RANDOM_FLOAT(pSubject->pev->mins.y, pSubject->pev->maxs.y),
			RANDOM_FLOAT(pSubject->pev->mins.z, pSubject->pev->maxs.z));
	default: // origin
		return vecOffset + pSubject->pev->origin;
	}
}


//=============================================
// calc_ratio - Performs math on a ratio value
//=============================================

class CCalcRatio : public CPointEntity
{
public:
	float CalcRatio(CBaseEntity* pLocus) override;
};

LINK_ENTITY_TO_CLASS(calc_ratio, CCalcRatio);

float CCalcRatio::CalcRatio(CBaseEntity* pLocus)
{
	float fBasis = CalcLocus_Ratio(pLocus, STRING(pev->target));

	switch (pev->impulse)
	{
	case 1:
		fBasis = 1 - fBasis;
		break; // reversed
	case 2:
		fBasis = -fBasis;
		break; // negative
	case 3:
		if (fBasis != 0)
			fBasis = 1 / fBasis;
		break; // reciprocal
	}

	fBasis += CalcLocus_Ratio(pLocus, STRING(pev->netname));
	fBasis = fBasis * CalcLocus_Ratio(pLocus, STRING(pev->message));

	if (!FStringNull(pev->noise))
	{
		float fMin = CalcLocus_Ratio(pLocus, STRING(pev->noise));

		if (!FStringNull(pev->noise1))
		{
			float fMax = CalcLocus_Ratio(pLocus, STRING(pev->noise1));

			if (fBasis >= fMin && fBasis <= fMax)
				return fBasis;

			switch ((int)pev->frags)
			{
			case 0: // clamp
				if (fBasis < fMin)
					return fMin;
				else
					return fMax;
			case 1: // wrap
				while (fBasis < fMin)
					fBasis += fMax - fMin;
				while (fBasis > fMax)
					fBasis -= fMax - fMin;
				return fBasis;
			case 2: // bounce
				while (fBasis < fMin || fBasis > fMax)
				{
					if (fBasis < fMin)
						fBasis = fMin + fMax - fBasis;
					else
						fBasis = fMax + fMax - fBasis;
				}
				return fBasis;
			}
		}

		if (fBasis > fMin)
			return fBasis;
		else
			return fMin;
	}
	else if (!FStringNull(pev->noise1))
	{
		float fMax = CalcLocus_Ratio(pLocus, STRING(pev->noise1));

		if (fBasis < fMax)
			return fBasis;
		else
			return fMax;
	}

	return fBasis;
}


//=============================================
// calc_subvelocity - Derives a velocity from another entity's
// velocity, angles, or attachment orientations
//=============================================

#define SF_CALCVELOCITY_NORMALIZE 1
#define SF_CALCVELOCITY_SWAPZ 2

class CCalcSubVelocity : public CPointEntity
{
	Vector Convert(CBaseEntity* pLocus, Vector vecVel);
	Vector ConvertAngles(CBaseEntity* pLocus, Vector vecAngles);

public:
	Vector CalcVelocity(CBaseEntity* pLocus) override;
};

LINK_ENTITY_TO_CLASS(calc_subvelocity, CCalcSubVelocity);

// Backwards compatibility: calc_velocity maps to the same class
LINK_ENTITY_TO_CLASS(calc_velocity, CCalcSubVelocity);

Vector CCalcSubVelocity::CalcVelocity(CBaseEntity* pLocus)
{
	CBaseEntity* pSubject = UTIL_FindEntityByTargetname(nullptr, STRING(pev->netname));
	if (pSubject == nullptr)
		pSubject = pLocus;
	if (pSubject == nullptr)
		return g_vecZero;

	Vector vecAngles;
	Vector vecJunk;

	switch (pev->impulse)
	{
	case 1: // angles
		return ConvertAngles(pSubject, pSubject->pev->angles);
	case 2: // v_angle
		return ConvertAngles(pSubject, pSubject->pev->v_angle);
	case 5:
		((CBaseAnimating*)pSubject)->GetAttachment(0, vecJunk, vecAngles);
		return ConvertAngles(pSubject, vecAngles);
	case 6:
		((CBaseAnimating*)pSubject)->GetAttachment(1, vecJunk, vecAngles);
		return ConvertAngles(pSubject, vecAngles);
	case 7:
		((CBaseAnimating*)pSubject)->GetAttachment(2, vecJunk, vecAngles);
		return ConvertAngles(pSubject, vecAngles);
	case 8:
		((CBaseAnimating*)pSubject)->GetAttachment(3, vecJunk, vecAngles);
		return ConvertAngles(pSubject, vecAngles);
	default: // velocity
		return Convert(pSubject, pSubject->pev->velocity);
	}
}

Vector CCalcSubVelocity::Convert(CBaseEntity* pLocus, Vector vecDir)
{
	if (pev->spawnflags & SF_CALCVELOCITY_NORMALIZE)
		vecDir = vecDir.Normalize();

	float fRatio = CalcLocus_Ratio(pLocus, STRING(pev->noise));
	Vector vecOffset = CalcLocus_Velocity(this, pLocus, STRING(pev->message));

	Vector vecResult = vecOffset + (vecDir * fRatio);

	if (pev->spawnflags & SF_CALCVELOCITY_SWAPZ)
		vecResult.z = -vecResult.z;

	return vecResult;
}

Vector CCalcSubVelocity::ConvertAngles(CBaseEntity* pLocus, Vector vecAngles)
{
	UTIL_MakeVectors(vecAngles);
	return Convert(pLocus, gpGlobals->v_forward);
}


//=============================================
// calc_velocity_path - Calculates velocity along a path between
// two points, with optional length transforms and trace clipping
//=============================================

class CCalcVelocityPath : public CPointEntity
{
public:
	Vector CalcVelocity(CBaseEntity* pLocus) override;
};

LINK_ENTITY_TO_CLASS(calc_velocity_path, CCalcVelocityPath);

Vector CCalcVelocityPath::CalcVelocity(CBaseEntity* pLocus)
{
	Vector vecStart = CalcLocus_Position(this, pLocus, STRING(pev->target));
	Vector vecOffs;
	float fFactor = CalcLocus_Ratio(pLocus, STRING(pev->noise));

	switch ((int)pev->armorvalue)
	{
	case 0:
		vecOffs = CalcLocus_Position(this, pLocus, STRING(pev->netname)) - vecStart;
		break;
	case 1:
		vecOffs = CalcLocus_Velocity(this, pLocus, STRING(pev->netname));
		break;
	default:
		vecOffs = g_vecZero;
		break;
	}

	if (pev->health)
	{
		float len = vecOffs.Length();
		if (len > 0)
		{
			switch ((int)pev->health)
			{
			case 1:
				vecOffs = vecOffs / len;
				break;
			case 2:
				vecOffs = vecOffs / (len * len);
				break;
			case 3:
				vecOffs = vecOffs / (len * len * len);
				break;
			case 4:
				vecOffs = vecOffs * len;
				break;
			}
		}
	}

	vecOffs = vecOffs * fFactor;

	if (pev->frags)
	{
		TraceResult tr;
		IGNORE_GLASS iIgnoreGlass = ignore_glass;
		IGNORE_MONSTERS iIgnoreMonsters = ignore_monsters;

		switch ((int)pev->frags)
		{
		case 2:
			iIgnoreGlass = dont_ignore_glass;
			break;
		case 4:
			iIgnoreGlass = dont_ignore_glass;
			// fall through
		case 3:
			iIgnoreMonsters = dont_ignore_monsters;
			break;
		}

		UTIL_TraceLine(vecStart, vecStart + vecOffs, iIgnoreMonsters, iIgnoreGlass, nullptr, &tr);
		vecOffs = tr.vecEndPos - vecStart;
	}

	return vecOffs;
}


//=============================================
// calc_velocity_polar - Converts a velocity from Cartesian to polar,
// applies angle offsets, and converts back
//=============================================

class CCalcVelocityPolar : public CPointEntity
{
public:
	Vector CalcVelocity(CBaseEntity* pLocus) override;
};

LINK_ENTITY_TO_CLASS(calc_velocity_polar, CCalcVelocityPolar);

Vector CCalcVelocityPolar::CalcVelocity(CBaseEntity* pLocus)
{
	Vector vecBasis = CalcLocus_Velocity(this, pLocus, STRING(pev->netname));
	Vector vecAngles = UTIL_VecToAngles(vecBasis) + pev->angles;
	Vector vecOffset = CalcLocus_Velocity(this, pLocus, STRING(pev->message));

	float fFactor = CalcLocus_Ratio(pLocus, STRING(pev->noise));

	if (!(pev->spawnflags & SF_CALCVELOCITY_NORMALIZE))
		fFactor = fFactor * vecBasis.Length();

	UTIL_MakeVectors(vecAngles);
	return (gpGlobals->v_forward * fFactor) + vecOffset;
}


//=============================================
// CMark - Temporary position/velocity/ratio marker entity
// Created by locus_variable, auto-removes after duration
//=============================================

class CMark : public CPointEntity
{
public:
	Vector CalcVelocity(CBaseEntity* pLocus) override { return pev->movedir; }
	float CalcRatio(CBaseEntity* pLocus) override { return pev->frags; }
	void Think() override { SUB_Remove(); }
};


//=============================================
// locus_variable - Spawns temporary CMark entities to hold
// calculated position/velocity/ratio values
//=============================================

class CLocusVariable : public CPointEntity
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	Vector CalcVelocity(CBaseEntity* pLocus) override { return pev->movedir; }
	float CalcRatio(CBaseEntity* pLocus) override { return pev->frags; }

	bool KeyValue(KeyValueData* pkvd) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	int m_iszPosition;
	int m_iszVelocity;
	int m_iszRatio;
	int m_iszTargetName;
	int m_iszFireOnSpawn;
	float m_fDuration;
};

TYPEDESCRIPTION CLocusVariable::m_SaveData[] =
{
	DEFINE_FIELD(CLocusVariable, m_iszPosition, FIELD_STRING),
	DEFINE_FIELD(CLocusVariable, m_iszVelocity, FIELD_STRING),
	DEFINE_FIELD(CLocusVariable, m_iszRatio, FIELD_STRING),
	DEFINE_FIELD(CLocusVariable, m_iszTargetName, FIELD_STRING),
	DEFINE_FIELD(CLocusVariable, m_iszFireOnSpawn, FIELD_STRING),
	DEFINE_FIELD(CLocusVariable, m_fDuration, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CLocusVariable, CPointEntity);
LINK_ENTITY_TO_CLASS(locus_variable, CLocusVariable);

bool CLocusVariable::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszPosition"))
	{
		m_iszPosition = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszVelocity"))
	{
		m_iszVelocity = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszRatio"))
	{
		m_iszRatio = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszTargetName"))
	{
		m_iszTargetName = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszFireOnSpawn"))
	{
		m_iszFireOnSpawn = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fDuration"))
	{
		m_fDuration = atof(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

void CLocusVariable::Spawn()
{
	SetMovedir(pev);
}

void CLocusVariable::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	Vector vecPos = g_vecZero;
	Vector vecDir = g_vecZero;
	float fRatio = 0;

	if (m_iszPosition)
		vecPos = CalcLocus_Position(this, pActivator, STRING(m_iszPosition));
	if (m_iszVelocity)
		vecDir = CalcLocus_Velocity(this, pActivator, STRING(m_iszVelocity));
	if (m_iszRatio)
		fRatio = CalcLocus_Ratio(pActivator, STRING(m_iszRatio));

	if (m_iszTargetName)
	{
		CMark* pMark = GetClassPtr((CMark*)nullptr);
		pMark->pev->classname = MAKE_STRING("mark");
		pMark->pev->origin = vecPos;
		pMark->pev->movedir = vecDir;
		pMark->pev->frags = fRatio;
		pMark->pev->targetname = m_iszTargetName;
		pMark->SetNextThink(m_fDuration);

		FireTargets(STRING(m_iszFireOnSpawn), pMark, this, USE_TOGGLE, 0);
	}
	else
	{
		pev->origin = vecPos;
		pev->movedir = vecDir;
		pev->frags = fRatio;

		FireTargets(STRING(m_iszFireOnSpawn), this, this, USE_TOGGLE, 0);
	}
}

//=========================================================
// locus_alias - An alias entity that refers to the activator.
// When triggered, sets its value to the activator entity.
// LRC - ported from SoHL 1.2
//=========================================================
class CLocusAlias : public CBaseAlias
{
public:
void PostSpawn() override;
void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
CBaseEntity* FollowAlias(CBaseEntity* pFrom) override;
void FlushChanges() override;

bool Save(CSave& save) override;
bool Restore(CRestore& restore) override;
static TYPEDESCRIPTION m_SaveData[];

EHANDLE m_hValue;
EHANDLE m_hChangeTo;
};

TYPEDESCRIPTION CLocusAlias::m_SaveData[] =
{
DEFINE_FIELD(CLocusAlias, m_hValue, FIELD_EHANDLE),
DEFINE_FIELD(CLocusAlias, m_hChangeTo, FIELD_EHANDLE),
};

LINK_ENTITY_TO_CLASS(locus_alias, CLocusAlias);
IMPLEMENT_SAVERESTORE(CLocusAlias, CBaseAlias);

void CLocusAlias::PostSpawn()
{
	m_hValue = nullptr;

	if (!FStringNull(pev->netname))
	{
		m_hValue = UTIL_FindEntityByTargetname(nullptr, STRING(pev->netname));
	}
}

void CLocusAlias::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
m_hChangeTo = pActivator;
UTIL_AddToAliasList(this);
}

void CLocusAlias::FlushChanges()
{
m_hValue = m_hChangeTo;
m_hChangeTo = nullptr;
}

CBaseEntity* CLocusAlias::FollowAlias(CBaseEntity* pFrom)
{
if (m_hValue == nullptr)
return nullptr;
else if (pFrom == nullptr || (OFFSET(m_hValue->pev) > OFFSET(pFrom->pev)))
return m_hValue;
else
return nullptr;
}
