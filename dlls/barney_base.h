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
// CBarneyBase -- shared base class for CBarney (monster_barney)
//               and COtis (monster_otis)
//=========================================================

#pragma once

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "talkmonster.h"
#include "schedule.h"
#include "defaultai.h"
#include "soundent.h"

//=========================================================
// Shared anim event values — same numeric constants in
// both the Barney and Otis model files
//=========================================================
#define BARNEYBASE_AE_DRAW    (2)
#define BARNEYBASE_AE_SHOOT   (3)
#define BARNEYBASE_AE_HOLSTER (4)

//=========================================================
// Shared pain/death sounds — Barney and Otis both use these
//=========================================================
inline const char* g_pBarneyPainSounds[] =
{
	"barney/ba_pain1.wav",
	"barney/ba_pain2.wav",
	"barney/ba_pain3.wav",
};

inline const char* g_pBarneyDeathSounds[] =
{
	"barney/ba_die1.wav",
	"barney/ba_die2.wav",
	"barney/ba_die3.wav",
};

//=========================================================
// Shared schedule tables — data is identical between
// Barney and Otis; each subclass still registers its own
// DEFINE_CUSTOM_SCHEDULES list that points to these tables
//=========================================================
inline Task_t tlBarneyBaseFollow[] =
{
	{TASK_MOVE_TO_TARGET_RANGE, (float)128}, // Move within 128 of target ent (client)
	{TASK_SET_SCHEDULE, (float)SCHED_TARGET_FACE},
};

inline Schedule_t slBarneyBaseFollow[] =
{
	{tlBarneyBaseFollow,
		ARRAYSIZE(tlBarneyBaseFollow),
		bits_COND_NEW_ENEMY |
			bits_COND_LIGHT_DAMAGE |
			bits_COND_HEAVY_DAMAGE |
			bits_COND_HEAR_SOUND |
			bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"Follow"},
};

inline Task_t tlBarneyBaseEnemyDraw[] =
{
	{TASK_STOP_MOVING, 0},
	{TASK_FACE_ENEMY, 0},
	{TASK_PLAY_SEQUENCE_FACE_ENEMY, (float)ACT_ARM},
};

inline Schedule_t slBarneyBaseEnemyDraw[] =
{
	{tlBarneyBaseEnemyDraw,
		ARRAYSIZE(tlBarneyBaseEnemyDraw),
		0,
		0,
		"Enemy Draw"}};

inline Task_t tlBarneyBaseFaceTarget[] =
{
	{TASK_SET_ACTIVITY, (float)ACT_IDLE},
	{TASK_FACE_TARGET, (float)0},
	{TASK_SET_ACTIVITY, (float)ACT_IDLE},
	{TASK_SET_SCHEDULE, (float)SCHED_TARGET_CHASE},
};

inline Schedule_t slBarneyBaseFaceTarget[] =
{
	{tlBarneyBaseFaceTarget,
		ARRAYSIZE(tlBarneyBaseFaceTarget),
		bits_COND_CLIENT_PUSH |
			bits_COND_NEW_ENEMY |
			bits_COND_LIGHT_DAMAGE |
			bits_COND_HEAVY_DAMAGE |
			bits_COND_HEAR_SOUND |
			bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"FaceTarget"},
};

inline Task_t tlBarneyBaseIdleStand[] =
{
	{TASK_STOP_MOVING, 0},
	{TASK_SET_ACTIVITY, (float)ACT_IDLE},
	{TASK_WAIT, (float)2},			// repick IDLESTAND every two seconds.
	{TASK_TLK_HEADRESET, (float)0}, // reset head position
};

inline Schedule_t slBarneyBaseIdleStand[] =
{
	{tlBarneyBaseIdleStand,
		ARRAYSIZE(tlBarneyBaseIdleStand),
		bits_COND_NEW_ENEMY |
			bits_COND_LIGHT_DAMAGE |
			bits_COND_HEAVY_DAMAGE |
			bits_COND_HEAR_SOUND |
			bits_COND_SMELL |
			bits_COND_PROVOKED,

		bits_SOUND_COMBAT | // sound flags - change these, and you'll break the talking code.
			//bits_SOUND_PLAYER		|
			//bits_SOUND_WORLD		|

			bits_SOUND_DANGER |
			bits_SOUND_MEAT | // scents
			bits_SOUND_CARCASS |
			bits_SOUND_GARBAGE,
		"IdleStand"},
};

//=========================================================
// CBarneyBase — abstract base class shared by CBarney and
// COtis.  Provides all behaviour that is identical between
// the two security-guard variants so that only the
// character-specific parts need to live in the subclass.
//=========================================================
class CBarneyBase : public CTalkMonster
{
public:
	// ---- overrides shared between CBarney and COtis ----
	void SetYawSpeed() override;
	int ISoundMask() override;
	int Classify() override;
	int ObjectCaps() override { return CTalkMonster::ObjectCaps() | FCAP_IMPULSE_USE; }
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	bool CheckRangeAttack1(float flDot, float flDist) override;
	void DeclineFollowing() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	Schedule_t* GetSchedule() override;
	MONSTERSTATE GetIdealState() override;
	void DeathSound() override;
	void PainSound() override;
	void AlertSound() override;
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	// ---- shared state fields ----
	bool m_fGunDrawn;
	float m_painTime;
	float m_checkAttackTime;
	bool m_lastAttackCheck;
	float m_flPlayerDamage; // how much pain has the player inflicted on me?

protected:
	// ---- pure-virtual hooks: each subclass supplies these ----

	// Default model path, e.g. "models/barney.mdl"
	virtual const char* GetDefaultModel() = 0;

	// Sentence group played when spotting an enemy, e.g. "BA_ATTACK"
	virtual const char* GetAlertSentence() = 0;

	// Sentence played when killing an enemy, e.g. "BA_KILL"
	virtual const char* GetKillSentence() = 0;

	// Sentence played when declining to follow, e.g. "BA_POK"
	virtual const char* GetDeclineSentence() = 0;

	// Sentence played when provoked by the player, e.g. "BA_MAD"
	virtual const char* GetMadSentence() = 0;

	// Sentence played when the player shoots near the NPC, e.g. "BA_SHOT"
	virtual const char* GetShotSentence() = 0;
};

//=========================================================
// Inline trivial implementations
//=========================================================

inline void CBarneyBase::SetYawSpeed()
{
	switch (m_Activity)
	{
	case ACT_RUN:
		pev->yaw_speed = 90;
		break;
	default:
		pev->yaw_speed = 70;
		break;
	}
}

inline int CBarneyBase::ISoundMask()
{
	return bits_SOUND_WORLD |
		   bits_SOUND_COMBAT |
		   bits_SOUND_CARCASS |
		   bits_SOUND_MEAT |
		   bits_SOUND_GARBAGE |
		   bits_SOUND_DANGER |
		   bits_SOUND_PLAYER;
}

inline int CBarneyBase::Classify()
{
	return m_iClass ? m_iClass : CLASS_PLAYER_ALLY;
}

inline void CBarneyBase::StartTask(Task_t* pTask)
{
	CTalkMonster::StartTask(pTask);
}

inline void CBarneyBase::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
		if (m_hEnemy != NULL && (m_hEnemy->IsPlayer()))
		{
			pev->framerate = 1.5;
		}
		CTalkMonster::RunTask(pTask);
		break;
	default:
		CTalkMonster::RunTask(pTask);
		break;
	}
}

inline MONSTERSTATE CBarneyBase::GetIdealState()
{
	return CTalkMonster::GetIdealState();
}
