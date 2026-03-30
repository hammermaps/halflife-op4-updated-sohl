# Spirit of Half-Life 1.2 — Integration Plan

This document outlines a phased plan to integrate all Spirit of Half-Life 1.2 (SoHL/LRC) features described in [`sohl.md`](sohl.md) into the Half-Life: Opposing Force Updated codebase.

## Current Status

**Phase 1 (Core Infrastructure)** — ✅ **COMPLETE**
- MoveWith system, Think/NextThink wrappers, and State system are fully implemented.
- New files: `dlls/movewith.h`, `dlls/movewith.cpp`

**Phase 2 (Base Entity Enhancements)** — ✅ **COMPLETE**
- New USE types (USE_KILL, USE_SAME, USE_NOT), Alias system, Locus system, and Trigger enhancements.
- New files: `dlls/alias.h`, `dlls/alias.cpp`, `dlls/locus.h`, `dlls/locus.cpp`
- New entities: `multi_watcher`, `trigger_command`, `trigger_changecvar`, `trigger_inout`, `trigger_bounce`, `trigger_onsight`, `trigger_startpatrol`, `trigger_motion`, `motion_manager`, `render_fx_fader`

**Phase 2.5 (Entity USE_TYPE Compliance)** — ✅ **COMPLETE**
- Added `ShouldToggle` to toggle entities that previously ignored `USE_TYPE`: doors, trains, sprite trains, rotating brushes, pendulums, conveyors, toggle triggers, player freeze.
- Updated `CMultiSource` to respect `USE_ON`/`USE_OFF`.
- Simplified `CSpeaker::ToggleUse` to use `ShouldToggle`.

**Phase 3 (Entity-Specific Enhancements)** — 🔧 **IN PROGRESS**
- **Phase 3A (Monster/NPC Enhancements)** — ✅ **COMPLETE**
  - Header declarations: `m_iClass`, `m_iPlayerReact`, `HasCustomGibs()`, `SF_MONSTER_NO_YELLOW_BLOBS`, `SF_MONSTER_NO_WPN_DROP`, custom gib `SpawnRandomGibs` overload
  - Custom model support for all HL1 monster files (agrunt, apache, barnacle, barney, bigmomma, bloater, bullsquid, controller, gargantua, genericmonster, gman, hassassin, headcrab, hgrunt, houndeye, ichthyosaur, islave, leech, nihilanth, osprey, rat, roach, scientist, turret, zombie)
  - Custom model support for all OpFor monster files (voltigore, baby_voltigore, pitdrone, gonome, shocktrooper, geneworm, pitworm, otis, drillsergeant, recruit, cleansuit_scientist, blkop_osprey, blkop_apache, hgrunt_medic, hgrunt_torch, male_assassin, hfgrunt, shockroach, loader, skeleton)
  - `m_iClass` allegiance override in `Classify()` for all HL1 monsters (agrunt, apache, barnacle, barney, bigmomma, bloater, bullsquid, controller, gargantua, genericmonster, gman, hassassin, headcrab, houndeye, ichthyosaur, islave, leech, nihilanth, osprey, roach, scientist/CSittingScientist, hgrunt, turret, zombie) and all OpFor monsters
  - Core monster logic: `m_iClass`/`m_iPlayerReact` KeyValue handlers, stuck warning with `SF_MONSTER_NO_YELLOW_BLOBS`, `info_monster_goal` entity
  - Talk monster: `m_iszDecline`/`m_iszSpeakAs` save/restore + KeyValue, `DeclineFollowing()`; SpeakAs voice group replacement in `Precache()`
  - Combat: custom gib model support via `HasCustomGibs()`, `studio.h` include
  - Scripted: `PreciseAttack()` implementation, `TaskComplete()` for immediate play start
  - Monster-specific: hornet allegiance (agrunt), `monster_bullsquid` link, `SF_MONSTER_NO_WPN_DROP` (hgrunt, hfgrunt, shocktrooper, hgrunt_medic, hgrunt_torch, male_assassin, otis), `m_iPlayerReact` override (islave + barney), osprey loop breaker + unit fallback, postdisaster sitting scientist, `m_iBaseBody` tracking (barney)
  - OpFor weapon entities use SoHL think wrappers (`SetNextThink`/`DontThink`)
  - OpFor rope and CTF entities use SoHL think wrappers where applicable
- **Phase 3B (Door Enhancements)** — ✅ **COMPLETE**
  - `SF_DOOR_FORCETOUCHABLE`: doors can now be touched even when named or use-only
  - Synched target firing: `USE_ON` fires when door starts to open, `USE_OFF` when door starts to close
  - `pev->message` as open target, `pev->netname` as close target
- **Phase 3C (Button Enhancements)** — ✅ **COMPLETE**
  - `SF_BUTTON_ONLYDIRECT` (16): button can't be used through walls (flag defined)
  - `game_state` entity: simple state-tracking entity for watchers
  - Touch-only buttons can now also be triggered via Use()
  - `CMomentaryRotButton::Use` checks `IsLockedByMaster()`
- **Phase 3D (Breakable Enhancements)** — ✅ **COMPLETE**
  - Save/restore for `m_flRespawnTime`, `m_flRespawnHealth`, `m_iszWhenHit`
  - KeyValue handlers for `respawn` (time) and `whenhit` (locus trigger)
  - `RespawnThink()`: automatically respawn breakable after `m_flRespawnTime` seconds
  - `m_iszWhenHit` fires locus target when breakable takes damage
  - `SF_PUSH_NOPULL` (256): pushable entity cannot be pulled
- **Phase 3E (Platform/Train Enhancements)** — 🔧 **PARTIALLY COMPLETE**
  - `SF_TRACKTRAIN_NOYAW`/`SF_TRACKTRAIN_AVELOCITY`/`SF_TRACKTRAIN_AVEL_GEARS` flags defined
  - `m_vecMasterAvel`/`m_vecBaseAvel` members declared and save/restored
  - `DesiredAction()` override implemented (calls `Next()`)
  - `UTIL_SetVelocity`/`UTIL_SetAvelocity` used in `Next()` and `Use()`
  - `SF_TRACKTRAIN_NOYAW` respected: skip yaw adjustment when flag is set
  - Non-crushing trains: `pev->dmg == -1` skips damage in `Blocked()`
  - `movewith.h` included
- **Phase 3F (Func_Tank Enhancements)** — 🔧 **PARTIALLY COMPLETE**
  - New flags: `SF_TANK_LASERSPOT`, `SF_TANK_MATCHTARGET`, `SF_TANK_SEQFIRE`
  - New members: `m_iszFireMaster`, `m_iCrosshair` with save/restore and KeyValue handlers
- **Phase 3G (Light Enhancements)** — ✅ **COMPLETE**
  - `GetStyle()`/`SetStyle(int)`/`SetCorrectStyle()` methods added to `CLight`
  - `m_iszCurrentStyle` member with save/restore
  - `CLightDynamic` (`env_dlight`): dynamic entity light creator
  - `CTriggerLightstyle` (`trigger_lightstyle`): temporarily changes a light's pattern
- **Phase 3H (Sound Enhancements)** — 🔧 **PARTIALLY COMPLETE**
  - `m_pPlayFrom` and `m_iChannel` members added to `CAmbientGeneric` with save/restore
  - Defaults: `m_pPlayFrom = edict()`, `m_iChannel = CHAN_STATIC`
  - `trigger_sound` stub entity registered
- **Phase 3I (Scripted Sequence Enhancements)** — ✅ **COMPLETE**
  - `scripted_action` and `aiscripted_sequence` both linked to `CCineMonster`
  - Full save/restore for: `m_iState`, `m_iszAttack`, `m_iszMoveTarget`, `m_iRepeats`, `m_iRepeatsLeft`, `m_fRepeatFrame`, `m_iPriority`
  - KeyValue handlers for `attack`, `movetarget`, `repeats`, `priority`
  - `InitIdleThink()` implemented: resets state and restarts idle sequence
- **Phase 3J (Effects Enhancements)** — 🔧 **PARTIALLY COMPLETE**
  - `bmodels.cpp`: `WaitForStart()` function for `CFuncRotating`; `m_fCurSpeed` member with save/restore
  - `effects.cpp`: `CLaser::GetTripEntity()` implemented for tripbeam support

**Phase 4 (Visual & Client-Side Features)** — ✅ **COMPLETE**
- Phase 4A (Fog System) — ✅ **COMPLETE**
  - Server: `CEnvFog` entity class in `effects.cpp` with save/restore, KeyValue, Use toggle, `SendFog()`
  - Server: `gmsgSetFog` message registered in `UserMessages.h/cpp`
  - Client: `MsgFunc_SetFog` handler stores fog color, start/end distance, density
  - Client: Fog data members in `CHud` (`m_iFogColor_R/G/B`, `m_fStartDist`, `m_fEndDist`, `m_fFogDensity`, `m_bFogOn`)
  - Client: `g_iWaterLevel` global added to `hl_weapons.cpp` for DMC fog support
- Phase 4B (Sky System) — ✅ **COMPLETE**
  - Server: `CEnvSky` entity class in `effects.cpp` with save/restore, Use toggle, `SendSky()`
  - Server: `gmsgSetSky` message registered
  - Client: `MsgFunc_SetSky` handler stores sky mode and position
  - Client: `SKY_OFF`/`SKY_ON` constants, `m_vecSkyPos`, `m_iSkyMode` in `CHud`
- Phase 4C (Custom HUD Color) — ✅ **COMPLETE**
  - Client: `m_iHUDColor` packed-int member added to `CHud`
  - Client: `__MsgFunc_HudColor` now sets both `giR/giG/giB` globals and `gHUD.m_iHUDColor`
  - Note: `gmsgHudColor` was already registered from previous work
- Phase 4D (Shiny/Reflective Surfaces) — ✅ **COMPLETE**
  - Common: `kRenderFxReflection` added to render FX enum in `const.h`
  - Server: `CShine` (`func_shine`) entity in `bmodels.cpp` sends `gmsgAddShine` message
  - Server: `gmsgAddShine` message registered
  - Client: `CShinySurface` class defined in `hud.h`, implemented in `tri.cpp`
  - Client: `MsgFunc_AddShine` handler creates linked list of shiny surfaces
  - Client: Shiny surfaces cleared on level load in `MsgFunc_InitHUD`
- Phase 4E (Dynamic Lighting) — ✅ **COMPLETE**
  - Server: `gmsgKeyedDLight` message registered
  - Server: `CLightDynamic::Use()` extended to send `KeyedDLight` message with position, radius, color
  - Client: `MsgFunc_KeyedDLight` handler creates/destroys persistent dynamic lights via `CL_AllocDlight`
- Phase 4F (Model/Animation Scaling) — ✅ **COMPLETE**
  - Client: `StudioDrawModel()` and `StudioDrawPlayer()` apply `curstate.scale` to rotation matrix
  - Server: `GetSequenceFrames()` added to `animation.cpp`/`animation.h`
- Phase 4G (Particle System) — ✅ **COMPLETE**
  - Client: New files `cl_dll/particlemgr.h` and `cl_dll/particlemgr.cpp` with `ParticleSystemManager` and `ParticleSystem` classes
  - Server: `CEnvParticle` (`env_particle`) entity in `effects.cpp` with save/restore, Use toggle, `SendParticle()`
  - Server: `gmsgParticle` message registered
  - Client: `MsgFunc_Particle` handler manages particle systems
  - Client: Particle systems updated in `HUD_DrawTransparentTriangles`
  - Build: `particlemgr.cpp` added to Linux Makefile and VS2019 project

**Phase 5** — ✅ Complete

## Guiding Principles

1. **Foundation First** — Core infrastructure systems (MoveWith, Think, State) must be implemented before dependent features.
2. **Server Before Client** — Server-side (dlls/) changes are implemented and verified before corresponding client-side (cl_dll/) changes.
3. **Incremental & Testable** — Each phase should compile and run without breaking existing functionality.
4. **Minimal Disruption** — Changes follow existing code style (Allman braces, tabs, `C`-prefixed classes, PascalCase methods).
5. **Save/Restore Integrity** — Every new field must have corresponding `DEFINE_FIELD` entries to prevent save/load corruption.

---

## Dependency Graph

```
Phase 1: Core Infrastructure
  ├── 1A: MoveWith System (foundation for entity parenting)
  ├── 1B: Think/NextThink System (foundation for timing)
  └── 1C: State System (foundation for entity states)
          │
Phase 2: Base Entity Enhancements (depends on Phase 1)
  ├── 2A: Trigger System Enhancements
  ├── 2B: New USE Types (USE_KILL, USE_SAME, USE_NOT)
  ├── 2C: Alias System
  └── 2D: Locus System
          │
Phase 2.5: Entity USE_TYPE Compliance (depends on Phase 2B)
  └── ShouldToggle for all toggle entities (doors, trains, rotating, pendulum, triggers, etc.)
          │
Phase 3: Entity-Specific Enhancements (depends on Phase 1 + 2 + 2.5)
  ├── 3A: Monster/NPC Enhancements
  ├── 3B: Door Enhancements
  ├── 3C: Button Enhancements
  ├── 3D: Breakable Enhancements
  ├── 3E: Platform/Train Enhancements
  ├── 3F: Func_Tank Enhancements
  ├── 3G: Light Enhancements
  ├── 3H: Sound Enhancements
  ├── 3I: Scripted Sequence Enhancements
  └── 3J: Effects Enhancements
          │
Phase 4: Visual & Client-Side Features (depends on Phase 1)
  ├── 4A: Fog System (env_fog + client rendering)
  ├── 4B: Sky System (env_sky + client rendering)
  ├── 4C: Custom HUD Color
  ├── 4D: Shiny/Reflective Surfaces
  ├── 4E: Dynamic Lighting (KeyedDLight)
  ├── 4F: Model/Animation Scaling
  └── 4G: Particle System
          │
Phase 5: New Entity Definitions & Polish (depends on all above)
  ├── 5A: New Entity Definitions (capability flags, classifications)
  └── 5B: Miscellaneous Changes & Debug Tools
```

---

## Phase 1: Core Infrastructure

### Phase 1A: MoveWith System

**Priority**: CRITICAL — Most SoHL features depend on this system.
**Estimated Scope**: ~130 LRC lines, 3 new files, ~10 modified files

The MoveWith system enables entity parenting: any entity can be attached to another and move/rotate with it. It introduces a linked-list of parent/child/sibling relationships and an "assist list" for coordinating physics updates.

#### New Files to Create

| File | Purpose |
|------|---------|
| `dlls/movewith.h` | Flag definitions (`LF_NOTEASY`, `LF_DOASSIST`, etc.), utility function declarations |
| `dlls/movewith.cpp` | Core MoveWith logic: `UTIL_AssignOrigin`, `UTIL_SetVelocity`, `UTIL_SetAvelocity`, `CheckAssistList`, `CheckDesiredList`, loop protection |

#### Files to Modify

| File | Changes |
|------|---------|
| `dlls/cbase.h` | **Member variables**: `m_pMoveWith`, `m_MoveWith`, `m_pChildMoveWith`, `m_pSiblingMoveWith`, `m_vecMoveWithOffset`, `m_vecRotWithOffset`, `m_pAssistLink`, `m_vecPostAssistVel`, `m_vecPostAssistAVel`, `m_iLFlags`, `m_vecSpawnOffset` |
| `dlls/cbase.h` | **Virtual methods**: `DesiredAction()`, `Activate()` override, `PostSpawn()` |
| `dlls/cbase.h` | **Utility functions**: `InitMoveWith()`, moved `UTIL_SetOrigin` declaration |
| `dlls/cbase.h` | **Other**: `CWorld*` global pointer, MoveWith KeyValue handler for `"movewith"` key on all entities, transition logic tied to MoveWith parent |
| `dlls/cbase.cpp` | Add Save/Restore fields for all new members, implement `Activate()`, implement `InitMoveWith()`, rebuild assist list on game start |
| `dlls/client.cpp` | Move AssistList processing to client frame (`CheckAssistList()` call) |
| `dlls/subs.cpp` | Clean up assist list on entity removal, propagate removal to MoveWith children, add `info_movewith` entity |
| `dlls/world.cpp` | Include `movewith.h`, initialize global MoveWith/Assist/Alias lists, set up `g_pWorld` |
| `dlls/bmodels.cpp` | Conditional logic for MoveWith state, bug fix for rotating entity center calculation |
| `dlls/maprules.cpp` | MoveWith support for `game_player_equip` |
| `dlls/triggers.cpp` | Defer trigger think via `UTIL_DesiredAction` |

#### Build Integration

- Add `dlls/movewith.cpp` to the Linux Makefile (`linux/Makefile`)
- Add `dlls/movewith.cpp` to the Visual Studio project (`projects/vs2019/`)

#### Verification

- [x] Project compiles on Linux (g++ and clang++)
- [x] Project compiles on Windows (MSBuild)
- [x] Existing entity behavior is unchanged when no `movewith` key is set

---

### Phase 1B: Think/NextThink System

**Priority**: CRITICAL — Required for accurate timing in MoveWith and many entities.
**Estimated Scope**: ~30 LRC lines, 0 new files, ~4 modified files

Replaces vanilla `pev->nextthink` with a wrapper system that tracks think times independently, preventing the engine from silently overriding scheduled thinks.

#### Files to Modify

| File | Changes |
|------|---------|
| `dlls/cbase.h` | Add `m_fNextThink`, `m_fPevNextThink`, `m_activated` members; add `SetNextThink(float)`, `SetNextThink(float, BOOL)`, `AbsoluteNextThink(float)`, `AbsoluteNextThink(float, BOOL)`, `SetEternalThink()`, `DontThink()`, `ResetThink()` methods |
| `dlls/cbase.cpp` | Implement `SetNextThink`, `AbsoluteNextThink`, `DontThink`, `ThinkCorrection`; add Save/Restore fields; rearrange think dispatch; call `ThinkCorrection` on restore |
| `dlls/weapons.h` | Add weapons override of `SetNextThink` |
| `dlls/subs.cpp` | Use `DontThink()` in `LinearMoveDone` |

#### Verification

- [x] Project compiles on both platforms
- [x] Existing entity think timing is preserved

---

### Phase 1C: State System

**Priority**: HIGH — Many entities need state tracking for master/slave logic.
**Estimated Scope**: ~22 LRC lines, 0 new files, ~10 modified files

Adds `GetState()` virtual method returning `STATE_ON`, `STATE_OFF`, `STATE_TURN_ON`, or `STATE_TURN_OFF`.

#### Files to Modify

| File | Changes |
|------|---------|
| `dlls/cbase.h` | Add `STATE` enum, add `virtual STATE GetState()`, add `ShouldToggle` using `GetState()` |
| `dlls/util.h` | Add STATE enum values |
| `dlls/bmodels.cpp` | Add `GetState()` to func_rotating |
| `dlls/buttons.cpp` | Add state tracking to CEnvSpark, state changes in button logic |
| `dlls/lights.cpp` | Add `GetState()` to light entities |
| `dlls/effects.cpp` | Add `GetState()` to env_shake, env_fade, and similar |
| `dlls/scripted.cpp` | Add `m_iState` tracking to CCineMonster |
| `dlls/triggers.cpp` | Add state tracking to CMultiManager |
| `dlls/plats.cpp` | State-aware sound restart in trains |
| `cl_dll/hl/hl_baseentity.cpp` | Add client-side stub for `CBaseToggle::GetState()` |

#### Verification

- [x] Project compiles on both platforms
- [x] `GetState()` returns correct values for toggle entities

---

## Phase 2: Base Entity Enhancements ✅ COMPLETE

### Phase 2A: New USE Types ✅

**Priority**: HIGH — Required by trigger enhancements.
**Estimated Scope**: ~15 LRC lines
**Status**: Implemented

#### Files to Modify

| File | Changes |
|------|---------|
| `dlls/cbase.h` | Add `USE_KILL = 4`, `USE_SAME = 5`, `USE_NOT = 6` to `USE_TYPE` enum; add `FCAP_ONLYDIRECT_USE` |
| `dlls/subs.cpp` | Implement `USE_SAME`/`USE_NOT`/`USE_KILL` processing in `SUB_UseTargets`; move `m_hActivator` to `CBaseDelay`; add Save/Restore for `m_hActivator` |

---

### Phase 2B: Alias System ✅

**Priority**: MEDIUM — Used by advanced trigger targeting.
**Estimated Scope**: ~15 LRC lines, 2 new files
**Status**: Implemented

#### New Files to Create

| File | Purpose |
|------|---------|
| `dlls/alias.cpp` | Alias entity implementation |
| `dlls/alias.h` | Alias class definitions (`CBaseAlias`, etc.) |

#### Files to Modify

| File | Changes |
|------|---------|
| `dlls/cbase.h` | Add `CBaseAlias` pointer and alias handling, move alias class definitions from alias.cpp |
| `dlls/util.cpp` | Add alias flush safety mechanism |
| `dlls/util.h` | Add alias/group utility declarations |

#### Build Integration

- Add `dlls/alias.cpp` to Linux Makefile and Visual Studio project

---

### Phase 2C: Locus System ✅

**Priority**: MEDIUM — Used by advanced trigger targeting and effects.
**Estimated Scope**: ~10 LRC lines, 2 new files
**Status**: Implemented

#### New Files to Create

| File | Purpose |
|------|---------|
| `dlls/locus.cpp` | Locus position/velocity/ratio calculation implementation |
| `dlls/locus.h` | Locus utility declarations |

#### Files to Modify

| File | Changes |
|------|---------|
| `dlls/cbase.h` | Add `CalcPosition()`, `CalcVelocity()`, `CalcRatio()` virtual methods |
| `dlls/effects.cpp` | Include `locus.h`, add `Direction(CBaseEntity*)` with locus activator |
| `dlls/triggers.cpp` | Include `locus.h` |

#### Build Integration

- Add `dlls/locus.cpp` to Linux Makefile and Visual Studio project

---

### Phase 2D: Trigger System Enhancements ✅

**Priority**: HIGH — Many new entities depend on the enhanced trigger infrastructure.
**Estimated Scope**: ~120 LRC lines
**Status**: Implemented

#### Files to Modify

| File | Changes |
|------|---------|
| `dlls/triggers.cpp` | Include `weapons.h`, `movewith.h`, `locus.h` |

**CMultiManager Enhancements** (`dlls/triggers.cpp`):
- New fields: `m_flWait`, `m_flMaxWait`, `m_sMaster`, `m_iMode`, `m_iszThreadName`, `m_iszLocusThread`, `m_triggerType`
- Save/Restore entries for all new fields
- KeyValue handlers for `master`, `m_iszThreadName`, `m_iszLocusThread`, `mode`, `triggerstate`
- Mode-based firing logic (timed, pick random, each random)
- Wait time processing (random, constant, immediate)
- Clone initialization with thread name, trigger type, mode, wait copying
- Master entity checking; `SF_MULTIMAN_SAMETRIG` flag

**New Trigger Entities** (`dlls/triggers.cpp`):
- `multi_watcher`: watches multiple entity states
- `trigger_command`: executes server console commands
- `RenderFxFader`: subsidiary entity for env_render fade effects
- Refactored `trigger_changelevel` (with lowercase name validation)
- `trigger_changecvar`: changes server cvars
- `trigger_inout`: fires only on enter/leave, maintains entity registry
- `trigger_bounce`: bounces touching entities
- `trigger_onsight`: fires when entities can see each other
- Enhanced teleporter with landmark support and view angle setting
- `trigger_startpatrol`: starts monster patrol routes
- `trigger_motion`: applies position/angle/velocity to entities
- `motion_manager`: continuous motion control entity
- NODRAW optimization for hiding entities

---

## Phase 2.5: Entity USE_TYPE Compliance ✅ COMPLETE

**Priority**: HIGH — Ensures all toggle entities properly respond to `USE_ON` / `USE_OFF` instead of blindly toggling.
**Estimated Scope**: ~20 modified lines, 6 modified files
**Status**: Implemented

This phase adds `ShouldToggle()` calls to existing toggle entities that previously ignored the `useType` parameter. This ensures entities correctly respond to directed USE types (`USE_ON`, `USE_OFF`) in addition to `USE_TOGGLE`, and by extension properly support the Phase 2 types (`USE_KILL`, `USE_SAME`, `USE_NOT`) through the infrastructure.

#### Files Modified

| File | Changes |
|------|---------|
| `dlls/doors.cpp` | Added `ShouldToggle` to `CBaseDoor::Use()` — doors respect `USE_ON` / `USE_OFF` |
| `dlls/plats.cpp` | Added `ShouldToggle` to `CFuncTrain::Use()` and `CSpriteTrain::Use()` — trains respect `USE_ON` / `USE_OFF` |
| `dlls/bmodels.cpp` | Added `ShouldToggle` to `CFuncRotating::RotatingUse()`, `CPendulum::PendulumUse()`, `CFuncConveyor::Use()` |
| `dlls/triggers.cpp` | Added `ShouldToggle` to `CBaseTrigger::ToggleUse()` and `CTriggerPlayerFreeze::Use()` |
| `dlls/buttons.cpp` | Updated `CMultiSource::Use()` to respect `USE_ON` / `USE_OFF` instead of always XOR-toggling |
| `dlls/sound.cpp` | Simplified `CSpeaker::ToggleUse()` to use `ShouldToggle` instead of manual USE_TYPE checks |

#### Entities Already Using ShouldToggle (no changes needed)

These entities were already correctly implemented:
- `CLight::Use()` (lights.cpp), `CBubbling::Use()` (effects.cpp), `CLaser::Use()` (effects.cpp)
- `CSprite::Use()` (effects.cpp), `CLightning::ToggleUse/StrikeUse` (effects.cpp)
- `CFuncWall::Use()` (bmodels.cpp), `CFuncWallToggle::Use()` (bmodels.cpp)
- `CButtonTarget::Use()` (buttons.cpp), `CFuncTrackAuto::Use()` (plats.cpp)
- `CGunTarget::Use()` (plats.cpp), `CFuncTrackTrain::Use()` (plats.cpp)
- `CTriggerCamera::Use()` (triggers.cpp), `CAmbientGeneric::ToggleUse()` (sound.cpp)
- `CBaseTurret::TurretUse()` (turret.cpp), `CFuncTank::Use()` (func_tank.cpp)
- `COp4Mortar::Use()` (op4mortar.cpp), `CNuclearBomb::Use()` (nuclearbomb.cpp)

#### Verification

- [x] Project compiles on Linux (g++)
- [x] Existing entity toggle behavior is preserved when using `USE_TOGGLE`
- [x] `USE_ON` / `USE_OFF` now have correct semantics for all toggle entities

---

## Phase 3: Entity-Specific Enhancements

### Phase 3A: Monster/NPC Enhancements

**Priority**: HIGH — Largest single category (143 LRC lines).
**Estimated Scope**: ~143 LRC lines, ~30 modified files

This phase implements:
- Custom model support (`SET_MODEL`/`PRECACHE_MODEL` using `pev->model`)
- Monster allegiance system (`m_iClass`, `m_iPlayerReact`, faction classes)
- Custom gibs support (`HasCustomGibs`)
- Pre-disaster mode flag (`SF_MONSTER_PREDISASTER`)
- Modified `CanPlaySequence` prototype
- Scripted action precision attacks (`PreciseAttack()`)
- Various combat and AI tweaks

#### Files to Modify

| File | Changes |
|------|---------|
| `dlls/basemonster.h` | Add `m_iClass`, `m_iPlayerReact` for allegiance; add `HasCustomGibs()` virtual; add faction class constants (`CLASS_FACTION_A/B/C`); modified `CanPlaySequence` signature |
| `dlls/monsters.h` | Add `SF_MONSTER_NO_YELLOW_BLOBS`, `SF_MONSTER_NO_WPN_DROP` flags; modify `SpawnRandomGibs` signature |
| `dlls/monsters.cpp` | KeyValue handlers for `m_iClass`, `m_iPlayerReact`; stuck monster tolerance; target classification with `m_iPlayerReact`; debug sequence logging; `info_monster_goal` entity |
| `dlls/combat.cpp` | Custom gib models from blood color; include `studio.h`; custom gib count from model; solid trigger for gib spread |
| `dlls/talkmonster.h` | Add `m_iszDecline` (refuse to accompany) |
| `dlls/talkmonster.cpp` | Add `m_iszDecline`, `m_iszSpeakAs` Save/Restore; SpeakAs voice group changing; predisaster flag handling; rewritten CanFollow/StopFollowing; KeyValue for RefusalSentence/SpeakAs |
| `dlls/agrunt.cpp` | Custom model, hornet allegiance, scripted action precision |
| `dlls/apache.cpp` | Custom model |
| `dlls/barnacle.cpp` | Custom model, MoveWith velocity on grabbed entity |
| `dlls/barney.cpp` | Custom model, `m_iBaseBody`, health default, predisaster flag, player reaction override |
| `dlls/bigmomma.cpp` | Custom model, animation header import |
| `dlls/bloater.cpp` | Custom model |
| `dlls/bullsquid.cpp` | Custom model, entity class link fix, scripted action support |
| `dlls/controller.cpp` | Custom model, scripted fire support |
| `dlls/gargantua.cpp` | Custom model, scripted action support |
| `dlls/genericmonster.cpp` | Null model safety, custom gib model precache, yaw speed fix, spawnflag clip fix |
| `dlls/gman.cpp` | Custom model |
| `dlls/hassassin.cpp` | Custom model, scripted action support, enemy check |
| `dlls/headcrab.cpp` | Custom model (both variants) |
| `dlls/hgrunt.cpp` | Custom model, allegiance override, script fire check, no weapon drop, grenade throw safety, scripted fire |
| `dlls/houndeye.cpp` | Custom model |
| `dlls/ichthyosaur.cpp` | Custom model |
| `dlls/islave.cpp` | Custom model, player reaction override |
| `dlls/islave_deamon.cpp` | Custom model, player reaction override |
| `dlls/leech.cpp` | Custom model |
| `dlls/nihilanth.cpp` | Custom model |
| `dlls/osprey.cpp` | Custom model, default speed, grunt spawning fix, loop breaker |
| `dlls/rat.cpp` | Custom model, classification override |
| `dlls/roach.cpp` | Custom model |
| `dlls/scientist.cpp` | Custom model (both variants), postdisaster sitter |
| `dlls/tempmonster.cpp` | Custom model |
| `dlls/turret.cpp` | Custom model (all variants), health default |
| `dlls/zombie.cpp` | Custom model |
| `dlls/defaultai.cpp` | AI schedule modifications |
| `dlls/AI_BaseNPC_Schedule.cpp` | Immediate play start, teleport script turn, turn type check |
| **OpFor Monster Files** | |
| `dlls/voltigore.cpp` | Custom model, `m_iClass` allegiance |
| `dlls/baby_voltigore.cpp` | Custom model, `m_iClass` allegiance |
| `dlls/pitdrone.cpp` | Custom model, `m_iClass` allegiance |
| `dlls/gonome.cpp` | Custom model, `m_iClass` allegiance |
| `dlls/shocktrooper.cpp` | Custom model, `m_iClass` allegiance, `SF_MONSTER_NO_WPN_DROP` in `GibMonster()`/`HandleAnimEvent()` |
| `dlls/geneworm.cpp` | Custom model, `m_iClass` allegiance |
| `dlls/pitworm_up.cpp` | Custom model, `m_iClass` allegiance |
| `dlls/otis.cpp` | Custom model, `m_iClass` allegiance, `SF_MONSTER_NO_WPN_DROP` in `Killed()` |
| `dlls/CDrillSergeant.cpp` | Custom model, `m_iClass` allegiance |
| `dlls/CRecruit.cpp` | Custom model, `m_iClass` allegiance |
| `dlls/cleansuit_scientist.cpp` | Custom model, `m_iClass` allegiance |
| `dlls/blkop_osprey.cpp` | Custom model, `m_iClass` allegiance |
| `dlls/blkop_apache.cpp` | Custom model, `m_iClass` allegiance |
| `dlls/hgrunt_medic.cpp` | Custom model, `m_iClass` allegiance, `SF_MONSTER_NO_WPN_DROP` in `GibMonster()`/`HandleAnimEvent()` |
| `dlls/hgrunt_torch.cpp` | Custom model, `m_iClass` allegiance, `SF_MONSTER_NO_WPN_DROP` in `GibMonster()`/`HandleAnimEvent()` |
| `dlls/male_assassin.cpp` | Custom model, `m_iClass` allegiance, `SF_MONSTER_NO_WPN_DROP` in `GibMonster()`/`HandleAnimEvent()` |
| `dlls/hfgrunt.cpp` | Custom model, `m_iClass` allegiance, `SF_MONSTER_NO_WPN_DROP` in `GibMonster()`/`HandleAnimEvent()` |
| `dlls/shockroach.cpp` | Custom model, `m_iClass` allegiance |
| `dlls/loader.cpp` | Custom model, `m_iClass` allegiance |
| `dlls/skeleton.cpp` | Custom model, `m_iClass` allegiance |

---

### Phase 3B: Door Enhancements

**Priority**: MEDIUM
**Estimated Scope**: ~17 LRC lines

#### Files to Modify

| File | Changes |
|------|---------|
| `dlls/doors.h` | Add `SF_DOOR_FORCETOUCHABLE` flag; document `SF_DOOR_SYNCHED` replacement |
| `dlls/doors.cpp` | Direct-use capability; immediate-use flags; synched firing on open/close start; MoveWith integration; open target via `message`; speed override for rotating doors; state-based position check |

---

### Phase 3C: Button Enhancements

**Priority**: MEDIUM
**Estimated Scope**: ~19 LRC lines

#### Files to Modify

| File | Changes |
|------|---------|
| `dlls/buttons.cpp` | Add `SF_BUTTON_ONLYDIRECT` flag; simple state entity; multisource rewrite; USE_ON/USE_OFF handling; master lock check for momentary_rot_button; boundary checking; avelocity calculation; state tracking for CEnvSpark |

---

### Phase 3D: Breakable Enhancements

**Priority**: MEDIUM
**Estimated Scope**: ~12 LRC lines

#### Files to Modify

| File | Changes |
|------|---------|
| `dlls/func_break.h` | Add respawn/whenhit fields |
| `dlls/func_break.cpp` | KeyValue for `respawn`, `whenhit`; respawn time/health; `m_iClass` support; locus trigger for `whenhit`; non-pullable pushable (`SF_PUSH_NOPULL`) |

---

### Phase 3E: Platform/Train Enhancements

**Priority**: MEDIUM
**Estimated Scope**: ~69 LRC lines

#### Files to Modify

| File | Changes |
|------|---------|
| `dlls/trains.h` | Add `SF_TRACKTRAIN_NOYAW`, `SF_TRACKTRAIN_AVELOCITY`, `SF_TRACKTRAIN_AVEL_GEARS`, `SF_PATH_AVELOCITY` flags; armortype/frags/armorvalue values; `DesiredAction()` rename; `m_vecMasterAvel`, `m_vecBaseAvel` members |
| `dlls/plats.cpp` | Scripted train sequence; m_activated move to CBaseEntity; sequence/avelocity Save/Restore; backward path search; avelocity path following; teleport/turn-to-face corners; PostSpawn for initial path; non-crushing trains (`pev->dmg == -1`); movement sound persistence after load; UTIL_SetVelocity/SetAvelocity/SetAngles/AssignOrigin usage; scripted train sequence entity |

---

### Phase 3F: Func_Tank Enhancements

**Priority**: MEDIUM
**Estimated Scope**: ~45 LRC lines

#### Files to Modify

| File | Changes |
|------|---------|
| `dlls/func_tank.cpp` | New spawnflags (laserspot, matchtarget, seqfire); crosshair member; TankControls go-between; TankSequence/SequenceEnemy; FireMaster; locus fire proxy; control migration to FuncTankControls; match target mode; tripbeams; crosshair support; CTankSequence entity |

---

### Phase 3G: Light Enhancements

**Priority**: LOW
**Estimated Scope**: ~6 LRC lines

#### Files to Modify

| File | Changes |
|------|---------|
| `dlls/lights.cpp` | `GetStyle()`/`SetStyle()` methods; `SetCorrectStyle()`; `CLightDynamic` entity (flashlight-like); `CTriggerLightstyle` entity (temporary light style changes) |

---

### Phase 3H: Sound Enhancements

**Priority**: LOW
**Estimated Scope**: ~14 LRC lines

#### Files to Modify

| File | Changes |
|------|---------|
| `dlls/sound.cpp` | `m_pPlayFrom` (entity to play from); `m_iChannel`; Save/Restore fields; `StartPlayFrom` think function; redirected EMIT_SOUND_DYN/STOP_SOUND to use m_pPlayFrom; `trigger_sound` entity |

---

### Phase 3I: Scripted Sequence Enhancements

**Priority**: MEDIUM
**Estimated Scope**: ~33 LRC lines

#### Files to Modify

| File | Changes |
|------|---------|
| `dlls/scripted.h` | `SF_SCRIPT_STAYDEAD` flag; rearranged turn flags; state enum; `InitIdleThink`; `IsAction()` inline; `PreciseAttack()`; repeat count/frame members; priority; remove obsolete CCineAI |
| `dlls/scripted.cpp` | State Save/Restore; attack/movetarget fields; link `scripted_action` and `aiscripted_sequence`; AI vs normal sequence distinction; redefined entity search; repeat counter reset; AMBUSH schedule; cleanup on script end; fire-on-begin target; solidity preservation; state tracking throughout |

---

### Phase 3J: Effects Enhancements

**Priority**: MEDIUM
**Estimated Scope**: ~73 LRC lines

#### Files to Modify

| File | Changes |
|------|---------|
| `dlls/effects.h` | Tripbeam support; smoother lasers; `GetTripEntity` |
| `dlls/effects.cpp` | Include `player.h`, `locus.h`, `movewith.h` |

**Effects Entity Changes** (`dlls/effects.cpp`):
- `info_target` as a proper entity class (using `sprites/null.spr`)
- `BeamUpdatePoints` method for dynamic beam endpoint updates
- Beam toggles use `USE_OFF`/`USE_ON` instead of generic toggle
- Follow any entity with a model (not just specific types)
- Non-restriking beams support (`m_restrike == -1`)
- Tripbeam entities and healing beams
- Spritename alias resolution for env_beam
- Velocity reset using `UTIL_SetVelocity`
- Gibshooter debug flag (`SF_GIBSHOOTER_DEBUG`)
- Simultaneous gib fire when delay is 0
- Direction calculation with locus activator
- Permanent fade flag (`SF_FADE_PERMANENT`) and `FFADE_STAYOUT`
- Bolt-on state for env_shake/env_fade

**New Effect Entities** (`dlls/effects.cpp`):
- `env_dlight`: dynamic entity light creator
- `env_elight`: entity-attached light
- Decal effect entity
- `env_particle`: aurora particle system
- Quake 1 particle effects
- Beam Trail effect
- Custom footstep sounds
- Rain effect (`env_rain`)
- Xen warp-in effect (`env_warpball`)
- Shockwave effect

**Brush Model Changes** (`dlls/bmodels.cpp`):
- Rotating entity center calculation bug fix
- MoveWith conditional logic
- `WaitForStart` for func_rotating startup behavior
- Spin speed tracking (`m_fCurSpeed`)
- State tracking for func_rotating
- `UTIL_SetAvelocity` usage throughout
- Shiny surfaces support

---

## Phase 4: Visual & Client-Side Features

### Phase 4A: Fog System

**Priority**: MEDIUM
**Estimated Scope**: ~13 LRC lines, server + client changes

#### Server-Side (`dlls/`)

| File | Changes |
|------|---------|
| `dlls/effects.cpp` | Implement `env_fog` entity class |
| `dlls/player.cpp` | Register `gmsgSetFog` message |

#### Client-Side (`cl_dll/`)

| File | Changes |
|------|---------|
| `cl_dll/hud.cpp` | `HOOK_MESSAGE(SetFog)` |
| `cl_dll/hud.h` | `MsgFunc_SetFog` declaration |
| `cl_dll/hud_msg.cpp` | SetFog message handler, fog reset on level load, fog clear |
| `cl_dll/hud_redraw.cpp` | Fog fading values and effects |
| `cl_dll/view.cpp` | `RenderFog()` call |
| `cl_dll/hl/hl_weapons.cpp` | `g_iWaterLevel` for DMC fog |

---

### Phase 4B: Sky System

**Priority**: MEDIUM
**Estimated Scope**: ~9 LRC lines, server + client changes

#### Server-Side

| File | Changes |
|------|---------|
| `dlls/effects.cpp` | Implement `env_sky` entity class |
| `dlls/player.cpp` | Register `gmsgSetSky` message |

#### Client-Side

| File | Changes |
|------|---------|
| `cl_dll/hud.cpp` | `HOOK_MESSAGE(SetSky)` |
| `cl_dll/hud.h` | `m_vecSkyPos`, `m_iSkyMode`, `SKY_OFF` constant, `MsgFunc_SetSky` declaration |
| `cl_dll/hud_msg.cpp` | SetSky handler, reset on level load |
| `cl_dll/view.cpp` | Sky view position override, weapon model hiding during sky draw, second-pass setup |

---

### Phase 4C: Custom HUD Color

**Priority**: LOW
**Estimated Scope**: ~7 LRC lines

#### Server-Side

| File | Changes |
|------|---------|
| `dlls/player.cpp` | Register `gmsgHUDColor` message |

#### Client-Side

| File | Changes |
|------|---------|
| `cl_dll/hud.cpp` | `HOOK_MESSAGE(HUDColor)` |
| `cl_dll/hud.h` | `m_iHUDColor` member, `MsgFunc_HUDColor` declaration |
| `cl_dll/ammo_secondary.cpp` | Use `gHUD.m_iHUDColor` for ammo display |
| `cl_dll/health.cpp` | Use `gHUD.m_iHUDColor` for health display |

---

### Phase 4D: Shiny/Reflective Surfaces

**Priority**: LOW
**Estimated Scope**: ~10 LRC lines

#### Server-Side

| File | Changes |
|------|---------|
| `dlls/player.cpp` | Register `gmsgAddShine` message |
| `dlls/bmodels.cpp` | Shiny surface support |
| `common/const.h` | Add `kRenderFxReflection` |

#### Client-Side

| File | Changes |
|------|---------|
| `cl_dll/hud.cpp` | `HOOK_MESSAGE(AddShine)`, initialize `m_pShinySurface` |
| `cl_dll/hud.h` | `CShinySurface*` member, `MsgFunc_AddShine` declaration |
| `cl_dll/hud_msg.cpp` | Clear shiny surfaces on level load |
| `cl_dll/tri.cpp` | `CShinySurface` implementation |

---

### Phase 4E: Dynamic Lighting

**Priority**: LOW
**Estimated Scope**: ~6 LRC lines

#### Server-Side

| File | Changes |
|------|---------|
| `dlls/player.cpp` | Register `gmsgKeyedDLight` message |
| `dlls/effects.cpp` | `env_dlight` entity (dynamic entity light) |
| `dlls/util.h` | `GetStdLightStyle()` declaration |

#### Client-Side

| File | Changes |
|------|---------|
| `cl_dll/hud.cpp` | `HOOK_MESSAGE(KeyedDLight)` |
| `cl_dll/hud.h` | `MsgFunc_KeyedDLight` declaration |

---

### Phase 4F: Model/Animation Scaling

**Priority**: LOW
**Estimated Scope**: ~6 LRC lines

#### Files to Modify

| File | Changes |
|------|---------|
| `cl_dll/StudioModelRenderer.cpp` | Apply scale to models in rendering |
| `dlls/animation.cpp` | Animation utility additions |
| `dlls/animation.h` | `GetSequenceFrames()` declaration |

---

### Phase 4G: Particle System

**Priority**: LOW
**Estimated Scope**: ~1 LRC line, 2 new files

#### New Files to Create

| File | Purpose |
|------|---------|
| `cl_dll/particlemgr.cpp` | Client-side particle system manager with time-based updates |
| `cl_dll/particlemgr.h` | Particle manager header |

#### Files to Modify

| File | Changes |
|------|---------|
| `cl_dll/tri.cpp` | Draw and update particle systems; time elapsed calculation |
| `dlls/effects.cpp` | `env_particle` entity using aurora particle system |
| `dlls/player.cpp` | Register `gmsgParticle` message |
| `dlls/player.h` | Declare `gmsgParticle` extern |

#### Build Integration

- Add `cl_dll/particlemgr.cpp` to Linux Makefile and Visual Studio project

---

## Phase 5: New Entity Definitions & Polish

### Phase 5A: New Entity Definitions

**Priority**: LOW
**Estimated Scope**: ~5 LRC lines

#### Files to Modify

| File | Changes |
|------|---------|
| `dlls/cbase.h` | Add `FCAP_ONLYDIRECT_USE`; add `CLASS_FACTION_A`, `CLASS_FACTION_B`, `CLASS_FACTION_C` classification constants |
| `common/const.h` | New content types (`CONTENT_SPECIAL1`, etc.); `kRenderFxReflection` (if not done in Phase 4D) |
| `dlls/cdll_dll.h` | `HIDEHUD_CUSTOMCROSSHAIR` flag |

---

### Phase 5B: Miscellaneous Changes & Debug Tools

**Priority**: LOW
**Estimated Scope**: ~80+ LRC lines scattered across many files

#### Key Changes

| File | Changes |
|------|---------|
| `dlls/game.cpp` | `impulsetarget` cvar, `mw_debug` cvar, register both |
| `dlls/game.h` | (If needed) declare new cvars |
| `dlls/client.cpp` | "fire" console command for manual entity triggering |
| `dlls/player.cpp` | Impulse 90/91/92 for USE_TOGGLE/USE_ON/USE_OFF; direct entity use improvements; suit flatline sound fix; various small fixes |
| `dlls/gamerules.cpp` | Misc LRC changes |
| `dlls/gamerules.h` | Misc LRC changes |
| `dlls/singleplay_gamerules.cpp` | "Start with HEV" flag support; allow `game_player_equip` in single player |
| `dlls/world.cpp` | Precache `sprites/null.spr`; start-with-suit flag |
| `dlls/h_battery.cpp` | LRC changes to battery entity |
| `dlls/healthkit.cpp` | LRC changes to healthkit entity |
| `dlls/items.cpp` | Custom model/sound for items |
| `dlls/crowbar.cpp` | Attack timing bug fix |
| `dlls/rpg.cpp` | Suspend indefinitely support (-1) |
| `dlls/explode.cpp` | Origin fix for explosions |
| `dlls/pathcorner.cpp` | `turnspeed` KeyValue |
| `dlls/weapons.cpp` | Misc LRC changes; weapon strip ammo removal |
| `dlls/schedule.h` | LRC schedule changes |
| `dlls/util.cpp` | `UTIL_AxisRotationToAngles`, `UTIL_AxisRotationToVec`, alias flush, `UTIL_IsFacing` move, `UTIL_StringToRandomVector` |
| `dlls/util.h` | Various new utility declarations; `SF_BREAK_FADE_RESPAWN`, `SF_PUSH_NOPULL`, `SF_PUSH_USECUSTOMSIZE` |
| `engine/eiface.h` | `at_debug` ALERT type |
| `cl_dll/cl_dll.h` | `CONPRINT` macro |
| `cl_dll/ammo.cpp` | Custom crosshair experiment; weapon pointer null |
| `cl_dll/hud.cpp` | Various message handler registrations |
| `cl_dll/hud.h` | Various new declarations |
| `cl_dll/hud_msg.cpp` | Various message handlers |
| `cl_dll/view.cpp` | Misc LRC changes |
| `cl_dll/hl/hl_baseentity.cpp` | Client-side stubs for new methods |
| `cl_dll/hl/hl_weapons.cpp` | Misc LRC changes |

---

## Build Integration Summary

### New Files to Add to Build System

| File | Build Target |
|------|-------------|
| `dlls/movewith.cpp` | Server (hl.dll / hl.so) |
| `dlls/alias.cpp` | Server (hl.dll / hl.so) |
| `dlls/locus.cpp` | Server (hl.dll / hl.so) |
| `cl_dll/particlemgr.cpp` | Client (client.dll / client.so) |

### Linux Makefile (`linux/Makefile`)

Add the new `.cpp` files to the appropriate object lists (server DLL and client DLL sections).

### Visual Studio Project (`projects/vs2019/`)

Add the new `.cpp` and `.h` files to the respective project filters.

---

## Risk Assessment

| Risk | Severity | Mitigation |
|------|----------|------------|
| Save/Restore corruption from new fields | HIGH | Add all `DEFINE_FIELD` entries in the same commit as the member variables |
| Infinite loops in MoveWith chains | MEDIUM | `MAX_MOVEWITH_DEPTH` constant with abort on exceed |
| Engine overriding nextthink | MEDIUM | `ThinkCorrection()` detects and compensates |
| Breaking existing entity behavior | HIGH | Test each phase independently; no MoveWith/Think changes fire without explicit keyvalue setup |
| Client-server message mismatch | MEDIUM | Register messages in consistent order; test both client and server DLLs together |
| Compile errors from missing declarations | LOW | Follow dependency order (Phase 1 → 2 → 3 → 4 → 5) |

---

## Estimated Total Scope

| Metric | Count |
|--------|-------|
| New files to create | 8 |
| Existing files to modify | ~90 |
| Total LRC lines to implement | ~795 |
| Phases | 5 (with 18 sub-phases) |
