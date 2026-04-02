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
// monster template
//=========================================================
// UNDONE: Holster weapon?

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "defaultai.h"
#include "scripted.h"
#include "weapons.h"
#include "soundent.h"
#include "barney_base.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
// first flag is barney dying for scripted sequences?
#define BARNEY_AE_DRAW    BARNEYBASE_AE_DRAW
#define BARNEY_AE_SHOOT   BARNEYBASE_AE_SHOOT
#define BARNEY_AE_HOLSTER BARNEYBASE_AE_HOLSTER

#define BARNEY_BODY_GUNHOLSTERED 0
#define BARNEY_BODY_GUNDRAWN 1
#define BARNEY_BODY_GUNGONE 2

class CBarney : public CBarneyBase
{
public:
	void Spawn() override;
	void Precache() override;
	void BarneyFirePistol();
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;

	void TalkInit();

	void Killed(entvars_t* pevAttacker, int iGib) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	int m_iBaseBody; // LRC - for barneys with different bodies

	CUSTOM_SCHEDULES;

protected:
	const char* GetDefaultModel() override { return "models/barney.mdl"; }
	const char* GetAlertSentence() override { return "BA_ATTACK"; }
	const char* GetKillSentence() override { return "BA_KILL"; }
	const char* GetDeclineSentence() override { return "BA_POK"; }
	const char* GetMadSentence() override { return "BA_MAD"; }
	const char* GetShotSentence() override { return "BA_SHOT"; }
};

LINK_ENTITY_TO_CLASS(monster_barney, CBarney);

TYPEDESCRIPTION CBarney::m_SaveData[] =
	{
		DEFINE_FIELD(CBarney, m_iBaseBody, FIELD_INTEGER), // LRC
};

IMPLEMENT_SAVERESTORE(CBarney, CBarneyBase);

//=========================================================
// AI Schedules Specific to this monster
// (schedule data is shared with COtis via slBarneyBase* tables in barney_base.h)
//=========================================================
DEFINE_CUSTOM_SCHEDULES(CBarney){
	slBarneyBaseFollow,
	slBarneyBaseEnemyDraw,
	slBarneyBaseFaceTarget,
	slBarneyBaseIdleStand,
};

IMPLEMENT_CUSTOM_SCHEDULES(CBarney, CTalkMonster);


//=========================================================
// BarneyFirePistol - shoots one round from the pistol at
// the enemy barney is facing.
//=========================================================
void CBarney::BarneyFirePistol()
{
	Vector vecShootOrigin;

	UTIL_MakeVectors(pev->angles);
	vecShootOrigin = pev->origin + Vector(0, 0, 55);
	Vector vecShootDir = ShootAtEnemy(vecShootOrigin);

	Vector angDir = UTIL_VecToAngles(vecShootDir);
	SetBlending(0, angDir.x);
	pev->effects = EF_MUZZLEFLASH;

	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_MONSTER_9MM);

	int pitchShift = RANDOM_LONG(0, 20);

	// Only shift about half the time
	if (pitchShift > 10)
		pitchShift = 0;
	else
		pitchShift -= 5;
	EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "barney/ba_attack2.wav", 1, ATTN_NORM, 0, 100 + pitchShift);

	CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, 384, 0.3);

	// UNDONE: Reload?
	m_cAmmoLoaded--; // take away a bullet!
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CBarney::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case BARNEY_AE_SHOOT:
		BarneyFirePistol();
		break;

	case BARNEY_AE_DRAW:
		// barney's bodygroup switches here so he can pull gun from holster
		pev->body = BARNEY_BODY_GUNDRAWN;
		m_fGunDrawn = true;
		break;

	case BARNEY_AE_HOLSTER:
		// change bodygroup to replace gun in holster
		pev->body = BARNEY_BODY_GUNHOLSTERED;
		m_fGunDrawn = false;
		break;

	default:
		CTalkMonster::HandleAnimEvent(pEvent);
	}
}

//=========================================================
// Spawn
//=========================================================
void CBarney::Spawn()
{
	Precache();

	if (FStringNull(pev->model))
		SET_MODEL(ENT(pev), "models/barney.mdl");
	else
		SET_MODEL(ENT(pev), STRING(pev->model));
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_RED;
	if (pev->health == 0)
		pev->health = gSkillData.barneyHealth;
	pev->view_ofs = Vector(0, 0, 50);  // position of the eyes relative to monster's origin.
	m_flFieldOfView = VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState = MONSTERSTATE_NONE;

	pev->body = 0; // gun in holster
	m_iBaseBody = pev->body; // LRC
	m_fGunDrawn = false;

	m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	MonsterInit();
	SetUse(&CBarney::FollowerUse);
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CBarney::Precache()
{
	if (FStringNull(pev->model))
		pev->model = MAKE_STRING("models/barney.mdl");
	PRECACHE_MODEL(STRING(pev->model));

	PRECACHE_SOUND("barney/ba_attack1.wav");
	PRECACHE_SOUND("barney/ba_attack2.wav");

	PRECACHE_SOUND_ARRAY(g_pBarneyPainSounds);
	PRECACHE_SOUND_ARRAY(g_pBarneyDeathSounds);

	// every new barney must call this, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)
	TalkInit();
	CTalkMonster::Precache();
}

// Init talk data
void CBarney::TalkInit()
{

	CTalkMonster::TalkInit();

	// scientists speach group names (group names are in sentences.txt)

	m_szGrp[TLK_ANSWER] = "BA_ANSWER";
	m_szGrp[TLK_QUESTION] = "BA_QUESTION";
	m_szGrp[TLK_IDLE] = "BA_IDLE";
	m_szGrp[TLK_STARE] = "BA_STARE";
	m_szGrp[TLK_USE] = "BA_OK";
	m_szGrp[TLK_UNUSE] = "BA_WAIT";
	m_szGrp[TLK_STOP] = "BA_STOP";

	m_szGrp[TLK_NOSHOOT] = "BA_SCARED";
	m_szGrp[TLK_HELLO] = "BA_HELLO";

	m_szGrp[TLK_PLHURT1] = "!BA_CUREA";
	m_szGrp[TLK_PLHURT2] = "!BA_CUREB";
	m_szGrp[TLK_PLHURT3] = "!BA_CUREC";

	m_szGrp[TLK_PHELLO] = NULL;			  //"BA_PHELLO";		// UNDONE
	m_szGrp[TLK_PIDLE] = NULL;			  //"BA_PIDLE";			// UNDONE
	m_szGrp[TLK_PQUESTION] = "BA_PQUEST"; // UNDONE

	m_szGrp[TLK_SMELL] = "BA_SMELL";

	m_szGrp[TLK_WOUND] = "BA_WOUND";
	m_szGrp[TLK_MORTAL] = "BA_MORTAL";

	// get voice for head - just one barney voice for now
	m_voicePitch = 100;
}

void CBarney::Killed(entvars_t* pevAttacker, int iGib)
{
	if (pev->body < BARNEY_BODY_GUNGONE)
	{ // drop the gun!
		Vector vecGunPos;
		Vector vecGunAngles;

		pev->body = BARNEY_BODY_GUNGONE;

		GetAttachment(0, vecGunPos, vecGunAngles);

		CBaseEntity* pGun = DropItem("weapon_9mmhandgun", vecGunPos, vecGunAngles);
	}

	SetUse(NULL);
	CTalkMonster::Killed(pevAttacker, iGib);
}


//=========================================================
// DEAD BARNEY PROP
//
// Designer selects a pose in worldcraft, 0 through num_poses-1
// this value is added to what is selected as the 'first dead pose'
// among the monster's normal animations. All dead poses must
// appear sequentially in the model file. Be sure and set
// the m_iFirstPose properly!
//
//=========================================================
class CDeadBarney : public CBaseMonster
{
public:
	void Spawn() override;
	int Classify() override { return CLASS_PLAYER_ALLY; }

	bool KeyValue(KeyValueData* pkvd) override;

	int m_iPose; // which sequence to display	-- temporary, don't need to save
	static const char* m_szPoses[3];
};

const char* CDeadBarney::m_szPoses[] = {"lying_on_back", "lying_on_side", "lying_on_stomach"};

bool CDeadBarney::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		return true;
	}

	return CBaseMonster::KeyValue(pkvd);
}

LINK_ENTITY_TO_CLASS(monster_barney_dead, CDeadBarney);

//=========================================================
// ********** DeadBarney SPAWN **********
//=========================================================
void CDeadBarney::Spawn()
{
	PRECACHE_MODEL("models/barney.mdl");
	SET_MODEL(ENT(pev), "models/barney.mdl");

	pev->effects = 0;
	pev->yaw_speed = 8;
	pev->sequence = 0;
	m_bloodColor = BLOOD_COLOR_RED;

	pev->sequence = LookupSequence(m_szPoses[m_iPose]);
	if (pev->sequence == -1)
	{
		ALERT(at_console, "Dead barney with bad pose\n");
	}
	// Corpses have less health
	pev->health = 8; //gSkillData.barneyHealth;

	MonsterInitDead();
}
