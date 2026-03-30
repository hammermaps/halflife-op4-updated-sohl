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
/*

===== bmodels.cpp ========================================================

  spawn, think, and use functions for entities that use brush models

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "doors.h"
#include "UserMessages.h"
#include "movewith.h" // LRC - UTIL_SetAvelocity / UTIL_SetAngles

#define SF_BRUSH_ACCDCC 16		 // brush should accelerate and decelerate when toggled
#define SF_BRUSH_HURT 32		 // rotating brush that inflicts pain based on rotation speed
#define SF_ROTATING_NOT_SOLID 64 // some special rotating objects are not solid.

// covering cheesy noise1, noise2, & noise3 fields so they make more sense (for rotating fans)
#define noiseStart noise1
#define noiseStop noise2
#define noiseRunning noise3

#define SF_PENDULUM_SWING 2 // spawnflag that makes a pendulum a rope swing.
//
// BModelOrigin - calculates origin of a bmodel from absmin/size because all bmodel origins are 0 0 0
//
Vector VecBModelOrigin(entvars_t* pevBModel)
{
	return (pevBModel->absmin + pevBModel->absmax) * 0.5;  // LRC - bug fix
}

// =================== FUNC_WALL ==============================================

/*QUAKED func_wall (0 .5 .8) ?
This is just a solid wall if not inhibited
*/
class CFuncWall : public CBaseEntity
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	// Bmodels don't go across transitions
	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

LINK_ENTITY_TO_CLASS(func_wall, CFuncWall);

bool CFuncWall::KeyValue(KeyValueData* pkvd)
{
	// SoHL 1.5 - Parse "message" key as rotation vector
	if (FStrEq(pkvd->szKeyName, "message"))
	{
		UTIL_StringToVector((float*)pev->angles, pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void CFuncWall::Spawn()
{
	pev->movetype = MOVETYPE_PUSH; // so it doesn't get pushed by anything
	pev->solid = SOLID_BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));

	// If it can't move/go away, it's really part of the world
	pev->flags |= FL_WORLDBRUSH;
}


void CFuncWall::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (ShouldToggle(useType, pev->frame != 0))
		pev->frame = 1 - pev->frame;
}


#define SF_WALL_START_OFF 0x0001

class CFuncWallToggle : public CFuncWall
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void TurnOff();
	void TurnOn();
	bool IsOn();
};

LINK_ENTITY_TO_CLASS(func_wall_toggle, CFuncWallToggle);

void CFuncWallToggle::Spawn()
{
	CFuncWall::Spawn();
	if ((pev->spawnflags & SF_WALL_START_OFF) != 0)
		TurnOff();
}


void CFuncWallToggle::TurnOff()
{
	pev->solid = SOLID_NOT;
	pev->effects |= EF_NODRAW;
	UTIL_SetOrigin(pev, pev->origin);
}


void CFuncWallToggle::TurnOn()
{
	pev->solid = SOLID_BSP;
	pev->effects &= ~EF_NODRAW;
	UTIL_SetOrigin(pev, pev->origin);
}


bool CFuncWallToggle::IsOn()
{
	if (pev->solid == SOLID_NOT)
		return false;
	return true;
}


void CFuncWallToggle::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	bool status = IsOn();

	if (ShouldToggle(useType, status))
	{
		if (status)
			TurnOff();
		else
			TurnOn();
	}
}


#define SF_CONVEYOR_VISUAL 0x0001
#define SF_CONVEYOR_NOTSOLID 0x0002

class CFuncConveyor : public CFuncWall
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void UpdateSpeed(float speed);
};

LINK_ENTITY_TO_CLASS(func_conveyor, CFuncConveyor);
void CFuncConveyor::Spawn()
{
	SetMovedir(pev);
	CFuncWall::Spawn();

	if ((pev->spawnflags & SF_CONVEYOR_VISUAL) == 0)
		SetBits(pev->flags, FL_CONVEYOR);

	// HACKHACK - This is to allow for some special effects
	if ((pev->spawnflags & SF_CONVEYOR_NOTSOLID) != 0)
	{
		pev->solid = SOLID_NOT;
		pev->skin = 0; // Don't want the engine thinking we've got special contents on this brush
	}

	if (pev->speed == 0)
		pev->speed = 100;

	UpdateSpeed(pev->speed);
}


// HACKHACK -- This is ugly, but encode the speed in the rendercolor to avoid adding more data to the network stream
void CFuncConveyor::UpdateSpeed(float speed)
{
	// Encode it as an integer with 4 fractional bits
	int speedCode = (int)(fabs(speed) * 16.0);

	if (speed < 0)
		pev->rendercolor.x = 1;
	else
		pev->rendercolor.x = 0;

	pev->rendercolor.y = (speedCode >> 8);
	pev->rendercolor.z = (speedCode & 0xFF);
}


void CFuncConveyor::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// LRC - ShouldToggle: respect USE_ON / USE_OFF / USE_TOGGLE
	if (!ShouldToggle(useType, pev->speed > 0))
		return;

	pev->speed = -pev->speed;
	UpdateSpeed(pev->speed);
}



// =================== FUNC_ILLUSIONARY ==============================================


/*QUAKED func_illusionary (0 .5 .8) ?
A simple entity that looks solid but lets you walk through it.
*/
class CFuncIllusionary : public CBaseToggle
{
public:
	void Spawn() override;
	void EXPORT SloshTouch(CBaseEntity* pOther);
	bool KeyValue(KeyValueData* pkvd) override;
	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

LINK_ENTITY_TO_CLASS(func_illusionary, CFuncIllusionary);

bool CFuncIllusionary::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "skin")) //skin is used for content type
	{
		pev->skin = atof(pkvd->szValue);
		return true;
	}

	return CBaseToggle::KeyValue(pkvd);
}

void CFuncIllusionary::Spawn()
{
	pev->angles = g_vecZero;
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT; // always solid_not
	SET_MODEL(ENT(pev), STRING(pev->model));

	// I'd rather eat the network bandwidth of this than figure out how to save/restore
	// these entities after they have been moved to the client, or respawn them ala Quake
	// Perhaps we can do this in deathmatch only.
	//	MAKE_STATIC(ENT(pev));
}


// -------------------------------------------------------------------------------
//
// Monster only clip brush
//
// This brush will be solid for any entity who has the FL_MONSTERCLIP flag set
// in pev->flags
//
// otherwise it will be invisible and not solid.  This can be used to keep
// specific monsters out of certain areas
//
// -------------------------------------------------------------------------------
class CFuncMonsterClip : public CFuncWall
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override {} // Clear out func_wall's use function
};

LINK_ENTITY_TO_CLASS(func_monsterclip, CFuncMonsterClip);

void CFuncMonsterClip::Spawn()
{
	CFuncWall::Spawn();
	if (CVAR_GET_FLOAT("showtriggers") == 0)
		pev->effects = EF_NODRAW;
	pev->flags |= FL_MONSTERCLIP;
}


// =================== FUNC_ROTATING ==============================================
class CFuncRotating : public CBaseEntity
{
public:
	// basic functions
	void Spawn() override;
	void Precache() override;
	void EXPORT SpinUp();
	void EXPORT SpinDown();
	void EXPORT WaitForStart(); // LRC - handle startup behavior
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT HurtTouch(CBaseEntity* pOther);
	void EXPORT RotatingUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT Rotate();
	void RampPitchVol(bool fUp);
	void Blocked(CBaseEntity* pOther) override;
	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	STATE GetState() override { return m_iState; }  // LRC

	static TYPEDESCRIPTION m_SaveData[];

	float m_flFanFriction;
	float m_flAttenuation;
	float m_flVolume;
	float m_pitch;
	int m_sounds;
	STATE m_iState;     // LRC
	float m_fCurSpeed;  // LRC - current speed during spin-up/down
};

TYPEDESCRIPTION CFuncRotating::m_SaveData[] =
	{
		DEFINE_FIELD(CFuncRotating, m_flFanFriction, FIELD_FLOAT),
		DEFINE_FIELD(CFuncRotating, m_flAttenuation, FIELD_FLOAT),
		DEFINE_FIELD(CFuncRotating, m_flVolume, FIELD_FLOAT),
		DEFINE_FIELD(CFuncRotating, m_pitch, FIELD_FLOAT),
		DEFINE_FIELD(CFuncRotating, m_sounds, FIELD_INTEGER),
		DEFINE_FIELD(CFuncRotating, m_iState, FIELD_INTEGER),  // LRC
		DEFINE_FIELD(CFuncRotating, m_fCurSpeed, FIELD_FLOAT), // LRC
};

IMPLEMENT_SAVERESTORE(CFuncRotating, CBaseEntity);


LINK_ENTITY_TO_CLASS(func_rotating, CFuncRotating);

bool CFuncRotating::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "fanfriction"))
	{
		m_flFanFriction = atof(pkvd->szValue) / 100;
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "Volume"))
	{
		m_flVolume = atof(pkvd->szValue) / 10.0;

		if (m_flVolume > 1.0)
			m_flVolume = 1.0;
		if (m_flVolume < 0.0)
			m_flVolume = 0.0;
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "spawnorigin"))
	{
		Vector tmp;
		UTIL_StringToVector((float*)tmp, pkvd->szValue);
		if (tmp != g_vecZero)
			pev->origin = tmp;
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

/*QUAKED func_rotating (0 .5 .8) ? START_ON REVERSE X_AXIS Y_AXIS
You need to have an origin brush as part of this entity.  The  
center of that brush will be
the point around which it is rotated. It will rotate around the Z  
axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"speed" determines how fast it moves; default value is 100.
"dmg"	damage to inflict when blocked (2 default)

REVERSE will cause the it to rotate in the opposite direction.
*/


void CFuncRotating::Spawn()
{
	// set final pitch.  Must not be PITCH_NORM, since we
	// plan on pitch shifting later.

	m_pitch = PITCH_NORM - 1;

	// maintain compatibility with previous maps
	if (m_flVolume == 0.0)
		m_flVolume = 1.0;

	// if the designer didn't set a sound attenuation, default to one.
	m_flAttenuation = ATTN_NORM;

	if (FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_SMALLRADIUS))
	{
		m_flAttenuation = ATTN_IDLE;
	}
	else if (FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_MEDIUMRADIUS))
	{
		m_flAttenuation = ATTN_STATIC;
	}
	else if (FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_LARGERADIUS))
	{
		m_flAttenuation = ATTN_NORM;
	}

	// prevent divide by zero if level designer forgets friction!
	if (m_flFanFriction <= 0) // LRC - also guard against negative values
	{
		m_flFanFriction = 1;
	}

	if (FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_Z_AXIS))
		pev->movedir = Vector(0, 0, 1);
	else if (FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_X_AXIS))
		pev->movedir = Vector(1, 0, 0);
	else
		pev->movedir = Vector(0, 1, 0); // y-axis

	// check for reverse rotation
	if (FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_BACKWARDS))
		pev->movedir = pev->movedir * -1;

	// some rotating objects like fake volumetric lights will not be solid.
	if (FBitSet(pev->spawnflags, SF_ROTATING_NOT_SOLID))
	{
		pev->solid = SOLID_NOT;
		pev->skin = CONTENTS_EMPTY;
		pev->movetype = MOVETYPE_PUSH;
	}
	else
	{
		pev->solid = SOLID_BSP;
		pev->movetype = MOVETYPE_PUSH;
	}

	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));

	// SoHL 1.5 - Auto-detect rotation center if no origin brush is set
	if (pev->origin == g_vecZero)
	{
		pev->origin = (pev->mins + pev->maxs) * 0.5;
		UTIL_SetOrigin(pev, pev->origin);
	}

	SetUse(&CFuncRotating::RotatingUse);
	// did level designer forget to assign speed?
	if (pev->speed <= 0)
		pev->speed = 0;

	// Removed this per level designers request.  -- JAY
	//	if (pev->dmg == 0)
	//		pev->dmg = 2;

	// instant-use brush?
	if (FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_INSTANT))
	{
		SetThink(&CFuncRotating::WaitForStart); // LRC - use WaitForStart for proper deferred spin-up
		SetNextThink(1.5); // leave a magic delay for client to start up
	}
	else if (FStringNull(pev->targetname)) // LRC - start immediately if unnamed
	{
		SetThink(&CFuncRotating::WaitForStart);
		SetNextThink(1.5);
	}
	// can this brush inflict pain?
	if (FBitSet(pev->spawnflags, SF_BRUSH_HURT))
	{
		SetTouch(&CFuncRotating::HurtTouch);
	}

	m_iState = STATE_OFF;  // LRC

	Precache();
}


void CFuncRotating::Precache()
{
	char* szSoundFile = (char*)STRING(pev->message);

	// set up fan sounds

	if (!FStringNull(pev->message) && strlen(szSoundFile) > 0)
	{
		// if a path is set for a wave, use it

		PRECACHE_SOUND(szSoundFile);

		pev->noiseRunning = ALLOC_STRING(szSoundFile);
	}
	else
	{
		// otherwise use preset sound
		switch (m_sounds)
		{
		case 1:
			PRECACHE_SOUND("fans/fan1.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan1.wav");
			break;
		case 2:
			PRECACHE_SOUND("fans/fan2.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan2.wav");
			break;
		case 3:
			PRECACHE_SOUND("fans/fan3.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan3.wav");
			break;
		case 4:
			PRECACHE_SOUND("fans/fan4.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan4.wav");
			break;
		case 5:
			PRECACHE_SOUND("fans/fan5.wav");
			pev->noiseRunning = ALLOC_STRING("fans/fan5.wav");
			break;

		case 0:
		default:
			if (!FStringNull(pev->message) && strlen(szSoundFile) > 0)
			{
				PRECACHE_SOUND(szSoundFile);

				pev->noiseRunning = ALLOC_STRING(szSoundFile);
				break;
			}
			else
			{
				pev->noiseRunning = ALLOC_STRING("common/null.wav");
				break;
			}
		}
	}

	if (pev->avelocity != g_vecZero)
	{
		// if fan was spinning, and we went through transition or save/restore,
		// make sure we restart the sound.  1.5 sec delay is magic number. KDB
		if (!m_pMoveWith) // LRC - only restart if not a MoveWith child (parent will handle timing)
		{
			SetThink(&CFuncRotating::SpinUp);
			SetNextThink(1.5);
		}
	}
}



//
// Touch - will hurt others based on how fast the brush is spinning
//
void CFuncRotating::HurtTouch(CBaseEntity* pOther)
{
	entvars_t* pevOther = pOther->pev;

	// we can't hurt this thing, so we're not concerned with it
	if (0 == pevOther->takedamage)
		return;

	// calculate damage based on rotation speed
	pev->dmg = m_fCurSpeed / 10; // LRC - use tracked speed rather than raw avelocity length

	pOther->TakeDamage(pev, pev, pev->dmg, DMG_CRUSH);

	pevOther->velocity = (pevOther->origin - VecBModelOrigin(pev)).Normalize() * pev->dmg;
}

//
// RampPitchVol - ramp pitch and volume up to final values, based on difference
// between how fast we're going vs how fast we plan to go
//
#define FANPITCHMIN 30
#define FANPITCHMAX 100

void CFuncRotating::RampPitchVol(bool fUp)
{
	// LRC - use m_fCurSpeed for more accurate pitch/vol ramping
	float fpct = (pev->speed != 0) ? (m_fCurSpeed / pev->speed) : 0.0f;
	float fvol = m_flVolume * fpct; // slowdown volume ramps down to 0

	float fpitch = FANPITCHMIN + (FANPITCHMAX - FANPITCHMIN) * fpct;

	int pitch = (int)fpitch;
	if (pitch == PITCH_NORM)
		pitch = PITCH_NORM - 1;

	// change the fan's vol and pitch

	EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseRunning),
		fvol, m_flAttenuation, SND_CHANGE_PITCH | SND_CHANGE_VOL, pitch);
}

//
// SpinUp - accelerates a non-moving func_rotating up to it's speed
//
void CFuncRotating::SpinUp()
{
	SetNextThink(0.1);
	m_fCurSpeed += pev->speed * m_flFanFriction; // LRC - track current speed separately

	// if we've met or exceeded target speed, set target speed and stop thinking
	if (m_fCurSpeed >= pev->speed)
	{
		m_fCurSpeed = pev->speed; // set speed in case we overshot
		m_iState = STATE_ON;  // LRC
		UTIL_SetAvelocity(this, pev->movedir * m_fCurSpeed); // LRC
		EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseRunning),
			m_flVolume, m_flAttenuation, SND_CHANGE_PITCH | SND_CHANGE_VOL, FANPITCHMAX);

		SetThink(&CFuncRotating::Rotate);
		Rotate();
	}
	else
	{
		UTIL_SetAvelocity(this, pev->movedir * m_fCurSpeed); // LRC
		RampPitchVol(true);
	}
}

//
// SpinDown - decelerates a moving func_rotating to a standstill.
//
void CFuncRotating::SpinDown()
{
	SetNextThink(0.1);

	m_fCurSpeed -= pev->speed * m_flFanFriction; // LRC - track current speed separately

	// if we've decelerated to zero, stop
	if (m_fCurSpeed <= 0) // LRC - stop when speed reaches or drops below zero
	{
		m_fCurSpeed = 0; // set speed in case we overshot
		m_iState = STATE_OFF;  // LRC
		UTIL_SetAvelocity(this, g_vecZero); // LRC

		// stop sound, we're done
		EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseRunning /* Stop */),
			0, 0, SND_STOP, m_pitch);

		SetThink(&CFuncRotating::Rotate);
		Rotate();
	}
	else
	{
		UTIL_SetAvelocity(this, pev->movedir * m_fCurSpeed); // LRC
		RampPitchVol(false);
	}
}

void CFuncRotating::Rotate()
{
	SetNextThink(10);
}

// LRC - wait one frame before starting rotation, to allow other entities to spawn
void CFuncRotating::WaitForStart()
{
	SetThink(&CFuncRotating::SpinUp);
	SetNextThink(0.1);
}

//=========================================================
// Rotating Use - when a rotating brush is triggered
//=========================================================
void CFuncRotating::RotatingUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// LRC - ShouldToggle: respect USE_ON / USE_OFF / USE_TOGGLE
	if (!ShouldToggle(useType, m_fCurSpeed != 0)) // LRC - use m_fCurSpeed for state
		return;

	// is this a brush that should accelerate and decelerate when turned on/off (fan)?
	if (FBitSet(pev->spawnflags, SF_BRUSH_ACCDCC))
	{
		// fan is spinning, so stop it.
		if (m_fCurSpeed != 0) // LRC
		{
			SetThink(&CFuncRotating::SpinDown);
			//EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, (char *)STRING(pev->noiseStop),
			//	m_flVolume, m_flAttenuation, 0, m_pitch);

			SetNextThink(0.1);
		}
		else // fan is not moving, so start it
		{
			SetThink(&CFuncRotating::SpinUp);
			EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseRunning),
				0.01, m_flAttenuation, 0, FANPITCHMIN);

			SetNextThink(0.1);
		}
	}
	else if (!FBitSet(pev->spawnflags, SF_BRUSH_ACCDCC)) //this is a normal start/stop brush.
	{
		if (m_fCurSpeed != 0) // LRC
		{
			// play stopping sound here
			SetThink(&CFuncRotating::SpinDown);

			// EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, (char *)STRING(pev->noiseStop),
			//	m_flVolume, m_flAttenuation, 0, m_pitch);

			SetNextThink(0.1);
			// pev->avelocity = g_vecZero;
		}
		else
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char*)STRING(pev->noiseRunning),
				m_flVolume, m_flAttenuation, 0, FANPITCHMAX);
			m_fCurSpeed = pev->speed; // LRC
			m_iState = STATE_ON;      // LRC
			UTIL_SetAvelocity(this, pev->movedir * m_fCurSpeed); // LRC

			SetThink(&CFuncRotating::Rotate);
			Rotate();
		}
	}
}


//
// RotatingBlocked - An entity has blocked the brush
//
void CFuncRotating::Blocked(CBaseEntity* pOther)

{
	pOther->TakeDamage(pev, pev, pev->dmg, DMG_CRUSH);
}






//#endif


class CPendulum : public CBaseEntity
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT Swing();
	void EXPORT PendulumUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT Stop();
	void Touch(CBaseEntity* pOther) override;
	void EXPORT RopeTouch(CBaseEntity* pOther); // this touch func makes the pendulum a rope
	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	void Blocked(CBaseEntity* pOther) override;

	static TYPEDESCRIPTION m_SaveData[];

	float m_accel;	  // Acceleration
	float m_distance; //
	float m_time;
	float m_damp;
	float m_maxSpeed;
	float m_dampSpeed;
	Vector m_center;
	Vector m_start;
};

LINK_ENTITY_TO_CLASS(func_pendulum, CPendulum);

TYPEDESCRIPTION CPendulum::m_SaveData[] =
	{
		DEFINE_FIELD(CPendulum, m_accel, FIELD_FLOAT),
		DEFINE_FIELD(CPendulum, m_distance, FIELD_FLOAT),
		DEFINE_FIELD(CPendulum, m_time, FIELD_TIME),
		DEFINE_FIELD(CPendulum, m_damp, FIELD_FLOAT),
		DEFINE_FIELD(CPendulum, m_maxSpeed, FIELD_FLOAT),
		DEFINE_FIELD(CPendulum, m_dampSpeed, FIELD_FLOAT),
		DEFINE_FIELD(CPendulum, m_center, FIELD_VECTOR),
		DEFINE_FIELD(CPendulum, m_start, FIELD_VECTOR),
};

IMPLEMENT_SAVERESTORE(CPendulum, CBaseEntity);



bool CPendulum::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "distance"))
	{
		m_distance = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "damp"))
	{
		m_damp = atof(pkvd->szValue) * 0.001;
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}


void CPendulum::Spawn()
{
	// set the axis of rotation
	CBaseToggle::AxisDir(pev);

	if (FBitSet(pev->spawnflags, SF_DOOR_PASSABLE))
		pev->solid = SOLID_NOT;
	else
		pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;
	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (m_distance == 0)
		return;

	if (pev->speed == 0)
		pev->speed = 100;

	m_accel = (pev->speed * pev->speed) / (2 * fabs(m_distance)); // Calculate constant acceleration from speed and distance
	m_maxSpeed = pev->speed;
	m_start = pev->angles;
	m_center = pev->angles + (m_distance * 0.5) * pev->movedir;

	if (FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_INSTANT))
	{
		SetThink(&CPendulum::SUB_CallUseToggle);
		SetNextThink(0.1);
	}
	pev->speed = 0;
	SetUse(&CPendulum::PendulumUse);

	if (FBitSet(pev->spawnflags, SF_PENDULUM_SWING))
	{
		SetTouch(&CPendulum::RopeTouch);
	}
}


void CPendulum::PendulumUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// LRC - ShouldToggle: respect USE_ON / USE_OFF / USE_TOGGLE
	if (!ShouldToggle(useType, 0 != pev->speed))
		return;

	if (0 != pev->speed) // Pendulum is moving, stop it and auto-return if necessary
	{
		if (FBitSet(pev->spawnflags, SF_PENDULUM_AUTO_RETURN))
		{
			float delta;

			delta = CBaseToggle::AxisDelta(pev->spawnflags, pev->angles, m_start);

			UTIL_SetAvelocity(this, m_maxSpeed * pev->movedir); // LRC
			SetNextThink(delta / m_maxSpeed);
			SetThink(&CPendulum::Stop);
		}
		else
		{
			pev->speed = 0; // Dead stop
			SetThink(NULL);
			UTIL_SetAvelocity(this, g_vecZero); // LRC
		}
	}
	else
	{
		SetNextThink(0.1); // Start the pendulum moving
		m_time = gpGlobals->time;		   // Save time to calculate dt
		SetThink(&CPendulum::Swing);
		m_dampSpeed = m_maxSpeed;
	}
}


void CPendulum::Stop()
{
	UTIL_SetAngles(this, m_start); // LRC
	pev->speed = 0;
	SetThink(NULL);
	UTIL_SetAvelocity(this, g_vecZero); // LRC
}


void CPendulum::Blocked(CBaseEntity* pOther)
{
	m_time = gpGlobals->time;
}


void CPendulum::Swing()
{
	float delta, dt;

	delta = CBaseToggle::AxisDelta(pev->spawnflags, pev->angles, m_center);
	dt = gpGlobals->time - m_time; // How much time has passed?
	m_time = gpGlobals->time;	   // Remember the last time called

	if (delta > 0 && m_accel > 0)
		pev->speed -= m_accel * dt; // Integrate velocity
	else
		pev->speed += m_accel * dt;

	if (pev->speed > m_maxSpeed)
		pev->speed = m_maxSpeed;
	else if (pev->speed < -m_maxSpeed)
		pev->speed = -m_maxSpeed;
	// scale the destdelta vector by the time spent traveling to get velocity
	UTIL_SetAvelocity(this, pev->speed * pev->movedir); // LRC

	// Call this again
	SetNextThink(0.1);

	if (0 != m_damp)
	{
		m_dampSpeed -= m_damp * m_dampSpeed * dt;
		if (m_dampSpeed < 30.0)
		{
			UTIL_SetAngles(this, m_center); // LRC
			pev->speed = 0;
			SetThink(NULL);
			UTIL_SetAvelocity(this, g_vecZero); // LRC
		}
		else if (pev->speed > m_dampSpeed)
			pev->speed = m_dampSpeed;
		else if (pev->speed < -m_dampSpeed)
			pev->speed = -m_dampSpeed;
	}
}


void CPendulum::Touch(CBaseEntity* pOther)
{
	entvars_t* pevOther = pOther->pev;

	if (pev->dmg <= 0)
		return;

	// we can't hurt this thing, so we're not concerned with it
	if (0 == pevOther->takedamage)
		return;

	// calculate damage based on rotation speed
	float damage = pev->dmg * pev->speed * 0.01;

	if (damage < 0)
		damage = -damage;

	pOther->TakeDamage(pev, pev, damage, DMG_CRUSH);

	pevOther->velocity = (pevOther->origin - VecBModelOrigin(pev)).Normalize() * damage;
}

void CPendulum::RopeTouch(CBaseEntity* pOther)
{
	entvars_t* pevOther = pOther->pev;

	if (!pOther->IsPlayer())
	{ // not a player!
		ALERT(at_console, "Not a client\n");
		return;
	}

	if (ENT(pevOther) == pev->enemy)
	{ // this player already on the rope.
		return;
	}

	pev->enemy = pOther->edict();
	pevOther->velocity = g_vecZero;
	pevOther->movetype = MOVETYPE_NONE;
}

// LRC - shiny surfaces
class CShine : public CBaseEntity
{
public:
	void Spawn() override;
	void Activate() override;
};

LINK_ENTITY_TO_CLASS(func_shine, CShine);

void CShine::Spawn()
{
	pev->solid = SOLID_NOT;
	SET_MODEL(ENT(pev), STRING(pev->model));
	pev->movetype = MOVETYPE_NONE;
	pev->effects |= EF_NODRAW;
}

void CShine::Activate()
{
	CBaseEntity::Activate();
	MESSAGE_BEGIN(MSG_ALL, gmsgAddShine);
	WRITE_BYTE(pev->scale * 10);
	WRITE_SHORT(ENTINDEX(ENT(pev)));
	WRITE_COORD(pev->absmin.x);
	WRITE_COORD(pev->absmin.y);
	WRITE_COORD(pev->absmin.z);
	WRITE_COORD(pev->absmax.x);
	WRITE_COORD(pev->absmax.y);
	WRITE_COORD(pev->absmax.z);
	MESSAGE_END();
}
