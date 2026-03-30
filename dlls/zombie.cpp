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
// Zombie — standard headcrab zombie (inherits from CZombieBase)
//=========================================================

#include "zombie_base.h"

class CZombie : public CZombieBase
{
public:
	void Spawn() override;
	void Precache() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;

protected:
	float GetOneSlashDamage() override { return gSkillData.zombieDmgOneSlash; }
	float GetBothSlashDamage() override { return gSkillData.zombieDmgBothSlash; }
	const char* GetDefaultModel() override { return "models/zombie.mdl"; }
};

LINK_ENTITY_TO_CLASS(monster_zombie, CZombie);

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CZombie::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	ZombieHandleAnimEvent(pEvent);
}

//=========================================================
// Spawn
//=========================================================
void CZombie::Spawn()
{
	pev->health = gSkillData.zombieHealth;
	ZombieSpawnHelper();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CZombie::Precache()
{
	ZombiePrecacheHelper();
}
