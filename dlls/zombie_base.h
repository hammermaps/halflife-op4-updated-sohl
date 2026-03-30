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
// CZombieBase — shared base class for all zombie variants
// (monster_zombie, monster_zombie_soldier, monster_zombie_barney)
//=========================================================

#pragma once

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define ZOMBIE_AE_ATTACK_RIGHT 0x01
#define ZOMBIE_AE_ATTACK_LEFT 0x02
#define ZOMBIE_AE_ATTACK_BOTH 0x03

#define ZOMBIE_FLINCH_DELAY 2

//=========================================================
// Shared sound arrays — identical for all zombie variants
//=========================================================
inline const char* g_pZombieAttackHitSounds[] =
	{
		"zombie/claw_strike1.wav",
		"zombie/claw_strike2.wav",
		"zombie/claw_strike3.wav",
};

inline const char* g_pZombieAttackMissSounds[] =
	{
		"zombie/claw_miss1.wav",
		"zombie/claw_miss2.wav",
};

inline const char* g_pZombieAttackSounds[] =
	{
		"zombie/zo_attack1.wav",
		"zombie/zo_attack2.wav",
};

inline const char* g_pZombieIdleSounds[] =
	{
		"zombie/zo_idle1.wav",
		"zombie/zo_idle2.wav",
		"zombie/zo_idle3.wav",
		"zombie/zo_idle4.wav",
};

inline const char* g_pZombieAlertSounds[] =
	{
		"zombie/zo_alert10.wav",
		"zombie/zo_alert20.wav",
		"zombie/zo_alert30.wav",
};

inline const char* g_pZombiePainSounds[] =
	{
		"zombie/zo_pain1.wav",
		"zombie/zo_pain2.wav",
};

//=========================================================
// CZombieBase — base class providing common zombie behavior
//=========================================================
class CZombieBase : public CBaseMonster
{
public:
	void SetYawSpeed() override;
	int Classify() override;
	int IgnoreConditions() override;

	void PainSound() override;
	void AlertSound() override;
	void IdleSound() override;
	void AttackSound();

	bool CheckRangeAttack1(float flDot, float flDist) override { return false; }
	bool CheckRangeAttack2(float flDot, float flDist) override { return false; }
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

	float m_flNextFlinch;

protected:
	virtual float GetOneSlashDamage() = 0;
	virtual float GetBothSlashDamage() = 0;
	virtual const char* GetDefaultModel() = 0;

	void ZombieSpawnHelper();
	void ZombiePrecacheHelper();
	void ZombieHandleAnimEvent(MonsterEvent_t* pEvent);
};

//=========================================================
// Inline implementations
//=========================================================

inline void CZombieBase::SetYawSpeed()
{
	pev->yaw_speed = 120;
}

inline int CZombieBase::Classify()
{
	return m_iClass ? m_iClass : CLASS_ALIEN_MONSTER;
}

inline bool CZombieBase::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if (bitsDamageType == DMG_BULLET)
	{
		if (pevInflictor != nullptr)
		{
			Vector vecDir = pev->origin - (pevInflictor->absmin + pevInflictor->absmax) * 0.5;
			vecDir = vecDir.Normalize();
			float flForce = DamageForce(flDamage);
			pev->velocity = pev->velocity + vecDir * flForce;
		}
		flDamage *= 0.3;
	}

	if (IsAlive())
		PainSound();
	return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

inline void CZombieBase::PainSound()
{
	int pitch = 95 + RANDOM_LONG(0, 9);

	if (RANDOM_LONG(0, 5) < 2)
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(g_pZombiePainSounds), 1.0, ATTN_NORM, 0, pitch);
}

inline void CZombieBase::AlertSound()
{
	int pitch = 95 + RANDOM_LONG(0, 9);

	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(g_pZombieAlertSounds), 1.0, ATTN_NORM, 0, pitch);
}

inline void CZombieBase::IdleSound()
{
	int pitch = 95 + RANDOM_LONG(0, 9);

	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(g_pZombieIdleSounds), 1.0, ATTN_NORM, 0, pitch);
}

inline void CZombieBase::AttackSound()
{
	int pitch = 95 + RANDOM_LONG(0, 9);

	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(g_pZombieAttackSounds), 1.0, ATTN_NORM, 0, pitch);
}

inline int CZombieBase::IgnoreConditions()
{
	int iIgnore = CBaseMonster::IgnoreConditions();

	if ((m_Activity == ACT_MELEE_ATTACK1) || (m_Activity == ACT_MELEE_ATTACK2))
	{
		if (m_flNextFlinch >= gpGlobals->time)
			iIgnore |= (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);
	}

	if ((m_Activity == ACT_SMALL_FLINCH) || (m_Activity == ACT_BIG_FLINCH))
	{
		if (m_flNextFlinch < gpGlobals->time)
			m_flNextFlinch = gpGlobals->time + ZOMBIE_FLINCH_DELAY;
	}

	return iIgnore;
}

inline void CZombieBase::ZombieSpawnHelper()
{
	Precache();

	if (FStringNull(pev->model))
		SET_MODEL(ENT(pev), GetDefaultModel());
	else
		SET_MODEL(ENT(pev), STRING(pev->model));
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->view_ofs = VEC_VIEW;
	m_flFieldOfView = 0.5;
	m_MonsterState = MONSTERSTATE_NONE;
	m_afCapability = bits_CAP_DOORS_GROUP;

	MonsterInit();
}

inline void CZombieBase::ZombiePrecacheHelper()
{
	if (FStringNull(pev->model))
		pev->model = MAKE_STRING(GetDefaultModel());
	PRECACHE_MODEL(STRING(pev->model));

	PRECACHE_SOUND_ARRAY(g_pZombieAttackHitSounds);
	PRECACHE_SOUND_ARRAY(g_pZombieAttackMissSounds);
	PRECACHE_SOUND_ARRAY(g_pZombieAttackSounds);
	PRECACHE_SOUND_ARRAY(g_pZombieIdleSounds);
	PRECACHE_SOUND_ARRAY(g_pZombieAlertSounds);
	PRECACHE_SOUND_ARRAY(g_pZombiePainSounds);
}

inline void CZombieBase::ZombieHandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case ZOMBIE_AE_ATTACK_RIGHT:
	{
		CBaseEntity* pHurt = CheckTraceHullAttack(70, GetOneSlashDamage(), DMG_SLASH);
		if (pHurt)
		{
			if ((pHurt->pev->flags & (FL_MONSTER | FL_CLIENT)) != 0)
			{
				pHurt->pev->punchangle.z = -18;
				pHurt->pev->punchangle.x = 5;
				pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_right * 100;
			}
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(g_pZombieAttackHitSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		else
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(g_pZombieAttackMissSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));

		if (RANDOM_LONG(0, 1))
			AttackSound();
	}
	break;

	case ZOMBIE_AE_ATTACK_LEFT:
	{
		CBaseEntity* pHurt = CheckTraceHullAttack(70, GetOneSlashDamage(), DMG_SLASH);
		if (pHurt)
		{
			if ((pHurt->pev->flags & (FL_MONSTER | FL_CLIENT)) != 0)
			{
				pHurt->pev->punchangle.z = 18;
				pHurt->pev->punchangle.x = 5;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_right * 100;
			}
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(g_pZombieAttackHitSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		else
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(g_pZombieAttackMissSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));

		if (RANDOM_LONG(0, 1))
			AttackSound();
	}
	break;

	case ZOMBIE_AE_ATTACK_BOTH:
	{
		CBaseEntity* pHurt = CheckTraceHullAttack(70, GetBothSlashDamage(), DMG_SLASH);
		if (pHurt)
		{
			if ((pHurt->pev->flags & (FL_MONSTER | FL_CLIENT)) != 0)
			{
				pHurt->pev->punchangle.x = 5;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * -100;
			}
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(g_pZombieAttackHitSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		else
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(g_pZombieAttackMissSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));

		if (RANDOM_LONG(0, 1))
			AttackSound();
	}
	break;

	default:
		CBaseMonster::HandleAnimEvent(pEvent);
		break;
	}
}
