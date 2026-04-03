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

// LRC - Alias system
// Allows entities to be referenced by alias names rather than just targetnames.

#pragma once

class CBaseEntity;
class CInfoGroup;

#define MAX_ALIAS_TARGETS 16

//=========================================================
// CBaseAlias
// Base class for alias entities. An alias maps a name to
// one or more actual entities, enabling dynamic targeting.
//=========================================================
class CBaseAlias : public CPointEntity
{
public:
	bool IsAlias() override { return true; }  // LRC - marks this as an alias entity

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	// Returns the next entity referred to by this alias
	virtual CBaseEntity* FollowAlias(CBaseEntity* pFrom) { return nullptr; }
	// Flush any cached values (called after targets fire)
	virtual void FlushChanges() {}

	// LRC - change the alias value to a string
	virtual void ChangeValue(string_t iszValue)
	{
		ALERT(at_error, "%s entities cannot change value!\n", STRING(pev->classname));
	}
	// LRC - change the alias value using another entity's targetname
	virtual void ChangeValue(CBaseEntity* pValue) { ChangeValue(pValue->pev->targetname); }

	// Linked list of all alias entities
	CBaseAlias* m_pNextAlias;
};

//=========================================================
// info_alias
// A simple alias that refers to a single entity by targetname.
//=========================================================
class CInfoAlias : public CBaseAlias
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	CBaseEntity* FollowAlias(CBaseEntity* pFrom) override;
	void FlushChanges() override;
	void ChangeValue(string_t iszValue) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	int m_cTargets;
	int m_iszTargets[MAX_ALIAS_TARGETS];
	int m_iCurrentTarget;
	CBaseEntity* m_pCachedEntity;
	bool m_bDirty;
};

//=========================================================
// info_group
// LRC - A group entity that maps member names to string values.
// When triggered, changes the target alias to point to this group,
// so that locus-based lookups can resolve member values.
//=========================================================
#define SF_GROUP_DEBUG 2

class CInfoGroup : public CPointEntity
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	// Returns the string_t value of the named member, or iDefault if not found
	string_t GetMember(const char* szMemberName);

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	int m_cMembers;
	int m_iszMemberName[MAX_ALIAS_TARGETS];
	int m_iszMemberValue[MAX_ALIAS_TARGETS];
	int m_iszDefaultMember;  // prefix appended to member name when not found
};

//=========================================================
// trigger_changealias
// Changes which entity an alias points to.
//=========================================================
class CTriggerChangeAlias : public CBaseEntity
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	int m_iszNewTarget;
};
