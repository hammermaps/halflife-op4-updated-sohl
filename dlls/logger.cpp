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

#include "extdll.h"
#include "util.h"
#include "logger.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>

// ---------------------------------------------------------------------------
// Global instance
// ---------------------------------------------------------------------------
CLogger g_Logger;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/** Map LogLevel to the engine ALERT_TYPE for console forwarding. */
static ALERT_TYPE LogLevelToAlertType(LogLevel level)
{
	switch (level)
	{
	case LogLevel::DEBUG:    return at_aiconsole;
	case LogLevel::INFO:     return at_console;
	case LogLevel::WARNING:  return at_warning;
	case LogLevel::ERR:      return at_error;
	case LogLevel::CRITICAL: return at_error;
	default:                 return at_console;
	}
}

// ---------------------------------------------------------------------------
// CLogger
// ---------------------------------------------------------------------------
CLogger::CLogger()
	: m_pFile(nullptr)
	, m_pLevelCvar(nullptr)
{
}

CLogger::~CLogger()
{
	Shutdown();
}

void CLogger::Init(cvar_t* pLevelCvar)
{
	if (m_pFile)
		return; // Already initialised

	m_pLevelCvar = pLevelCvar;

	// Build path: <gamedir>/sohl_debug.log
	char szGameDir[256] = {};
	GET_GAME_DIR(szGameDir);

	char szPath[512];
	snprintf(szPath, sizeof(szPath), "%s/sohl_debug.log", szGameDir);

	m_pFile = fopen(szPath, "a");
	if (!m_pFile)
	{
		ALERT(at_error, "[Logger] Failed to open log file: %s\n", szPath);
		return;
	}

	// Write a session separator so restarts are clearly visible in the file.
	time_t now = time(nullptr);
	struct tm* t = localtime(&now);
	char szTimestamp[32];
	strftime(szTimestamp, sizeof(szTimestamp), "%Y-%m-%d %H:%M:%S", t);

	fprintf(m_pFile,
		"------------------------------------------------------------\n"
		"[%s] game.INFO: Logger initialised — sohl_debug.log\n"
		"------------------------------------------------------------\n",
		szTimestamp);
	fflush(m_pFile);

	ALERT(at_console, "[Logger] Writing to %s\n", szPath);
}

void CLogger::Shutdown()
{
	if (!m_pFile)
		return;

	time_t now = time(nullptr);
	struct tm* t = localtime(&now);
	char szTimestamp[32];
	strftime(szTimestamp, sizeof(szTimestamp), "%Y-%m-%d %H:%M:%S", t);

	fprintf(m_pFile,
		"[%s] game.INFO: Logger shutdown\n"
		"------------------------------------------------------------\n\n",
		szTimestamp);

	fflush(m_pFile);
	fclose(m_pFile);
	m_pFile = nullptr;
}

LogLevel CLogger::GetCurrentLevel() const
{
	if (m_pLevelCvar)
	{
		int val = static_cast<int>(m_pLevelCvar->value);
		if (val < 0)
			val = 0;
		if (val >= static_cast<int>(LogLevel::COUNT))
			val = static_cast<int>(LogLevel::COUNT) - 1;
		return static_cast<LogLevel>(val);
	}
	return LogLevel::WARNING; // Safe default before cvar is registered
}

void CLogger::Write(LogLevel level, const char* channel, const char* fmt, ...)
{
	// Ignore messages below the current minimum level.
	if (level < GetCurrentLevel())
		return;

	// Format the caller's message.
	char szMessage[2048];
	va_list args;
	va_start(args, fmt);
	vsnprintf(szMessage, sizeof(szMessage), fmt, args);
	va_end(args);

	// Strip a trailing newline, if present — we add our own.
	size_t len = strlen(szMessage);
	while (len > 0 && (szMessage[len - 1] == '\n' || szMessage[len - 1] == '\r'))
	{
		szMessage[--len] = '\0';
	}

	// Build the monolog-style timestamp.
	time_t now = time(nullptr);
	struct tm* t = localtime(&now);
	char szTimestamp[32];
	strftime(szTimestamp, sizeof(szTimestamp), "%Y-%m-%d %H:%M:%S", t);

	// Write to file if initialised.
	if (m_pFile)
	{
		fprintf(m_pFile, "[%s] %s.%s: %s\n",
			szTimestamp, channel, LogLevelName(level), szMessage);
		fflush(m_pFile);
	}

	// Forward to the engine console for interactive visibility.
	// Always forward ERR/CRITICAL; forward others only when the logger is
	// initialised so we don't duplicate messages that were already printed
	// before Init() ran.
	if (level >= LogLevel::ERR)
	{
		ALERT(LogLevelToAlertType(level), "[%s] %s\n", LogLevelName(level), szMessage);
	}
	else if (m_pFile)
	{
		// For DEBUG/INFO/WARNING also show on console when file logging is active.
		ALERT(LogLevelToAlertType(level), "[%s] %s\n", LogLevelName(level), szMessage);
	}
}
