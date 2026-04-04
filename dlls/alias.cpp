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
#include "logger.h"

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
		DEFINE_ARRAY(CInfoAlias, m_iszTargets, FIELD_STRING, MAX_ALIAS_TARGETS),
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

void CInfoAlias::ChangeValue(string_t iszValue)
{
	// Update to point to a single new target
	m_cTargets = 1;
	m_iszTargets[0] = iszValue;
	m_iCurrentTarget = 0;
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
		LOG_ERROR("trigger_changealias \"%s\" can't find alias \"%s\"",
			STRING(pev->targetname), STRING(pev->target));
		return;
	}

	if (!pAlias->IsAlias())
	{
		LOG_ERROR("trigger_changealias \"%s\": target \"%s\" is not an alias!",
			STRING(pev->targetname), STRING(pev->target));
		return;
	}

	if (!FStringNull(m_iszNewTarget))
		static_cast<CBaseAlias*>(pAlias)->ChangeValue(m_iszNewTarget);
}

//=========================================================
// CInfoGroup
// LRC - A group entity that provides a map of member name -> value.
// When triggered, updates the target alias to reference this group.
//=========================================================
LINK_ENTITY_TO_CLASS(info_group, CInfoGroup);

TYPEDESCRIPTION CInfoGroup::m_SaveData[] =
	{
		DEFINE_FIELD(CInfoGroup, m_cMembers, FIELD_INTEGER),
		DEFINE_ARRAY(CInfoGroup, m_iszMemberName, FIELD_STRING, MAX_ALIAS_TARGETS),
		DEFINE_ARRAY(CInfoGroup, m_iszMemberValue, FIELD_STRING, MAX_ALIAS_TARGETS),
		DEFINE_FIELD(CInfoGroup, m_iszDefaultMember, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CInfoGroup, CPointEntity);

bool CInfoGroup::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "defaultmember"))
	{
		m_iszDefaultMember = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (m_cMembers < MAX_ALIAS_TARGETS)
	{
		char tmp[128];
		UTIL_StripToken(pkvd->szKeyName, tmp, sizeof(tmp));
		m_iszMemberName[m_cMembers] = ALLOC_STRING(tmp);
		m_iszMemberValue[m_cMembers] = ALLOC_STRING(pkvd->szValue);
		m_cMembers++;
		return true;
	}
	else
	{
		LOG_ERROR("Too many members for info_group \"%s\" (limit is %d)",
			STRING(pev->targetname), MAX_ALIAS_TARGETS);
	}
	return CPointEntity::KeyValue(pkvd);
}

void CInfoGroup::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (FStringNull(pev->target))
		return;

	CBaseEntity* pTarget = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));
	if (pTarget && pTarget->IsAlias())
	{
		if (pev->spawnflags & SF_GROUP_DEBUG)
			LOG_DEBUG("info_group \"%s\" changes alias \"%s\"",
				STRING(pev->targetname), STRING(pTarget->pev->targetname));
		static_cast<CBaseAlias*>(pTarget)->ChangeValue(this);
	}
	else if (pev->target)
	{
		LOG_ERROR("info_group \"%s\": alias \"%s\" was not found or not an alias!",
			STRING(pev->targetname), STRING(pev->target));
	}
}

string_t CInfoGroup::GetMember(const char* szMemberName)
{
	if (!szMemberName)
		return iStringNull;

	for (int i = 0; i < m_cMembers; i++)
	{
		if (FStrEq(szMemberName, STRING(m_iszMemberName[i])))
			return m_iszMemberValue[i];
	}

	if (!FStringNull(m_iszDefaultMember))
	{
		static char szBuffer[128];
		snprintf(szBuffer, sizeof(szBuffer), "%s%s", STRING(m_iszDefaultMember), szMemberName);
		return ALLOC_STRING(szBuffer);
	}

	LOG_DEBUG("info_group \"%s\" has no member called \"%s\".",
		STRING(pev->targetname), szMemberName);
	return iStringNull;
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

//=========================================================
// multi_alias
// LRC - alias entity that references multiple targets.
//=========================================================
LINK_ENTITY_TO_CLASS(multi_alias, CMultiAlias);

TYPEDESCRIPTION CMultiAlias::m_SaveData[] =
{
DEFINE_FIELD(CMultiAlias, m_cTargets, FIELD_INTEGER),
DEFINE_ARRAY(CMultiAlias, m_iszTargets, FIELD_STRING, MAX_ALIAS_TARGETS),
DEFINE_ARRAY(CMultiAlias, m_fTargetType, FIELD_INTEGER, MAX_ALIAS_TARGETS),
};

IMPLEMENT_SAVERESTORE(CMultiAlias, CBaseAlias);

bool CMultiAlias::KeyValue(KeyValueData* pkvd)
{
if (m_cTargets < MAX_ALIAS_TARGETS)
{
char tmp[128];
UTIL_StripToken(pkvd->szKeyName, tmp, sizeof(tmp));
m_iszTargets[m_cTargets] = ALLOC_STRING(tmp);
m_fTargetType[m_cTargets] = atoi(pkvd->szValue);
m_cTargets++;
return true;
}
else
{
LOG_ERROR("Too many targets for multi_alias %s (limit is %d)", STRING(pev->targetname), MAX_ALIAS_TARGETS);
}
return CBaseAlias::KeyValue(pkvd);
}

CBaseEntity* CMultiAlias::FollowAlias(CBaseEntity* pFrom)
{
CBaseEntity* pBest = nullptr;

for (int i = 0; i < m_cTargets; i++)
{
CBaseEntity* pTemp;
if (m_fTargetType[i])
pTemp = UTIL_FindEntityByClassname(pFrom, STRING(m_iszTargets[i]));
else
pTemp = UTIL_FindEntityByTargetname(pFrom, STRING(m_iszTargets[i]));

if (pTemp)
{
// Return the first match found
if (!pBest)
pBest = pTemp;
}
}

return pBest;
}
