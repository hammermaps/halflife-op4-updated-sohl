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
// CBarneyBase -- non-inline method implementations and
//               save/restore data shared by CBarney/COtis
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "talkmonster.h"
#include "schedule.h"
#include "defaultai.h"
#include "soundent.h"
#include "barney_base.h"

TYPEDESCRIPTION CBarneyBase::m_SaveData[] =
	{
		DEFINE_FIELD(CBarneyBase, m_fGunDrawn, FIELD_BOOLEAN),
		DEFINE_FIELD(CBarneyBase, m_painTime, FIELD_TIME),
		DEFINE_FIELD(CBarneyBase, m_checkAttackTime, FIELD_TIME),
		DEFINE_FIELD(CBarneyBase, m_lastAttackCheck, FIELD_BOOLEAN),
		DEFINE_FIELD(CBarneyBase, m_flPlayerDamage, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CBarneyBase, CTalkMonster);

//=========================================================
// AlertSound — plays the combat/attack sentence
//=========================================================
void CBarneyBase::AlertSound()
{
	if (m_hEnemy != NULL)
	{
		if (FOkToSpeak())
		{
			PlaySentence(GetAlertSentence(), RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE);
		}
	}
}

//=========================================================
// PainSound
//=========================================================
void CBarneyBase::PainSound()
{
	if (gpGlobals->time < m_painTime)
		return;

	m_painTime = gpGlobals->time + RANDOM_FLOAT(0.5, 0.75);

	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(g_pBarneyPainSounds), 1, ATTN_NORM, 0, GetVoicePitch());
}

//=========================================================
// DeathSound
//=========================================================
void CBarneyBase::DeathSound()
{
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(g_pBarneyDeathSounds), 1, ATTN_NORM, 0, GetVoicePitch());
}

//=========================================================
// TakeDamage — react when the player hits us
//=========================================================
bool CBarneyBase::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	// make sure friends talk about it if player hurts talkmonsters...
	bool ret = CTalkMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	if (!IsAlive() || pev->deadflag == DEAD_DYING)
		return ret;

	if (m_MonsterState != MONSTERSTATE_PRONE && (pevAttacker->flags & FL_CLIENT) != 0)
	{
		m_flPlayerDamage += flDamage;

		// LRC - if my reaction to the player has been overridden, don't do this stuff
		if (!m_iPlayerReact)
		{
			// This is a heuristic to determine if the player intended to harm me.
			// If I have an enemy, we can't establish intent (may just be crossfire).
			if (m_hEnemy == NULL)
			{
				// If the player was facing directly at me, or I'm already suspicious, get mad
				if ((m_afMemory & bits_MEMORY_SUSPICIOUS) != 0 || IsFacing(pevAttacker, pev->origin))
				{
					// Alright, now I'm pissed!
					PlaySentence(GetMadSentence(), 4, VOL_NORM, ATTN_NORM);

					Remember(bits_MEMORY_PROVOKED);
					StopFollowing(true);
				}
				else
				{
					// Hey, be careful with that
					PlaySentence(GetShotSentence(), 4, VOL_NORM, ATTN_NORM);
					Remember(bits_MEMORY_SUSPICIOUS);
				}
			}
			else if (!(m_hEnemy->IsPlayer()) && pev->deadflag == DEAD_NO)
			{
				PlaySentence(GetShotSentence(), 4, VOL_NORM, ATTN_NORM);
			}
		}
	}

	return ret;
}

//=========================================================
// CheckRangeAttack1
//=========================================================
bool CBarneyBase::CheckRangeAttack1(float flDot, float flDist)
{
	if (flDist <= 1024 && flDot >= 0.5)
	{
		if (gpGlobals->time > m_checkAttackTime)
		{
			TraceResult tr;

			Vector shootOrigin = pev->origin + Vector(0, 0, 55);
			CBaseEntity* pEnemy = m_hEnemy;
			Vector shootTarget = ((pEnemy->BodyTarget(shootOrigin) - pEnemy->pev->origin) + m_vecEnemyLKP);
			UTIL_TraceLine(shootOrigin, shootTarget, dont_ignore_monsters, ENT(pev), &tr);
			m_checkAttackTime = gpGlobals->time + 1;
			if (tr.flFraction == 1.0 || (tr.pHit != NULL && CBaseEntity::Instance(tr.pHit) == pEnemy))
				m_lastAttackCheck = true;
			else
				m_lastAttackCheck = false;
			m_checkAttackTime = gpGlobals->time + 1.5;
		}
		return m_lastAttackCheck;
	}
	return false;
}

//=========================================================
// TraceAttack — armour logic (helmet hitgroup 10, vest)
//=========================================================
void CBarneyBase::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	switch (ptr->iHitgroup)
	{
	case HITGROUP_CHEST:
	case HITGROUP_STOMACH:
		if ((bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_BLAST)) != 0)
		{
			flDamage = flDamage / 2;
		}
		break;
	case 10:
		if ((bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_CLUB)) != 0)
		{
			flDamage -= 20;
			if (flDamage <= 0)
			{
				UTIL_Ricochet(ptr->vecEndPos, 1.0);
				flDamage = 0.01;
			}
		}
		// always a head shot
		ptr->iHitgroup = HITGROUP_HEAD;
		break;
	}

	CTalkMonster::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}

//=========================================================
// DeclineFollowing
//=========================================================
void CBarneyBase::DeclineFollowing()
{
	if (!FStringNull(m_iszDecline))
		PlaySentence(STRING(m_iszDecline), 2, VOL_NORM, ATTN_NORM); // LRC
	else
		PlaySentence(GetDeclineSentence(), 2, VOL_NORM, ATTN_NORM);
}

//=========================================================
// GetScheduleOfType
//=========================================================
Schedule_t* CBarneyBase::GetScheduleOfType(int Type)
{
	Schedule_t* psched;

	switch (Type)
	{
	case SCHED_ARM_WEAPON:
		if (m_hEnemy != NULL)
		{
			// face enemy, then draw.
			return slBarneyBaseEnemyDraw;
		}
		break;

	// Hook these to make a looping schedule
	case SCHED_TARGET_FACE:
		// call base class default so that barney/otis will talk when 'used'
		psched = CTalkMonster::GetScheduleOfType(Type);

		if (psched == slIdleStand)
			return slBarneyBaseFaceTarget;
		else
			return psched;

	case SCHED_TARGET_CHASE:
		return slBarneyBaseFollow;

	case SCHED_IDLE_STAND:
		// call base class default so that barney/otis will talk when standing during idle
		psched = CTalkMonster::GetScheduleOfType(Type);

		if (psched == slIdleStand)
			return slBarneyBaseIdleStand;
		else
			return psched;
	}

	return CTalkMonster::GetScheduleOfType(Type);
}

//=========================================================
// GetSchedule
//=========================================================
Schedule_t* CBarneyBase::GetSchedule()
{
	if (HasConditions(bits_COND_HEAR_SOUND))
	{
		CSound* pSound;
		pSound = PBestSound();

		ASSERT(pSound != NULL);
		if (pSound && (pSound->m_iType & bits_SOUND_DANGER) != 0)
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_BEST_SOUND);
	}
	if (HasConditions(bits_COND_ENEMY_DEAD) && FOkToSpeak())
	{
		PlaySentence(GetKillSentence(), 4, VOL_NORM, ATTN_NORM);
	}

	switch (m_MonsterState)
	{
	case MONSTERSTATE_COMBAT:
	{
		// dead enemy
		if (HasConditions(bits_COND_ENEMY_DEAD))
		{
			// call base class, all code to handle dead enemies is centralized there.
			return CBaseMonster::GetSchedule();
		}

		// always act surprized with a new enemy
		if (HasConditions(bits_COND_NEW_ENEMY) && HasConditions(bits_COND_LIGHT_DAMAGE))
			return GetScheduleOfType(SCHED_SMALL_FLINCH);

		// wait for one schedule to draw gun
		if (!m_fGunDrawn)
			return GetScheduleOfType(SCHED_ARM_WEAPON);

		if (HasConditions(bits_COND_HEAVY_DAMAGE))
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
	}
	break;

	case MONSTERSTATE_ALERT:
	case MONSTERSTATE_IDLE:
		if (HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
		{
			// flinch if hurt
			return GetScheduleOfType(SCHED_SMALL_FLINCH);
		}

		if (m_hEnemy == NULL && IsFollowing())
		{
			if (!m_hTargetEnt->IsAlive())
			{
				// UNDONE: Comment about the recently dead player here?
				StopFollowing(false);
				break;
			}
			else
			{
				if (HasConditions(bits_COND_CLIENT_PUSH))
				{
					return GetScheduleOfType(SCHED_MOVE_AWAY_FOLLOW);
				}
				return GetScheduleOfType(SCHED_TARGET_FACE);
			}
		}

		if (HasConditions(bits_COND_CLIENT_PUSH))
		{
			return GetScheduleOfType(SCHED_MOVE_AWAY);
		}

		// try to say something about smells
		TrySmellTalk();
		break;
	}

	return CTalkMonster::GetSchedule();
}
