# Agent Guide

This document provides context for AI coding agents working on this repository.

## Project Overview

This is **Half-Life: Opposing Force Updated** with **Spirit of Half-Life (SOHL)** modifications. It is a C/C++ game SDK based on the GoldSrc (Half-Life 1) engine, providing an updated and bug-fixed version of the Opposing Force expansion pack source code.

The project targets **32-bit x86** platforms (Windows and Linux).

## Repository Structure

```
cl_dll/          - Client-side DLL source code (HUD, rendering, input, VGUI)
dlls/            - Server-side DLL source code (game logic, monsters, weapons, maps)
  ctf/           - Capture The Flag game mode
  rope/          - Rope entity system
  weapons/       - Weapon-specific code
common/          - Shared headers between client and server
engine/          - Engine API headers
external/        - Third-party libraries and headers
fgd/             - Forge Game Data files for level editors
game_shared/     - Code shared between client and server (voice, VGUI utilities, bot)
lib/             - Precompiled libraries
linux/           - Linux Makefiles and build output
network/         - Network delta definitions
pm_shared/       - Player movement prediction code (shared client/server)
projects/        - Visual Studio solution and project files
  vs2019/        - Visual Studio 2019+ solution
public/          - Public engine headers
utils/           - Utility tools (studiomdl, etc.)
docs/            - Documentation
  tutorials/     - Tutorials
```

## Build Instructions

### Windows

Requirements: Visual Studio 2019 or newer.

```
msbuild projects/vs2019/projects.sln -t:rebuild -property:Configuration=Release -maxcpucount:2
```

Output:
- `projects/vs2019/Release/hl_cdll/client.dll` (client DLL)
- `projects/vs2019/Release/hldll/hl.dll` (server DLL)

### Linux

Requirements: GCC 9+ or Clang with C++17 support, `g++-multilib`, `libgl1-mesa-dev`.

```bash
sudo apt install -y g++-multilib clang libgl1-mesa-dev
cd linux
make COMPILER=g++ CFG=release -j$(nproc)
```

Output:
- `linux/release/client.so` (client DLL)
- `linux/release/hl.so` (server DLL)

## CI/CD

The GitHub Actions workflow (`.github/workflows/ci-cd.yml`) builds on every push and pull request:
- **Linux-x86**: Builds with both `g++` and `clang++`
- **Win32**: Builds with MSBuild (Visual Studio)

## Coding Conventions

### Formatting (`.clang-format`)

- Based on LLVM style with Allman (opening brace on new line) brace style
- **Indentation**: Tabs, width 4
- **No column limit** (`ColumnLimit: 0`)
- Pointer alignment: Left (`int* ptr`)
- Includes are **not** sorted automatically
- Short blocks, case labels, and functions allowed on a single line

### Linting (`.clang-tidy`)

Only two checks are enabled:
- `readability-delete-null-pointer` — flags unnecessary `delete` on null pointer
- `readability-implicit-bool-conversion` — flags implicit bool conversions (pointer conditions allowed)

### Naming

- Classes use `C` prefix (e.g., `COFAllyMonster`, `CDrillSergeant`, `CRecruit`)
- Header guards and macros use uppercase with underscores
- Member functions use PascalCase
- The codebase follows original Half-Life SDK naming conventions

### General Style

- C++ with C-style patterns common in GoldSrc SDK code
- `#include` order follows existing file patterns — do not reorder
- Extensive use of inheritance hierarchies for monsters and weapons
- Game entity classes inherit from `CBaseEntity` / `CBaseMonster` / `CSquadMonster` etc.

## Key Files

| File | Purpose |
|------|---------|
| `dlls/cbase.h` | Base entity class definitions |
| `dlls/monsters.h` | Monster-related declarations |
| `dlls/weapons.h` | Weapon base classes and declarations |
| `dlls/player.h` | Player class definition |
| `dlls/game.h` | Game-wide CVARs and settings |
| `dlls/skill.h` | Skill/difficulty level definitions |
| `dlls/util.h` | Server-side utility functions and macros |
| `dlls/movewith.h` | MoveWith entity parenting system (SoHL) |
| `dlls/movewith.cpp` | MoveWith core logic (SoHL) |
| `dlls/alias.h` | Alias entity system (SoHL) |
| `dlls/alias.cpp` | Alias entity implementations (SoHL) |
| `dlls/locus.h` | Locus position/velocity/ratio system (SoHL) |
| `dlls/locus.cpp` | Locus system implementations (SoHL) |
| `cl_dll/hud.h` | Client HUD class definition |
| `cl_dll/particlemgr.h` | SoHL aurora particle system manager (SoHL) |
| `cl_dll/particlemgr.cpp` | Particle system manager implementation (SoHL) |
| `pm_shared/pm_shared.h` | Shared player movement code |

## SoHL Integration Status

Spirit of Half-Life features are being integrated in phases. See [`INTEGRATION_PLAN.md`](INTEGRATION_PLAN.md) for full details and [`sohl.md`](sohl.md) for the comprehensive feature guide.

| Phase | Description | Status |
|-------|-------------|--------|
| **Phase 1** | Core Infrastructure (MoveWith, Think/NextThink, State) | ✅ Complete |
| **Phase 2** | Base Entity Enhancements (USE types, Alias, Locus, Triggers) | ✅ Complete |
| **Phase 2.5** | Entity USE_TYPE Compliance (ShouldToggle for toggle entities) | ✅ Complete |
| **Phase 3A** | Monster/NPC Enhancements | 🔧 Partial |
| **Phase 3B** | Door Enhancements (`SF_DOOR_FORCETOUCHABLE`, synched fire, `message` target) | ✅ Complete |
| **Phase 3C** | Button Enhancements (`game_state` entity, `SF_BUTTON_ONLYDIRECT`, master lock) | ✅ Complete |
| **Phase 3D** | Breakable Enhancements (respawn, `whenhit`, `SF_PUSH_NOPULL`) | ✅ Complete |
| **Phase 3E** | Platform/Train Enhancements (`UTIL_SetVelocity`, `SF_TRACKTRAIN_NOYAW`, non-crushing) | 🔧 Partial |
| **Phase 3F** | Func_Tank Enhancements (new flags, `m_iszFireMaster`, `m_iCrosshair`) | 🔧 Partial |
| **Phase 3G** | Light Enhancements (`env_dlight`, `trigger_lightstyle`, `SetStyle`) | ✅ Complete |
| **Phase 3H** | Sound Enhancements (`m_pPlayFrom`, `trigger_sound`) | 🔧 Partial |
| **Phase 3I** | Scripted Sequence Enhancements (`scripted_action`, repeat/priority, `InitIdleThink`) | ✅ Complete |
| **Phase 3J** | Effects Enhancements (`WaitForStart`, `m_fCurSpeed`, `GetTripEntity`) | 🔧 Partial |
| **Phase 4A** | Fog System (`env_fog`, `SetFog` message, client-side fog data) | ✅ Complete |
| **Phase 4B** | Sky System (`env_sky`, `SetSky` message, client-side sky data) | ✅ Complete |
| **Phase 4C** | Custom HUD Color (`m_iHUDColor`, `HUDColor` message) | ✅ Complete |
| **Phase 4D** | Shiny/Reflective Surfaces (`kRenderFxReflection`, `CShinySurface`, `AddShine`) | ✅ Complete |
| **Phase 4E** | Dynamic Lighting (`KeyedDLight` message, `env_dlight` client integration) | ✅ Complete |
| **Phase 4F** | Model/Animation Scaling (`StudioModelRenderer` scale, `GetSequenceFrames`) | ✅ Complete |
| **Phase 4G** | Particle System (`particlemgr.h/cpp`, `env_particle`, `gmsgParticle`) | ✅ Complete |
| **Phase 5** | New Entity Definitions & Polish | 🔲 Pending |

### SoHL Conventions

- New USE types: `USE_KILL` (4), `USE_SAME` (5), `USE_NOT` (6) — extend the USE_TYPE enum
- Entity states: `STATE_OFF`, `STATE_TURN_ON`, `STATE_ON`, `STATE_TURN_OFF` — used by `GetState()` virtual
- Think wrappers: `SetNextThink(delay)`, `AbsoluteNextThink(time)`, `DontThink()` — never set `pev->nextthink` directly
- `m_hActivator` is in `CBaseDelay` (moved from `CBaseToggle` for wider access)
- MoveWith members: `m_pMoveWith`, `m_pChildMoveWith`, `m_pSiblingMoveWith` — entity parenting linked list
- Locus virtuals: `CalcPosition()`, `CalcVelocity()`, `CalcRatio()` — for dynamic entity position/velocity resolution
- Toggle entities **must** use `ShouldToggle(useType, currentState)` in their `Use()` handler to properly respect `USE_ON`, `USE_OFF`, `USE_TOGGLE`, and the extended types

## Scope of Changes

The following types of changes are **in scope**:
- Bug fixes
- Code improvements (refactoring, generalizing, simplifying)
- Fixing game-breaking bugs in game assets

The following types of changes are **out of scope**:
- Graphical upgrades
- Physics engine changes
- Other engine changes
- Gameplay changes

## Testing

There is no automated test suite. Verify changes by:
1. Ensuring the project **builds successfully** on both Windows (MSBuild) and Linux (Make)
2. Reviewing compiler warnings
3. Manual in-game testing when applicable
