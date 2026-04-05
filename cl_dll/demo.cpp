/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
#include "demo.h"
#include "demo_api.h"
#include <memory.h>
#include "Exports.h"

int g_demosniper = 0;
int g_demosniperdamage = 0;
float g_demosniperorg[3];
float g_demosniperangles[3];
float g_demozoom;

// Buffer read/write helpers that avoid type-punning via pointer casts.
static void WriteInt(unsigned char* buf, int& pos, int value)
{
	memcpy(&buf[pos], &value, sizeof(value));
	pos += sizeof(value);
}

static void WriteFloat(unsigned char* buf, int& pos, float value)
{
	memcpy(&buf[pos], &value, sizeof(value));
	pos += sizeof(value);
}

static int ReadInt(const unsigned char* buf, int& pos)
{
	int value;
	memcpy(&value, &buf[pos], sizeof(value));
	pos += sizeof(value);
	return value;
}

static float ReadFloat(const unsigned char* buf, int& pos)
{
	float value;
	memcpy(&value, &buf[pos], sizeof(value));
	pos += sizeof(value);
	return value;
}

/*
=====================
Demo_WriteBuffer

Write some data to the demo stream
=====================
*/
void Demo_WriteBuffer(int type, int size, unsigned char* buffer)
{
	int pos = 0;
	unsigned char buf[32 * 1024];
	WriteInt(buf, pos, type);

	memcpy(&buf[pos], buffer, size);

	// Write full buffer out
	gEngfuncs.pDemoAPI->WriteBuffer(size + sizeof(int), buf);
}

/*
=====================
Demo_ReadBuffer

Engine wants us to parse some data from the demo stream
=====================
*/
void DLLEXPORT Demo_ReadBuffer(int size, unsigned char* buffer)
{
	//	RecClReadDemoBuffer(size, buffer);

	int i = 0;

	int type = ReadInt(buffer, i);
	switch (type)
	{
	case TYPE_SNIPERDOT:
		g_demosniper = ReadInt(buffer, i);

		if (0 != g_demosniper)
		{
			g_demosniperdamage = ReadInt(buffer, i);

			g_demosniperangles[0] = ReadFloat(buffer, i);
			g_demosniperangles[1] = ReadFloat(buffer, i);
			g_demosniperangles[2] = ReadFloat(buffer, i);
			g_demosniperorg[0] = ReadFloat(buffer, i);
			g_demosniperorg[1] = ReadFloat(buffer, i);
			g_demosniperorg[2] = ReadFloat(buffer, i);
		}
		break;
	case TYPE_ZOOM:
		g_demozoom = ReadFloat(buffer, i);
		break;
	default:
		gEngfuncs.Con_DPrintf("Unknown demo buffer type, skipping.\n");
		break;
	}
}
