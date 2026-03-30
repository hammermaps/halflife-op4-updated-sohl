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

===== lights.cpp ========================================================

  spawn and think functions for editor-placed lights

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "UserMessages.h"



#define MIN_CUSTOM_LIGHT_STYLE 32 // LRC - styles 0-31 are reserved engine styles; styles >= 32 are designer-defined

class CTriggerLightstyle; // LRC - forward declaration

class CLight : public CPointEntity
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	// LRC
	STATE GetState() override { return m_iState; }
	int GetStyle() { return m_iszCurrentStyle; } // LRC
	void SetStyle(int iszPattern);                // LRC
	void SetCorrectStyle();                       // LRC

	static TYPEDESCRIPTION m_SaveData[];

	friend class CTriggerLightstyle; // LRC - allow CTriggerLightstyle to access m_iStyle

private:
	int m_iStyle;
	int m_iszPattern;
	STATE m_iState;        // LRC
	int m_iszCurrentStyle; // LRC - currently active pattern
};
LINK_ENTITY_TO_CLASS(light, CLight);

TYPEDESCRIPTION CLight::m_SaveData[] =
	{
		DEFINE_FIELD(CLight, m_iStyle, FIELD_INTEGER),
		DEFINE_FIELD(CLight, m_iszPattern, FIELD_STRING),
		DEFINE_FIELD(CLight, m_iszCurrentStyle, FIELD_STRING), // LRC
};

IMPLEMENT_SAVERESTORE(CLight, CPointEntity);


//
// Cache user-entity-field values until spawn is called.
//
bool CLight::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "style"))
	{
		m_iStyle = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "pitch"))
	{
		pev->angles.x = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "pattern"))
	{
		m_iszPattern = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) LIGHT_START_OFF
Non-displayed light.
Default light value is 300
Default style is 0
If targeted, it will toggle between on or off.
*/

void CLight::Spawn()
{
	if (FStringNull(pev->targetname))
	{ // inert light
		REMOVE_ENTITY(ENT(pev));
		return;
	}

	if (m_iStyle >= MIN_CUSTOM_LIGHT_STYLE) // styles 0-31 are reserved engine styles
	{
		//		CHANGE_METHOD(ENT(pev), em_use, light_use);
		if (FBitSet(pev->spawnflags, SF_LIGHT_START_OFF))
		{
			LIGHT_STYLE(m_iStyle, "a");
			m_iState = STATE_OFF;  // LRC
		}
		else if (!FStringNull(m_iszPattern))
		{
			LIGHT_STYLE(m_iStyle, (char*)STRING(m_iszPattern));
			m_iState = STATE_ON;  // LRC
		}
		else
		{
			LIGHT_STYLE(m_iStyle, "m");
			m_iState = STATE_ON;  // LRC
		}
	}
}


void CLight::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (m_iStyle >= MIN_CUSTOM_LIGHT_STYLE) // styles 0-31 are reserved engine styles
	{
		if (!ShouldToggle(useType, !FBitSet(pev->spawnflags, SF_LIGHT_START_OFF)))
			return;

		if (FBitSet(pev->spawnflags, SF_LIGHT_START_OFF))
		{
			if (!FStringNull(m_iszPattern))
				LIGHT_STYLE(m_iStyle, (char*)STRING(m_iszPattern));
			else
				LIGHT_STYLE(m_iStyle, "m");
			ClearBits(pev->spawnflags, SF_LIGHT_START_OFF);
			m_iState = STATE_ON;  // LRC
		}
		else
		{
			LIGHT_STYLE(m_iStyle, "a");
			SetBits(pev->spawnflags, SF_LIGHT_START_OFF);
			m_iState = STATE_OFF;  // LRC
		}
	}
}

//
// shut up spawn functions for new spotlights
//
LINK_ENTITY_TO_CLASS(light_spot, CLight);


class CEnvLight : public CLight
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
};

LINK_ENTITY_TO_CLASS(light_environment, CEnvLight);

bool CEnvLight::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "_light"))
	{
		int r = 0, g = 0, b = 0, v = 0;
		char szColor[64];
		const int j = sscanf(pkvd->szValue, "%d %d %d %d\n", &r, &g, &b, &v);
		if (j == 1)
		{
			g = b = r;
		}
		else if (j == 4)
		{
			r = r * (v / 255.0);
			g = g * (v / 255.0);
			b = b * (v / 255.0);
		}

		// simulate qrad direct, ambient,and gamma adjustments, as well as engine scaling
		r = pow(r / 114.0, 0.6) * 264;
		g = pow(g / 114.0, 0.6) * 264;
		b = pow(b / 114.0, 0.6) * 264;

		snprintf(szColor, sizeof(szColor), "%d", r);
		CVAR_SET_STRING("sv_skycolor_r", szColor);
		snprintf(szColor, sizeof(szColor), "%d", g);
		CVAR_SET_STRING("sv_skycolor_g", szColor);
		snprintf(szColor, sizeof(szColor), "%d", b);
		CVAR_SET_STRING("sv_skycolor_b", szColor);

		return true;
	}

	return CLight::KeyValue(pkvd);
}


void CEnvLight::Spawn()
{
	char szVector[64];
	UTIL_MakeAimVectors(pev->angles);

	snprintf(szVector, sizeof(szVector), "%f", gpGlobals->v_forward.x);
	CVAR_SET_STRING("sv_skyvec_x", szVector);
	snprintf(szVector, sizeof(szVector), "%f", gpGlobals->v_forward.y);
	CVAR_SET_STRING("sv_skyvec_y", szVector);
	snprintf(szVector, sizeof(szVector), "%f", gpGlobals->v_forward.z);
	CVAR_SET_STRING("sv_skyvec_z", szVector);

	CLight::Spawn();
}

// LRC
void CLight::SetStyle(int iszPattern)
{
	if (m_iStyle < MIN_CUSTOM_LIGHT_STYLE) return; // styles 0-31 are reserved engine styles
	m_iszCurrentStyle = iszPattern;
	if (iszPattern)
		LIGHT_STYLE(m_iStyle, (char*)STRING(iszPattern));
	else
		LIGHT_STYLE(m_iStyle, "m");
}

// LRC
void CLight::SetCorrectStyle()
{
	if (m_iStyle < MIN_CUSTOM_LIGHT_STYLE) return; // styles 0-31 are reserved engine styles
	if (m_iState == STATE_OFF)
		LIGHT_STYLE(m_iStyle, "a");
	else
		SetStyle(m_iszCurrentStyle ? m_iszCurrentStyle : m_iszPattern);
}

// LRC - CLightDynamic: dynamic entity light (works like flashlight)
class CLightDynamic : public CPointEntity
{
public:
	void Spawn() override
	{
		if (pev->armorvalue == 0)
			pev->armorvalue = 200;
	}
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	STATE GetState() override { return m_iState; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

private:
	STATE m_iState = STATE_OFF;
};
LINK_ENTITY_TO_CLASS(env_dlight, CLightDynamic);
TYPEDESCRIPTION CLightDynamic::m_SaveData[] = {
	DEFINE_FIELD(CLightDynamic, m_iState, FIELD_INTEGER),
};
IMPLEMENT_SAVERESTORE(CLightDynamic, CPointEntity);

void CLightDynamic::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType, m_iState == STATE_ON))
		return;
	if (m_iState == STATE_ON)
	{
		m_iState = STATE_OFF;
		pev->effects |= EF_NODRAW;
	}
	else
	{
		m_iState = STATE_ON;
		pev->effects &= ~EF_NODRAW;
	}

	// Send keyed dynamic light message to client
	MESSAGE_BEGIN(MSG_ALL, gmsgKeyedDLight);
	WRITE_BYTE(ENTINDEX(ENT(pev)));
	WRITE_BYTE(m_iState == STATE_ON ? 1 : 0);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_BYTE((int)pev->armorvalue); // radius
	WRITE_BYTE((int)pev->rendercolor.x); // R
	WRITE_BYTE((int)pev->rendercolor.y); // G
	WRITE_BYTE((int)pev->rendercolor.z); // B
	MESSAGE_END();
}

// LRC - CTriggerLightstyle: temporarily changes the style of a CLight entity
class CTriggerLightstyle : public CPointEntity
{
public:
	bool KeyValue(KeyValueData* pkvd) override
	{
		if (FStrEq(pkvd->szKeyName, "style"))
		{
			m_iszNewStyle = ALLOC_STRING(pkvd->szValue);
			return true;
		}
		return CPointEntity::KeyValue(pkvd);
	}
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override
	{
		CBaseEntity* pLight = UTIL_FindEntityByTargetname(NULL, STRING(pev->target));
		while (pLight)
		{
			if (FClassnameIs(pLight->pev, "light") || FClassnameIs(pLight->pev, "light_spot"))
			{
				CLight* pCLight = static_cast<CLight*>(pLight);
				if (!FStringNull(m_iszNewStyle))
					LIGHT_STYLE(pCLight->m_iStyle, (char*)STRING(m_iszNewStyle));
			}
			pLight = UTIL_FindEntityByTargetname(pLight, STRING(pev->target));
		}
	}

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

private:
	int m_iszNewStyle = 0;
};
LINK_ENTITY_TO_CLASS(trigger_lightstyle, CTriggerLightstyle);
TYPEDESCRIPTION CTriggerLightstyle::m_SaveData[] = {
	DEFINE_FIELD(CTriggerLightstyle, m_iszNewStyle, FIELD_STRING),
};
IMPLEMENT_SAVERESTORE(CTriggerLightstyle, CPointEntity);
