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
// Zombie Barney — guard variant (inherits from CZombieBase)
//=========================================================

#include "zombie_base.h"

class CZombieBarney : public CZombieBase
{
public:
	void Spawn() override;
	void Precache() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;

protected:
	float GetOneSlashDamage() override { return gSkillData.zombieBarneyDmgOneSlash; }
	float GetBothSlashDamage() override { return gSkillData.zombieBarneyDmgBothSlash; }
	const char* GetDefaultModel() override { return "models/zombie_barney.mdl"; }
};

LINK_ENTITY_TO_CLASS(monster_zombie_barney, CZombieBarney);

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CZombieBarney::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	ZombieHandleAnimEvent(pEvent);
}

//=========================================================
// Spawn
//=========================================================
void CZombieBarney::Spawn()
{
	pev->health = gSkillData.zombieBarneyHealth;
	ZombieSpawnHelper();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CZombieBarney::Precache()
{
	ZombiePrecacheHelper();
}
