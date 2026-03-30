/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
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
//
//  hud_msg.cpp
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "r_efx.h"

#include "vgui_TeamFortressViewport.h"
#include "vgui_ScorePanel.h"

#include "particleman.h"
extern IParticleMan* g_pParticleMan;

extern int giTeamplay;

extern BEAM* pBeam;
extern BEAM* pBeam2;


/// USER-DEFINED SERVER MESSAGE HANDLERS

bool CHud::MsgFunc_ResetHUD(const char* pszName, int iSize, void* pbuf)
{
	ASSERT(iSize == 0);

	// clear all hud data
	HUDLIST* pList = m_pHudList;

	while (pList)
	{
		if (pList->p)
			pList->p->Reset();
		pList = pList->pNext;
	}

	//Reset weapon bits.
	m_iWeaponBits = 0ULL;

	// reset sensitivity
	m_flMouseSensitivity = 0;

	// reset concussion effect
	m_iConcussionEffect = 0;

	// reset fog
	m_bFogOn = false;

	// reset sky
	m_iSkyMode = SKY_OFF;

	return true;
}

void CAM_ToFirstPerson();

void CHud::MsgFunc_ViewMode(const char* pszName, int iSize, void* pbuf)
{
	CAM_ToFirstPerson();
}

void CHud::MsgFunc_InitHUD(const char* pszName, int iSize, void* pbuf)
{
	// prepare all hud data
	HUDLIST* pList = m_pHudList;

	while (pList)
	{
		if (pList->p)
			pList->p->InitHUDData();
		pList = pList->pNext;
	}


	//TODO: needs to be called on every map change, not just when starting a new game
	if (g_pParticleMan)
		g_pParticleMan->ResetParticles();

	//Probably not a good place to put this.
	pBeam = pBeam2 = NULL;

	// Clear shiny surfaces
	if (m_pShinySurface)
	{
		delete m_pShinySurface;
		m_pShinySurface = NULL;
	}
}


bool CHud::MsgFunc_GameMode(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	//Note: this user message could be updated to include multiple gamemodes
	//See CHalfLifeTeamplay::UpdateGameMode
	//TODO: define game mode constants
	m_Teamplay = giTeamplay = READ_BYTE();

	if (gViewPort && !gViewPort->m_pScoreBoard)
	{
		gViewPort->CreateScoreBoard();
		gViewPort->m_pScoreBoard->Initialize();

		if (!gHUD.m_iIntermission)
		{
			gViewPort->HideScoreBoard();
		}
	}

#ifdef STEAM_RICH_PRESENCE
	if (m_Teamplay)
		gEngfuncs.pfnClientCmd("richpresence_gamemode Teamplay\n");
	else
		gEngfuncs.pfnClientCmd("richpresence_gamemode\n"); // reset

	gEngfuncs.pfnClientCmd("richpresence_update\n");
#endif

	return true;
}


bool CHud::MsgFunc_Damage(const char* pszName, int iSize, void* pbuf)
{
	int armor, blood;
	Vector from;
	int i;
	float count;

	BEGIN_READ(pbuf, iSize);
	armor = READ_BYTE();
	blood = READ_BYTE();

	for (i = 0; i < 3; i++)
		from[i] = READ_COORD();

	count = (blood * 0.5) + (armor * 0.5);

	if (count < 10)
		count = 10;

	// TODO: kick viewangles,  show damage visually

	return true;
}

bool CHud::MsgFunc_Concuss(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	m_iConcussionEffect = READ_BYTE();
	if (0 != m_iConcussionEffect)
	{
		this->m_StatusIcons.EnableIcon("dmg_concuss", giR, giG, giB);
	}
	else
		this->m_StatusIcons.DisableIcon("dmg_concuss");
	return true;
}

bool CHud::MsgFunc_Weapons(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	const std::uint64_t lowerBits = READ_LONG();
	const std::uint64_t upperBits = READ_LONG();

	m_iWeaponBits = (lowerBits & 0XFFFFFFFF) | ((upperBits & 0XFFFFFFFF) << 32ULL);

	return true;
}

bool CHud::MsgFunc_SetFog(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	m_iFogColor_R = READ_BYTE();
	m_iFogColor_G = READ_BYTE();
	m_iFogColor_B = READ_BYTE();
	m_fStartDist = (float)READ_SHORT();
	m_fEndDist = (float)READ_SHORT();
	m_fFogDensity = (float)READ_SHORT() / 1000.0f;
	m_bFogOn = (m_fEndDist > 0);
	return true;
}

bool CHud::MsgFunc_SetSky(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	m_iSkyMode = READ_BYTE();
	if (m_iSkyMode == SKY_ON)
	{
		m_vecSkyPos.x = READ_COORD();
		m_vecSkyPos.y = READ_COORD();
		m_vecSkyPos.z = READ_COORD();
	}
	return true;
}

void CHud::MsgFunc_AddShine(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	float fScale = (float)READ_BYTE() / 10.0f;
	int iEntIndex = READ_SHORT();
	float fxMin = READ_COORD();
	float fyMin = READ_COORD();
	float fzMin = READ_COORD();
	float fxMax = READ_COORD();
	float fyMax = READ_COORD();
	float fzMax = READ_COORD();

	CShinySurface* pSurface = new CShinySurface(fScale, fxMin, fyMin, fzMin, fxMax, fyMax, fzMax);
	pSurface->m_iEntIndex = iEntIndex;
	pSurface->m_pNext = m_pShinySurface;
	m_pShinySurface = pSurface;
}

void CHud::MsgFunc_KeyedDLight(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int iKey = READ_BYTE();
	int iActive = READ_BYTE();
	float x = READ_COORD();
	float y = READ_COORD();
	float z = READ_COORD();
	int iRadius = READ_BYTE();
	int r = READ_BYTE();
	int g = READ_BYTE();
	int b = READ_BYTE();

	if (iActive)
	{
		dlight_t* dl = gEngfuncs.pEfxAPI->CL_AllocDlight(iKey);
		dl->origin[0] = x;
		dl->origin[1] = y;
		dl->origin[2] = z;
		dl->radius = (float)iRadius;
		dl->color.r = r;
		dl->color.g = g;
		dl->color.b = b;
		dl->die = gEngfuncs.GetClientTime() + 99999.0f;
		dl->decay = 0;
	}
	else
	{
		dlight_t* dl = gEngfuncs.pEfxAPI->CL_AllocDlight(iKey);
		dl->die = 0;
		dl->radius = 0;
	}
}
