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

// LRC - Alias system implementation

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "alias.h"

// Global alias list head
CBaseAlias* g_pAliasList = nullptr;

//=========================================================
// CBaseAlias
//=========================================================
TYPEDESCRIPTION CBaseAlias::m_SaveData[] =
	{
		DEFINE_FIELD(CBaseAlias, m_pNextAlias, FIELD_CLASSPTR),
};

IMPLEMENT_SAVERESTORE(CBaseAlias, CPointEntity);

//=========================================================
// CInfoAlias - A simple alias that refers to a single entity
//=========================================================
LINK_ENTITY_TO_CLASS(info_alias, CInfoAlias);

TYPEDESCRIPTION CInfoAlias::m_SaveData[] =
	{
		DEFINE_FIELD(CInfoAlias, m_cTargets, FIELD_INTEGER),
		DEFINE_ARRAY(CInfoAlias, m_iszTargets, FIELD_STRING, 16),
		DEFINE_FIELD(CInfoAlias, m_iCurrentTarget, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CInfoAlias, CBaseAlias);

void CInfoAlias::Spawn()
{
	m_pCachedEntity = nullptr;
	m_bDirty = true;

	// Add to global alias list
	m_pNextAlias = g_pAliasList;
	g_pAliasList = this;
}

void CInfoAlias::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// Cycle through targets
	if (useType == USE_ON || useType == USE_TOGGLE)
	{
		m_iCurrentTarget++;
		if (m_iCurrentTarget >= m_cTargets)
			m_iCurrentTarget = 0;
	}
	else if (useType == USE_OFF)
	{
		m_iCurrentTarget = 0;
	}

	m_bDirty = true;
	m_pCachedEntity = nullptr;
}

CBaseEntity* CInfoAlias::FollowAlias(CBaseEntity* pFrom)
{
	if (m_cTargets == 0)
		return nullptr;

	if (m_iCurrentTarget >= m_cTargets)
		m_iCurrentTarget = 0;

	const char* sTarget = STRING(m_iszTargets[m_iCurrentTarget]);

	CBaseEntity* pTarget;
	if (pFrom)
		pTarget = UTIL_FindEntityByTargetname(pFrom, sTarget);
	else
		pTarget = UTIL_FindEntityByTargetname(nullptr, sTarget);

	return pTarget;
}

void CInfoAlias::FlushChanges()
{
	m_pCachedEntity = nullptr;
	m_bDirty = true;
}

//=========================================================
// CTriggerChangeAlias - Changes which entity an alias points to
//=========================================================
LINK_ENTITY_TO_CLASS(trigger_changealias, CTriggerChangeAlias);

void CTriggerChangeAlias::Spawn()
{
}

bool CTriggerChangeAlias::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "target"))
	{
		pev->target = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "newtarget"))
	{
		m_iszNewTarget = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	return CBaseEntity::KeyValue(pkvd);
}

void CTriggerChangeAlias::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// Find the alias entity
	CBaseEntity* pAlias = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));
	if (!pAlias)
	{
		ALERT(at_error, "trigger_changealias \"%s\" can't find alias \"%s\"\n",
			STRING(pev->targetname), STRING(pev->target));
		return;
	}

	// For info_alias, update the target
	CInfoAlias* pInfoAlias = dynamic_cast<CInfoAlias*>(pAlias);
	if (pInfoAlias && !FStringNull(m_iszNewTarget))
	{
		pInfoAlias->m_cTargets = 1;
		pInfoAlias->m_iszTargets[0] = m_iszNewTarget;
		pInfoAlias->m_iCurrentTarget = 0;
		pInfoAlias->m_pCachedEntity = nullptr;
		pInfoAlias->m_bDirty = true;
	}
}

//=========================================================
// FlushAliases - called after firing targets to let aliases
// reflect any new values
//=========================================================
void FlushAliases()
{
	CBaseAlias* pAlias = g_pAliasList;
	while (pAlias)
	{
		pAlias->FlushChanges();
		pAlias = pAlias->m_pNextAlias;
	}
}

//=========================================================
// InitAliasListOnRestore - rebuild the alias list after restore
//=========================================================
void InitAliasListOnRestore()
{
	g_pAliasList = nullptr;

	edict_t* pEdict = UTIL_GetEntityList();
	if (!pEdict)
		return;

	for (int i = 0; i < gpGlobals->maxEntities; i++, pEdict++)
	{
		if (pEdict->free)
			continue;

		CBaseEntity* pEntity = CBaseEntity::Instance(pEdict);
		if (!pEntity)
			continue;

		CBaseAlias* pAlias = dynamic_cast<CBaseAlias*>(pEntity);
		if (pAlias)
		{
			pAlias->m_pNextAlias = g_pAliasList;
			g_pAliasList = pAlias;
		}
	}
}
