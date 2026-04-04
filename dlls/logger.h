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

/**
 * @file logger.h
 * @brief System-wide logger for the game DLL.
 *
 * Writes timestamped log entries to <gamedir>/sohl_debug.log in a
 * monolog (PHP) compatible format:
 *
 *   [YYYY-MM-DD HH:MM:SS] game.LEVEL: message
 *
 * Log level is controlled at runtime by the cvar "sohl_log_level":
 *   0 = DEBUG    (most verbose)
 *   1 = INFO
 *   2 = WARNING  (default)
 *   3 = ERROR
 *   4 = CRITICAL (least verbose)
 *
 * Usage:
 *   #include "logger.h"
 *
 *   LOG_DEBUG("Spawning monster %s at (%.0f, %.0f, %.0f)", name, x, y, z);
 *   LOG_INFO("Player %s connected", name);
 *   LOG_WARNING("Missing model \"%s\", using fallback", path);
 *   LOG_ERROR("NULL pointer in %s", __func__);
 *   LOG_CRITICAL("Fatal save/restore error in entity %d", id);
 */

#pragma once

#include <cstdio>
#include <ctime>

/**
 * @brief Log severity levels (compatible with monolog channel names).
 */
enum class LogLevel : int
{
	DEBUG    = 0,
	INFO     = 1,
	WARNING  = 2,
	ERR      = 3, // ERROR conflicts with a Windows macro
	CRITICAL = 4,

	COUNT // Sentinel — keep last
};

/**
 * @brief Returns the monolog-style string name for a log level.
 */
inline const char* LogLevelName(LogLevel level)
{
	switch (level)
	{
	case LogLevel::DEBUG:    return "DEBUG";
	case LogLevel::INFO:     return "INFO";
	case LogLevel::WARNING:  return "WARNING";
	case LogLevel::ERR:      return "ERROR";
	case LogLevel::CRITICAL: return "CRITICAL";
	default:                 return "UNKNOWN";
	}
}

// Forward-declare the cvar type to avoid a full engine header pull-in.
struct cvar_s;
typedef struct cvar_s cvar_t;

/**
 * @brief System-wide logger singleton.
 *
 * Call Init() in GameDLLInit() once the engine is ready (so GET_GAME_DIR
 * is available). Call Shutdown() in GameDLLShutdown() to flush and close
 * the file handle.
 */
class CLogger
{
public:
	CLogger();
	~CLogger();

	/**
	 * @brief Open the log file in the mod directory.
	 *
	 * Creates (or appends to) <gamedir>/sohl_debug.log. Should be called
	 * from GameDLLInit() after the engine has been initialised.
	 *
	 * @param pLevelCvar  Pointer to the sohl_log_level cvar.  The cvar's
	 *                    integer value is read each time Write() is called
	 *                    so the level can be changed at runtime without
	 *                    restarting the server.
	 */
	void Init(cvar_t* pLevelCvar);

	/**
	 * @brief Flush and close the log file.
	 * Safe to call even if Init() was never called.
	 */
	void Shutdown();

	/**
	 * @brief Write a log entry if @p level is >= the current minimum level.
	 *
	 * The entry is written to the log file and, for levels >= WARNING, also
	 * forwarded to the engine's ALERT() mechanism so it appears on the
	 * server console.
	 *
	 * @param level    Severity of the message.
	 * @param channel  Short subsystem tag, e.g. "game", "movewith", "sound".
	 * @param fmt      printf-style format string.
	 */
	void Write(LogLevel level, const char* channel, const char* fmt, ...)
#ifdef __GNUC__
		__attribute__((format(printf, 4, 5)))
#endif
		;

	/** @brief Return true if the logger has been successfully initialised. */
	bool IsInitialized() const { return m_pFile != nullptr; }

	// Non-copyable
	CLogger(const CLogger&) = delete;
	CLogger& operator=(const CLogger&) = delete;

private:
	FILE*    m_pFile;
	cvar_t*  m_pLevelCvar;

	/** @brief Resolve the current minimum log level from the cvar. */
	LogLevel GetCurrentLevel() const;
};

/** @brief Global logger instance — defined in logger.cpp. */
extern CLogger g_Logger;

// ---------------------------------------------------------------------------
// Convenience macros
// ---------------------------------------------------------------------------
// Each macro passes a subsystem channel of "game".  If you want a custom
// channel per subsystem, call g_Logger.Write() directly.

#define LOG_DEBUG(fmt, ...)    g_Logger.Write(LogLevel::DEBUG,    "game", fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)     g_Logger.Write(LogLevel::INFO,     "game", fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...)  g_Logger.Write(LogLevel::WARNING,  "game", fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)    g_Logger.Write(LogLevel::ERR,      "game", fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) g_Logger.Write(LogLevel::CRITICAL, "game", fmt, ##__VA_ARGS__)
