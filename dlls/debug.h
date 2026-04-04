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
 * @file debug.h
 * @brief Centralized debug, logging, and profiling infrastructure for Half-Life SDK.
 *
 * Usage:
 *   #include "debug.h"
 *
 * Debug logging (compiled out in release):
 *   DBG_LOG("Monster %s spawned at (%f, %f, %f)", name, x, y, z);
 *   DBG_WARN("Unexpected state %d", state);
 *   DBG_ERROR("NULL pointer in %s", __func__);
 *
 * Profiling (compiled out in release):
 *   void ExpensiveFunction()
 *   {
 *       PROFILE_SCOPE("ExpensiveFunction");
 *       // ... work ...
 *   }
 *
 * Conditional debug (always available, controlled by cvar at runtime):
 *   DEV_LOG(mw_debug, "MoveWith offset: %f", offset);
 */

#pragma once

#include "Platform.h"
#include "enginecallback.h"
#include "logger.h"

/**
 * @brief Compile-time debug logging macros.
 * These are completely stripped in release builds for zero overhead.
 * In debug builds they route through the system-wide logger so output
 * goes both to the console and to sohl_debug.log.
 */
#ifdef DEBUG

#define DBG_LOG(fmt, ...) \
	LOG_DEBUG(fmt, ##__VA_ARGS__)

#define DBG_WARN(fmt, ...) \
	g_Logger.Write(LogLevel::WARNING, "game", "%s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

#define DBG_ERROR(fmt, ...) \
	g_Logger.Write(LogLevel::ERR, "game", "%s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

#else // !DEBUG

#define DBG_LOG(fmt, ...) ((void)0)
#define DBG_WARN(fmt, ...) ((void)0)
#define DBG_ERROR(fmt, ...) ((void)0)

#endif // DEBUG


/**
 * @brief Runtime-controlled debug logging via cvar.
 * Always compiled in, but only prints when the given cvar is non-zero.
 * Useful for debugging specific subsystems without recompiling.
 *
 * Example:
 *   extern cvar_t mw_debug;
 *   DEV_LOG(mw_debug, "MoveWith child count: %d", count);
 */
#define DEV_LOG(cvar, fmt, ...)                                         \
	do                                                                  \
	{                                                                   \
		if ((cvar).value != 0)                                          \
			g_Logger.Write(LogLevel::DEBUG, (cvar).name, fmt, ##__VA_ARGS__); \
	} while (0)


/**
 * @brief Simple RAII-based scope profiler for debug builds.
 *
 * Measures wall-clock time spent in a scope and prints it to the console.
 * Completely compiled out in release builds for zero overhead.
 *
 * Usage:
 *   void MyFunction()
 *   {
 *       PROFILE_SCOPE("MyFunction");
 *       // ... work ...
 *   }
 *   // Prints: [PROFILE] MyFunction: 1.234 ms
 */
#ifdef DEBUG

#include <chrono>

class CScopeProfiler
{
public:
	explicit CScopeProfiler(const char* name)
		: m_pszName(name)
		, m_start(std::chrono::high_resolution_clock::now())
	{
	}

	~CScopeProfiler()
	{
		const auto end = std::chrono::high_resolution_clock::now();
		const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start);
		const float ms = static_cast<float>(duration.count()) / 1000.0f;
		ALERT(at_console, "[PROFILE] %s: %.3f ms\n", m_pszName, ms);
	}

	// Non-copyable
	CScopeProfiler(const CScopeProfiler&) = delete;
	CScopeProfiler& operator=(const CScopeProfiler&) = delete;

private:
	const char* m_pszName;
	std::chrono::high_resolution_clock::time_point m_start;
};

#define PROFILE_SCOPE(name) CScopeProfiler _profiler_##__COUNTER__(name)

#else // !DEBUG

#define PROFILE_SCOPE(name) ((void)0)

#endif // DEBUG
