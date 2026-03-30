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

#pragma once

class ParticleSystem;

class ParticleSystemManager
{
public:
	ParticleSystemManager();
	~ParticleSystemManager();

	void AddSystem(ParticleSystem* pNewSystem);
	ParticleSystem* FindSystem(int iEntIndex);
	void UpdateSystems(float frametime);
	void ClearSystems();

	ParticleSystem* m_pFirstSystem;
};

class ParticleSystem
{
public:
	ParticleSystem(int iEntIndex, const char* szFilename);
	~ParticleSystem();

	void Update(float frametime);
	bool IsAlive() const { return m_bAlive; }

	int m_iEntIndex;
	ParticleSystem* m_pNextSystem;

private:
	bool m_bAlive;
	float m_fLifetime;
	char m_szFilename[256];
};

extern ParticleSystemManager* g_pParticleSystems;
