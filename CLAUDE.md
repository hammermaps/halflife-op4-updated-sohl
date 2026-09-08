# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repository is

This is **Half-Life: Opposing Force Updated**, one of the TWHL "Updated" repositories (siblings: `halflife-updated`, `halflife-bs-updated`) that maintain modernized, buildable versions of Valve's Half-Life 1 SDK for the three official HL1-engine games. It builds two native shared libraries per platform: the game/server DLL (`hl.dll` / `hl.so`, from `dlls/`) and the client DLL (`client.dll` / `client.so`, from `cl_dll/` + `game_shared/`). These are loaded by the (closed-source) Half-Life engine — this repo does not contain or build the engine itself.

### Scope policy (important for reviewing/proposing changes)

Per `README.md`, in scope: bug fixes, and code-quality improvements (refactoring/generalizing/simplifying) that don't change observable behavior or require a full redesign — this must stay approachable for modders. Out of scope: graphical upgrades, physics engine changes, other engine changes, gameplay changes. Keep this in mind before suggesting "improvements" — a change that alters gameplay or requires re-architecting a system is likely unwanted here even if it's technically nicer.

### Fork goal: Hybrid AI Core (branch `ext-ki`)

This fork carries one additional, deliberate exception to the "no gameplay changes" rule above: a **hybrid Utility-AI decision layer** for NPC tactical behavior in `dlls/`, specified in detail in [`AGENTS_HYBRID_AI.md`](AGENTS_HYBRID_AI.md) — read that file before touching monster/AI code on this branch. Key constraints from that spec that apply to any AI work here:
- Keep the existing GoldSrc/HLSDK execution infrastructure (schedules, tasks, nodegraph, navigation, animation, `scripted_sequence`, save/restore, relationships, `EHANDLE`, entity lifecycle) intact. The new layer decides **what** an NPC wants to do; existing HLSDK code still decides **how**.
- Never delete original per-class `GetSchedule()` logic — it must remain as a fallback when the hybrid AI is disabled, uncertain, or fails.
- Gate all new behavior behind CVARs (master switch `ai_hybrid`, plus feature-specific gates); behavior must be unchanged when disabled.
- Roll out per NPC class in the phase order the spec defines (`CHGrunt` first as the reference implementation, then other HECU/friendly classes, then aliens/scientists/Barney-likes) — don't refactor unrelated monster classes as a side effect.
- Respect the explicit "do not" list in the spec (no behavior-tree/GOAP frameworks, no navmesh, no external AI/LLM runtime, no entity-system rewrite, no removing `scripted_sequence` or bypassing the nodegraph).

#### Current status (check this first on a new session)

- **Implemented and playtest-verified (2026-09-08):**
  - [`docs/designs/hybrid-ai-core-phase-a.md`](docs/designs/hybrid-ai-core-phase-a.md) — framework scaffolding (`dlls/ai_hybrid_core.h/.cpp`, `dlls/ai_hybrid.h/.cpp`, `ai_hybrid`/`ai_hybrid_debug` CVars). Zero behavior change while `ai_hybrid` is 0.
  - [`docs/designs/hybrid-ai-core-phase-b.md`](docs/designs/hybrid-ai-core-phase-b.md) — first real tactical logic for `CHGrunt`: enemy-memory confidence decay, utility scoring for ATTACK/COVER/SEARCH, hysteresis, and a narrow `GetSchedule()` integration that only takes over "plain ongoing combat" (never repel/grenade-danger/first-contact/no-ammo/flinch states). Confirmed working in-game on `c2a5f` (19 hostile grunts) with `ai_hybrid 1`.
  - [`docs/designs/hybrid-ai-core-activity-logger.md`](docs/designs/hybrid-ai-core-activity-logger.md) — JSON-Lines decision log (`ai_hybrid_log` CVar), independent of the console debug output. Confirmed working alongside the Phase B playtest.
- **Not started:** Phase C (squad-shared enemy memory + squad blackboard/roles, `AGENTS_HYBRID_AI.md` §19-20) — the next planned chunk of work. Read the Phase B design doc's Open Questions before starting it (score-tuning is still first-pass, not yet revisited against real playtest data).
- **Standing decisions** (durable, don't re-litigate without a reason):
  - `ext-ki` stays fully independent from `master` — no rebase/merge planned, even though `master` is actively diverging (Spirit of Half-Life backport work) in overlapping monster files. This was an explicit user override of a rebase recommendation.
  - The AI logic is split into an engine-free POD core (`ai_hybrid_core.*`, no HLSDK includes, unit-tested standalone via `scripts/run-ai-hybrid-core-tests.sh` / `make check`) plus a thin engine-aware adapter (`ai_hybrid.*`) — keep new tactical logic in the core, not the adapter, unless it genuinely needs engine state.
  - `GetSchedule()` integrations must stay narrow (see the Phase B doc's decision #2) — never wrap a whole monster's `GetSchedule()` in a top-level hybrid check; gate out every safety-critical/scripted state explicitly instead.

## Build commands

### Linux
```
cd linux
make COMPILER=g++ CFG=release -j8      # or COMPILER=clang++
```
- Requires GCC 9+/Clang with C++17, `g++-multilib`, `libgl1-mesa-dev`.
- Builds 32-bit (`-m32`) binaries via `Makefile.hldll` (game) and `Makefile.hl_cdll` (client), driven by the top-level `linux/Makefile`.
- `CFG=release` (default) or `CFG=debug`.
- Output goes to `linux/release/` (or `linux/debug/`): `hl.so`, `client.so`, plus `.dbg` symbol files.
- There is no test suite and no lint target — CI (`.github/workflows/ci-cd.yml`) simply builds Linux (g++ and clang++) and Windows (MSBuild) and uploads the resulting binaries. Treat "does it build" as the correctness bar for CI purposes.

### Windows
- Visual Studio 2019/2022, solution: `projects/vs2019/projects.sln` (game+client) and `projects/vs2019/utils.sln` (asset tools in `utils/`).
- CI builds via `msbuild projects/vs2019/projects.sln -t:rebuild -property:Configuration=Release`.

### Formatting
- `.clang-format` defines the house style (LLVM-based, 4-space indent via `AccessModifierOffset: -4`, Allman-ish custom brace wrapping, `ColumnLimit: 0`). Run `clang-format` on touched files rather than hand-matching style.
- `.clang-tidy` only enables `readability-delete-null-pointer` and `readability-implicit-bool-conversion` — don't expect broader static analysis from it.

### Packaging (for producing a distributable mod)
```
dotnet script halflife_updated/scripts/packager/CreatePackage.csx --package-name <name>
```

### Dev helper scripts (`scripts/`)
`scripts/build.sh` / `scripts/rebuild.sh` wrap the Linux `make` invocation above for faster iteration. `scripts/save-baseline.sh` snapshots the current build's `.so` files (with checksums) into `reference-builds/<git-sha>-<cfg>/` (gitignored, local-only); `scripts/diff-baseline.sh <dir>` hashes a later build against a saved one — a byte-identical `.so` is strong evidence a change didn't alter behavior at the binary level (useful for verifying "no behavior change while `ai_hybrid` is disabled" claims from the Phase A design doc). `scripts/deploy-test-mod.sh` copies a build into the locally prepared Steam test-mod install (`~/.var/app/com.valvesoftware.Steam/.../halflife_op4_updated_ki` by default, override with `MOD_DIR=`) for manual in-game verification — this repo checkout is not itself a runnable game install, that separate mod directory is. `scripts/debug-test-mod.sh` runs it under `gdb` (must be launched from a real terminal — a Flatpak Steam quirk breaks `SteamAPI_Init()` for backgrounded/detached launches). `scripts/build-test-map.sh` compiles a `mapsrc/*.map` with the local VHLT toolchain into a `.bsp` and deploys it — `mapsrc/ai_test_grunt.map` is a minimal room with a `monster_human_grunt` for exercising AI debug output without a full campaign map. See `scripts/README.md` (including a GoldSrc `.map` gotcha: brush planes need **inward**-pointing normals, not outward).

## Architecture

### Two DLLs, one engine contract
The engine loads two modules and talks to them entirely through C-style interface structs defined in `engine/eiface.h` / `engine/APIProxy.h` and `common/`:
- **Game (server) DLL** — `dlls/`. Owns all entity logic, AI, weapons, game rules. Entry points in `dlls/h_export.cpp`, `dlls/client.cpp`.
- **Client DLL** — `cl_dll/`. Owns HUD, view/camera, client-side prediction hooks, VGUI menus. Entry points in `cl_dll/cdll_int.cpp`, exported via `cl_dll/Exports.h`.
- **`pm_shared/`** — player movement code (`pm_shared.cpp`) that is compiled into *both* DLLs so client-side prediction and server simulation stay bit-compatible. Never assume this code runs on only one side.
- **`game_shared/`** — code genuinely shared and compiled into both game and client DLLs (VGUI widget helpers, voice chat, bot support under `game_shared/bot/`, filesystem helpers).
- **`common/`, `engine/`, `public/`** — read-only-in-spirit SDK headers/types (network structs, `mathlib`, studio model format, engine interfaces). Changes here ripple into both DLLs and must stay ABI-compatible with the engine.
- **`external/SDL2/`** — vendored SDL2 headers used by the client for windowing/input on Linux.

### Entity/game framework (`dlls/`)
- Everything derives from `CBaseEntity` (`dlls/cbase.h`/`.cpp`); monsters derive further through `CBaseMonster` → `CBaseAnimating` (`animating.cpp`) → `basemonster.h`, with squad/talk/ally-specific mixins (`squadmonster.cpp`, `talkmonster.cpp`, `COFSquadTalkMonster.*`, `COFAllyMonster.*`).
- AI uses schedule/task system (`schedule.cpp/.h`, `scripted.cpp`, `defaultai.cpp`, `monsterstate.cpp`) plus a shared node graph (`nodes.cpp`) and sound-based sensing (`soundent.cpp`).
- Weapons live in `dlls/weapons/` (server-side logic, mostly Opposing Force-specific weapons like `CDisplacer`, `CEagle`, `CGrapple`, `CPipewrench`) plus shared weapon base code in `dlls/weapons.cpp`/`weapons_shared.cpp` (compiled into both DLLs for prediction).
- Game rules (singleplayer/multiplayer/teamplay/coop/CTF) are selected via `dlls/gamerules.*` subclasses; Opposing Force's CTF mode lives under `dlls/ctf/`.
- `dlls/rope/` implements the physics rope entity used by some OpFor content.
- Monster source files at repo root of `dlls/` are one-file-per-monster (e.g. `hgrunt.cpp`, `bullsquid.cpp`, `gonome.cpp`) — when fixing monster behavior, the relevant logic is almost always self-contained in that single `.cpp`/matching `.h`.

### Client framework (`cl_dll/`)
- HUD elements are individual `.cpp` files registered with the HUD system (`hud.cpp`/`hud.h`, `hud_redraw.cpp`, `hud_update.cpp`); each HUD element (ammo, health, geiger counter, flashlight, etc.) is its own file.
- VGUI-based menus/panels are `vgui_*.cpp` (scoreboard, team menu, spectator panel, MOTD, class menu for team games).
- `StudioModelRenderer.cpp` / `GameStudioModelRenderer.cpp` handle model rendering; `view.cpp`/`tri.cpp` handle the render view and triangle API hooks.
- Prediction-related weapon code (`com_weapons.cpp`, `ev_hldm.cpp`, `ev_common.cpp`, `eventscripts.h`) mirrors/consumes the shared weapon logic from `dlls/weapons_shared.cpp` and `pm_shared/`.

### Utilities (`utils/`)
Standalone asset-pipeline tools (map compilers `qcsg`/`qbsp2`/`qrad`/`vis`, `studiomdl` model compiler, `sprgen`, `mdlviewer`, etc.), each with a matching `.vcxproj` in `projects/vs2019/`. These are separate from the game/client build and only relevant when touching content tooling, not gameplay code.

## Patch export (project directive)

Whenever a working, verified change is completed (built successfully, and behaviorally checked as far as possible), export it as a patch file into `patches/` for later transfer to sibling projects — see `patches/README.md` for the exact naming convention and `git format-patch`/`git am` commands. Do this in addition to the normal commit, not instead of it.

## Language policy (project directive)

- **Chat output to the user**: always German ("Ausgabe ist immer in Deutsch").
- **Code comments**: always English.
- **README content, code descriptions, and commit messages**: always English.

This applies regardless of which language the user writes their request in.

## Working conventions
- The codebase as a whole has no automated tests; verification is "it builds" plus manual in-game testing, which Claude generally cannot perform. Be explicit that behavioral changes are unverified beyond compilation. **Exception (decided 2026-09-08, `ext-ki` only):** new Hybrid AI Core logic should get smoke/unit tests where technically feasible — see the "Current status" note above and `docs/designs/hybrid-ai-core-phase-a.md` for how (an engine-free core compiled and tested standalone, no engine headers, no DLL link).
- `-fno-exceptions` is used on Linux builds — do not introduce C++ exceptions into shared/game/client code.
- The project intentionally preserves original-game code duplication across `dlls/` (e.g. near-identical logic across monster files or weapon variants) for parity with the shipped game and to keep mod integration simple; don't unilaterally deduplicate across unrelated entities as a drive-by change.

## gstack (recommended)

This project uses [gstack](https://github.com/garrytan/gstack) for AI-assisted workflows.
Install it for the best experience:

```bash
git clone --depth 1 https://github.com/garrytan/gstack.git ~/.claude/skills/gstack
cd ~/.claude/skills/gstack && ./setup --team
```

Skills like /qa, /ship, /review, /investigate, and /browse become available after install.
Use /browse for all web browsing. Use ~/.claude/skills/gstack/... for gstack file paths.

## Skill routing

When the user's request matches an available skill, invoke it via the Skill tool. When in doubt, invoke the skill.

Key routing rules:
- Product ideas/brainstorming → invoke /office-hours
- Strategy/scope → invoke /plan-ceo-review
- Architecture → invoke /plan-eng-review
- Design system/plan review → invoke /design-consultation or /plan-design-review
- Full review pipeline → invoke /autoplan
- Bugs/errors → invoke /investigate
- QA/testing site behavior → invoke /qa or /qa-only
- Code review/diff check → invoke /review
- Visual polish → invoke /design-review
- Ship/deploy/PR → invoke /ship or /land-and-deploy
- Save progress → invoke /context-save
- Resume context → invoke /context-restore
- Author a backlog-ready spec/issue → invoke /spec
