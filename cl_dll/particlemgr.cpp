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

#include "hud.h"
#include "cl_util.h"
#include "particlemgr.h"

#include <cstring>

ParticleSystemManager* g_pParticleSystems = NULL;

// =============================================
// ParticleSystemManager
// =============================================
ParticleSystemManager::ParticleSystemManager()
{
	m_pFirstSystem = NULL;
}

ParticleSystemManager::~ParticleSystemManager()
{
	ClearSystems();
}

void ParticleSystemManager::AddSystem(ParticleSystem* pNewSystem)
{
	pNewSystem->m_pNextSystem = m_pFirstSystem;
	m_pFirstSystem = pNewSystem;
}

ParticleSystem* ParticleSystemManager::FindSystem(int iEntIndex)
{
	ParticleSystem* pSys = m_pFirstSystem;
	while (pSys)
	{
		if (pSys->m_iEntIndex == iEntIndex)
			return pSys;
		pSys = pSys->m_pNextSystem;
	}
	return NULL;
}

void ParticleSystemManager::UpdateSystems(float frametime)
{
	ParticleSystem** ppPrev = &m_pFirstSystem;
	ParticleSystem* pSys = m_pFirstSystem;
	while (pSys)
	{
		pSys->Update(frametime);
		if (!pSys->IsAlive())
		{
			*ppPrev = pSys->m_pNextSystem;
			ParticleSystem* pDead = pSys;
			pSys = pSys->m_pNextSystem;
			delete pDead;
		}
		else
		{
			ppPrev = &pSys->m_pNextSystem;
			pSys = pSys->m_pNextSystem;
		}
	}
}

void ParticleSystemManager::ClearSystems()
{
	ParticleSystem* pSys = m_pFirstSystem;
	while (pSys)
	{
		ParticleSystem* pNext = pSys->m_pNextSystem;
		delete pSys;
		pSys = pNext;
	}
	m_pFirstSystem = NULL;
}

// =============================================
// ParticleSystem
// =============================================
ParticleSystem::ParticleSystem(int iEntIndex, const char* szFilename)
{
	m_iEntIndex = iEntIndex;
	m_pNextSystem = NULL;
	m_bAlive = true;
	m_fLifetime = 0;
	strncpy(m_szFilename, szFilename, sizeof(m_szFilename) - 1);
	m_szFilename[sizeof(m_szFilename) - 1] = '\0';
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::Update(float frametime)
{
	m_fLifetime += frametime;
	// Particle rendering would be implemented here
	// using the aurora particle file format
}
