/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// Zombie Soldier — soldier variant (inherits from CZombieBase)
//=========================================================

#include "zombie_base.h"

class CZombieSoldier : public CZombieBase
{
public:
	void Spawn() override;
	void Precache() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;

protected:
	float GetOneSlashDamage() override { return gSkillData.zombieSoldierDmgOneSlash; }
	float GetBothSlashDamage() override { return gSkillData.zombieSoldierDmgBothSlash; }
	const char* GetDefaultModel() override { return "models/zombie_soldier.mdl"; }
};

LINK_ENTITY_TO_CLASS(monster_zombie_soldier, CZombieSoldier);

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CZombieSoldier::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	ZombieHandleAnimEvent(pEvent);
}

//=========================================================
// Spawn
//=========================================================
void CZombieSoldier::Spawn()
{
	pev->health = gSkillData.zombieSoldierHealth;
	ZombieSpawnHelper();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CZombieSoldier::Precache()
{
	ZombiePrecacheHelper();
}

//=========================================================
// DEAD ZOMBIE SOLDIER PROP
//=========================================================
class CDeadZombieSoldier : public CBaseMonster
{
public:
	void Spawn() override;
	int Classify() override { return CLASS_ALIEN_MONSTER; }

	bool KeyValue(KeyValueData* pkvd) override;

	int m_iPose;
	static const char* m_szPoses[];
};

const char* CDeadZombieSoldier::m_szPoses[] = {"dead_on_stomach", "dead_on_back"};

bool CDeadZombieSoldier::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		return true;
	}

	return CBaseMonster::KeyValue(pkvd);
}

LINK_ENTITY_TO_CLASS(monster_zombie_soldier_dead, CDeadZombieSoldier);

//=========================================================
// ********** DeadZombieSoldier SPAWN **********
//=========================================================
void CDeadZombieSoldier::Spawn()
{
	PRECACHE_MODEL("models/zombie_soldier.mdl");
	SET_MODEL(ENT(pev), "models/zombie_soldier.mdl");

	pev->effects = 0;
	pev->yaw_speed = 8;
	pev->sequence = 0;
	m_bloodColor = BLOOD_COLOR_RED;

	if (m_iPose < 0 || m_iPose >= (int)ARRAYSIZE(m_szPoses))
		m_iPose = 0;

	pev->sequence = LookupSequence(m_szPoses[m_iPose]);

	if (pev->sequence == -1)
	{
		ALERT(at_console, "Dead zombie soldier with bad pose\n");
	}

	// Corpses have less health
	pev->health = 8;

	MonsterInitDead();
}
