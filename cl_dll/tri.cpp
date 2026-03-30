//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

// Triangle rendering, if any

#include "hud.h"
#include "cl_util.h"

// Triangle rendering apis are in gEngfuncs.pTriAPI

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "Exports.h"

#include "particleman.h"
#include "tri.h"
extern IParticleMan* g_pParticleMan;

// LRC - CShinySurface: reflective surface rendering
class CShinySurface
{
public:
	CShinySurface(float fScale, float fxMin, float fyMin, float fzMin, float fxMax, float fyMax, float fzMax);
	~CShinySurface();
	void Draw(const Vector& vecOrigin);

	CShinySurface* m_pNext;
	float m_fScale;
	float m_fxMin, m_fyMin, m_fzMin;
	float m_fxMax, m_fyMax, m_fzMax;
	int m_iEntIndex;
};

CShinySurface::CShinySurface(float fScale, float fxMin, float fyMin, float fzMin, float fxMax, float fyMax, float fzMax)
{
	m_fScale = fScale;
	m_fxMin = fxMin;
	m_fyMin = fyMin;
	m_fzMin = fzMin;
	m_fxMax = fxMax;
	m_fyMax = fyMax;
	m_fzMax = fzMax;
	m_iEntIndex = 0;
	m_pNext = NULL;
}

CShinySurface::~CShinySurface()
{
	if (m_pNext)
		delete m_pNext;
}

void CShinySurface::Draw(const Vector& vecOrigin)
{
	// Placeholder: actual reflection rendering requires engine-level support
	// The shiny surface data is available for mods that implement custom rendering
	if (m_pNext)
		m_pNext->Draw(vecOrigin);
}

/*
=================
HUD_DrawNormalTriangles

Non-transparent triangles-- add them here
=================
*/
void DLLEXPORT HUD_DrawNormalTriangles()
{
	//	RecClDrawNormalTriangles();

	gHUD.m_Spectator.DrawOverview();
}


/*
=================
HUD_DrawTransparentTriangles

Render any triangles with transparent rendermode needs here
=================
*/
void DLLEXPORT HUD_DrawTransparentTriangles()
{
	//	RecClDrawTransparentTriangles();


	if (g_pParticleMan)
		g_pParticleMan->Update();
}
