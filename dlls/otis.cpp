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
// first flag is otis dying for scripted sequences?
#define OTIS_AE_DRAW    BARNEYBASE_AE_DRAW
#define OTIS_AE_SHOOT   BARNEYBASE_AE_SHOOT
#define OTIS_AE_HOLSTER BARNEYBASE_AE_HOLSTER

#define OTIS_BODY_GUNHOLSTERED 0
#define OTIS_BODY_GUNDRAWN 1
#define OTIS_BODY_GUNGONE 2

namespace OtisBodyGroup
{
enum OtisBodyGroup
{
	Weapons = 1,
	Heads = 2
};
}

namespace OtisWeapon
{
enum OtisWeapon
{
	Random = -1,
	None = 0,
	DesertEagle,
	Donut
};
}

namespace OtisHead
{
enum OtisHead
{
	Random = -1,
	Hair = 0,
	Balding
};
}

class COtis : public CBarneyBase
{
public:
	void Spawn() override;
	void Precache() override;
	void OtisFirePistol();
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;

	void TalkInit();

	void Killed(entvars_t* pevAttacker, int iGib) override;

	bool KeyValue(KeyValueData* pkvd) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	//These were originally used to store off the setting AND track state,
	//but state is now tracked by calling GetBodygroup
	int m_iOtisBody;
	int m_iOtisHead;

	CUSTOM_SCHEDULES;

protected:
	const char* GetDefaultModel() override { return "models/otis.mdl"; }
	const char* GetAlertSentence() override { return "OT_ATTACK"; }
	const char* GetKillSentence() override { return "OT_KILL"; }
	const char* GetDeclineSentence() override { return "OT_POK"; }
	const char* GetMadSentence() override { return "OT_MAD"; }
	const char* GetShotSentence() override { return "OT_SHOT"; }
};

LINK_ENTITY_TO_CLASS(monster_otis, COtis);

TYPEDESCRIPTION COtis::m_SaveData[] =
	{
		DEFINE_FIELD(COtis, m_iOtisBody, FIELD_INTEGER),
		DEFINE_FIELD(COtis, m_iOtisHead, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(COtis, CBarneyBase);

//=========================================================
// AI Schedules Specific to this monster
// (schedule data is shared with CBarney via slBarneyBase* tables in barney_base.h)
//=========================================================
DEFINE_CUSTOM_SCHEDULES(COtis){
	slBarneyBaseFollow,
	slBarneyBaseEnemyDraw,
	slBarneyBaseFaceTarget,
	slBarneyBaseIdleStand,
};

IMPLEMENT_CUSTOM_SCHEDULES(COtis, CTalkMonster);

//=========================================================
// OtisFirePistol - shoots one round from the pistol at
// the enemy otis is facing.
//=========================================================
void COtis::OtisFirePistol()
{
	Vector vecShootOrigin;

	UTIL_MakeVectors(pev->angles);
	vecShootOrigin = pev->origin + Vector(0, 0, 55);
	Vector vecShootDir = ShootAtEnemy(vecShootOrigin);

	Vector angDir = UTIL_VecToAngles(vecShootDir);
	SetBlending(0, angDir.x);
	pev->effects = EF_MUZZLEFLASH;

	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_PLAYER_357);

	int pitchShift = RANDOM_LONG(0, 20);

	// Only shift about half the time
	if (pitchShift > 10)
		pitchShift = 0;
	else
		pitchShift -= 5;
	EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/de_shot1.wav", 1, ATTN_NORM, 0, 100 + pitchShift);

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
void COtis::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case OTIS_AE_SHOOT:
		OtisFirePistol();
		break;

	case OTIS_AE_DRAW:
		// otis's bodygroup switches here so he can pull gun from holster
		if (GetBodygroup(OtisBodyGroup::Weapons) == OtisWeapon::None)
		{
			SetBodygroup(OtisBodyGroup::Weapons, OtisWeapon::DesertEagle);
			m_iOtisBody = OtisWeapon::DesertEagle;
			m_fGunDrawn = true;
		}

		break;

	case OTIS_AE_HOLSTER:
		// change bodygroup to replace gun in holster
		if (GetBodygroup(OtisBodyGroup::Weapons) == OtisWeapon::DesertEagle)
		{
			SetBodygroup(OtisBodyGroup::Weapons, OtisWeapon::None);
			m_iOtisBody = OtisWeapon::None;
			m_fGunDrawn = false;
		}
		break;

	default:
		CTalkMonster::HandleAnimEvent(pEvent);
	}
}

//=========================================================
// Spawn
//=========================================================
void COtis::Spawn()
{
	Precache();

	if (FStringNull(pev->model))
		SET_MODEL(ENT(pev), "models/otis.mdl");
	else
		SET_MODEL(ENT(pev), STRING(pev->model));
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_RED;
	pev->health = gSkillData.otisHealth;
	pev->view_ofs = Vector(0, 0, 50);  // position of the eyes relative to monster's origin.
	m_flFieldOfView = VIEW_FIELD_WIDE; // NOTE: we need a wide field of view so npc will notice player and say hello
	m_MonsterState = MONSTERSTATE_NONE;

	m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	//Note: This originally didn't use SetBodygroup
	if (m_iOtisHead == OtisHead::Random)
	{
		m_iOtisHead = RANDOM_LONG(0, 1);
	}

	if (m_iOtisBody == OtisWeapon::Random)
	{
		m_iOtisBody = OtisWeapon::None;
	}

	SetBodygroup(OtisBodyGroup::Weapons, m_iOtisBody);
	SetBodygroup(OtisBodyGroup::Heads, m_iOtisHead);

	m_fGunDrawn = m_iOtisBody == OtisWeapon::DesertEagle;

	MonsterInit();
	SetUse(&COtis::FollowerUse);
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void COtis::Precache()
{
	if (FStringNull(pev->model))
		pev->model = MAKE_STRING("models/otis.mdl");
	PRECACHE_MODEL(STRING(pev->model));

	PRECACHE_SOUND("barney/ba_attack1.wav");
	PRECACHE_SOUND("barney/ba_attack2.wav");

	PRECACHE_SOUND("weapons/de_shot1.wav");

	PRECACHE_SOUND_ARRAY(g_pBarneyPainSounds);
	PRECACHE_SOUND_ARRAY(g_pBarneyDeathSounds);

	// every new otis must call this, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)
	TalkInit();
	CTalkMonster::Precache();
}

// Init talk data
void COtis::TalkInit()
{

	CTalkMonster::TalkInit();

	// scientists speach group names (group names are in sentences.txt)

	m_szGrp[TLK_ANSWER] = "OT_ANSWER";
	m_szGrp[TLK_QUESTION] = "OT_QUESTION";
	m_szGrp[TLK_IDLE] = "OT_IDLE";
	m_szGrp[TLK_STARE] = "OT_STARE";
	m_szGrp[TLK_USE] = "OT_OK";
	m_szGrp[TLK_UNUSE] = "OT_WAIT";
	m_szGrp[TLK_STOP] = "OT_STOP";

	m_szGrp[TLK_NOSHOOT] = "OT_SCARED";
	m_szGrp[TLK_HELLO] = "OT_HELLO";

	m_szGrp[TLK_PLHURT1] = "!OT_CUREA";
	m_szGrp[TLK_PLHURT2] = "!OT_CUREB";
	m_szGrp[TLK_PLHURT3] = "!OT_CUREC";

	m_szGrp[TLK_PHELLO] = NULL;			  //"OT_PHELLO";		// UNDONE
	m_szGrp[TLK_PIDLE] = NULL;			  //"OT_PIDLE";			// UNDONE
	m_szGrp[TLK_PQUESTION] = "OT_PQUEST"; // UNDONE

	m_szGrp[TLK_SMELL] = "OT_SMELL";

	m_szGrp[TLK_WOUND] = "OT_WOUND";
	m_szGrp[TLK_MORTAL] = "OT_MORTAL";

	// get voice for head - just one otis voice for now
	m_voicePitch = 100;
}

void COtis::Killed(entvars_t* pevAttacker, int iGib)
{
	if (pev->spawnflags & SF_MONSTER_NO_WPN_DROP)
	{
		SetUse(NULL);
		CTalkMonster::Killed(pevAttacker, iGib);
		return;
	}

	if (GetBodygroup(OtisBodyGroup::Weapons) == OtisWeapon::DesertEagle)
	{ // drop the gun!
		Vector vecGunPos;
		Vector vecGunAngles;

		SetBodygroup(OtisBodyGroup::Weapons, OtisWeapon::None);
		m_iOtisBody = OtisWeapon::None;

		GetAttachment(0, vecGunPos, vecGunAngles);

		CBaseEntity* pGun = DropItem("weapon_eagle", vecGunPos, vecGunAngles);
	}

	SetUse(NULL);
	CTalkMonster::Killed(pevAttacker, iGib);
}

bool COtis::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq("head", pkvd->szKeyName))
	{
		m_iOtisHead = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq("bodystate", pkvd->szKeyName))
	{
		m_iOtisBody = atoi(pkvd->szValue);
		return true;
	}

	return CBaseMonster::KeyValue(pkvd);
}



//=========================================================
// DEAD OTIS PROP
//
// Designer selects a pose in worldcraft, 0 through num_poses-1
// this value is added to what is selected as the 'first dead pose'
// among the monster's normal animations. All dead poses must
// appear sequentially in the model file. Be sure and set
// the m_iFirstPose properly!
//
//=========================================================
class CDeadOtis : public CBaseMonster
{
public:
	void Spawn() override;
	int Classify() override { return CLASS_PLAYER_ALLY; }

	bool KeyValue(KeyValueData* pkvd) override;

	int m_iPose; // which sequence to display	-- temporary, don't need to save
	static char* m_szPoses[5];
};

char* CDeadOtis::m_szPoses[] = {"lying_on_back", "lying_on_side", "lying_on_stomach", "stuffed_in_vent", "dead_sitting"};

bool CDeadOtis::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		return true;
	}

	return CBaseMonster::KeyValue(pkvd);
}

LINK_ENTITY_TO_CLASS(monster_otis_dead, CDeadOtis);

//=========================================================
// ********** DeadOtis SPAWN **********
//=========================================================
void CDeadOtis::Spawn()
{
	if (FStringNull(pev->model))
		pev->model = MAKE_STRING("models/otis.mdl");
	PRECACHE_MODEL(STRING(pev->model));
	SET_MODEL(ENT(pev), STRING(pev->model));

	pev->effects = 0;
	pev->yaw_speed = 8;
	pev->sequence = 0;
	m_bloodColor = BLOOD_COLOR_RED;

	pev->sequence = LookupSequence(m_szPoses[m_iPose]);
	if (pev->sequence == -1)
	{
		ALERT(at_console, "Dead otis with bad pose\n");
	}
	// Corpses have less health
	pev->health = 8; //gSkillData.otisHealth;

	MonsterInitDead();
}
