# TODO / Unfinished Code Passages

> Automatisch generiert. Gesamt: **263** Einträge in **125** Dateien.


## Zusammenfassung nach Typ

| Typ | Anzahl |
|-----|--------|
| `TODO` | 178 |
| `FIXME` | 30 |
| `HACK` | 37 |
| `BUG` | 18 |


---

## Einträge nach Verzeichnis


### `cl_dll/`


#### `cl_dll/StudioModelRenderer.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [442](cl_dll/StudioModelRenderer.cpp#L442) | `TODO` | TODO: should really be stored with the entity instead of being reconstructed |
| [443](cl_dll/StudioModelRenderer.cpp#L443) | `TODO` | TODO: should use a look-up table |
| [444](cl_dll/StudioModelRenderer.cpp#L444) | `TODO` | TODO: could cache lazily, stored in the entity |
| [544](cl_dll/StudioModelRenderer.cpp#L544) | `FIXME` | FIXME: make this work for clipped case too? |
| [605](cl_dll/StudioModelRenderer.cpp#L605) | `BUG` | BUG ( somewhere else ) but this code should validate this data. |


#### `cl_dll/demo.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [28](cl_dll/demo.cpp#L28) | `FIXME` | FIXME:  There should be buffer helper functions to avoid all of the *(int *)& crap. |


#### `cl_dll/ev_hldm.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [91](cl_dll/ev_hldm.cpp#L91) | `FIXME` | FIXME check if playtexture sounds movevar is set |
| [1285](cl_dll/ev_hldm.cpp#L1285) | `TODO` | TODO: Fully predict the fliying bolt. |
| [1468](cl_dll/ev_hldm.cpp#L1468) | `HACK` | HACK: only reset animation if the Egon is still equipped. |


#### `cl_dll/health.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [100](cl_dll/health.cpp#L100) | `TODO` | TODO: update local health data |
| [329](cl_dll/health.cpp#L329) | `TODO` | TODO:  get the shift value of the health |


#### `cl_dll/hl/hl_weapons.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [659](cl_dll/hl/hl_weapons.cpp#L659) | `FIXME` | FIXME, make this a method in each weapon?  where you pass in an entity_state_t *? |


#### `cl_dll/hud_flagicons.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [60](cl_dll/hud_flagicons.cpp#L60) | `TODO` | TODO: can this ever return 2? |
| [96](cl_dll/hud_flagicons.cpp#L96) | `TODO` | TODO: this buffer is static in vanilla Op4 |


#### `cl_dll/hud_msg.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [92](cl_dll/hud_msg.cpp#L92) | `TODO` | TODO: needs to be called on every map change, not just when starting a new game |
| [117](cl_dll/hud_msg.cpp#L117) | `TODO` | TODO: define game mode constants |
| [163](cl_dll/hud_msg.cpp#L163) | `TODO` | TODO: kick viewangles,  show damage visually |


#### `cl_dll/hud_playerbrowse.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [129](cl_dll/hud_playerbrowse.cpp#L129) | `TODO` | TODO: is the leading space supposed to be here? |
| [136](cl_dll/hud_playerbrowse.cpp#L136) | `TODO` | TODO: unsafe use of strncat count parameter |
| [201](cl_dll/hud_playerbrowse.cpp#L201) | `TODO` | TODO: unsafe |


#### `cl_dll/hud_spectator.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [121](cl_dll/hud_spectator.cpp#L121) | `TODO` | TODO: none of this spectator stuff exists in Op4 |
| [407](cl_dll/hud_spectator.cpp#L407) | `TODO` | TODO: this flags check is incorrect, fix it. Comment contains original code before bool fix. |
| [668](cl_dll/hud_spectator.cpp#L668) | `TODO` | TODO: this is pretty ugly, need a better way. |


#### `cl_dll/in_camera.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [412](cl_dll/in_camera.cpp#L412) | `HACK` | extern void KeyDown(kbutton_t* b); // HACK |
| [413](cl_dll/in_camera.cpp#L413) | `HACK` | extern void KeyUp(kbutton_t* b);   // HACK |


#### `cl_dll/input.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [314](cl_dll/input.cpp#L314) | `TODO` | TODO: define constants |


#### `cl_dll/inputw32.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [225](cl_dll/inputw32.cpp#L225) | `TODO` | TODO: accessing the cvar value is a race condition |


#### `cl_dll/particleman/CBaseParticle.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [101](cl_dll/particleman/CBaseParticle.cpp#L101) | `TODO` | TODO: not sure if these are correct. If low left is half of the scaled directions then the full direction * 2 results... |
| [209](cl_dll/particleman/CBaseParticle.cpp#L209) | `TODO` | TODO: shouldn't this be accounting for stretch Y? |


#### `cl_dll/particleman/CFrustum.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [25](cl_dll/particleman/CFrustum.cpp#L25) | `TODO` | TODO: this function always operates on the frustum matrix that's part of this object, so there is no need to pass the... |
| [48](cl_dll/particleman/CFrustum.cpp#L48) | `TODO` | TODO: clean this up. It's a 4x4 matrix multiplication. |


#### `cl_dll/particleman/IParticleMan_Active.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [134](cl_dll/particleman/IParticleMan_Active.cpp#L134) | `TODO` | TODO: engine doesn't support printing size_t, use local printf |


#### `cl_dll/particleman/particleman_internal.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [24](cl_dll/particleman/particleman_internal.h#L24) | `TODO` | TODO: remove this once the clamp macro has been removed |


#### `cl_dll/status_icons.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [238](cl_dll/status_icons.cpp#L238) | `TODO` | TODO: potential overflow |


#### `cl_dll/studio_util.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [28](cl_dll/studio_util.cpp#L28) | `FIXME` | FIXME: rescale the inputs to 1/2 angle |


#### `cl_dll/vgui_ClassMenu.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [279](cl_dll/vgui_ClassMenu.cpp#L279) | `BUG` | TODO: apparently bugged in vanilla, still uses old indexing code with no second array index |


#### `cl_dll/vgui_ScorePanel.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [277](cl_dll/vgui_ScorePanel.cpp#L277) | `TODO` | return 0 != gEngfuncs.GetPlayerUniqueID(iPlayer, playerID); // TODO remove after testing |


#### `cl_dll/vgui_StatsMenuPanel.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [128](cl_dll/vgui_StatsMenuPanel.cpp#L128) | `BUG` | TODO: this is using YRES for an X coord. Bugged in vanilla |
| [368](cl_dll/vgui_StatsMenuPanel.cpp#L368) | `TODO` | TODO: this passes an arbitrary string as the format string which is incredibly dangerous (also in vanilla) |
| [416](cl_dll/vgui_StatsMenuPanel.cpp#L416) | `TODO` | TODO: missing from vanilla |
| [453](cl_dll/vgui_StatsMenuPanel.cpp#L453) | `TODO` | TODO: this passes an arbitrary string as the format string which is incredibly dangerous (also in vanilla) |
| [485](cl_dll/vgui_StatsMenuPanel.cpp#L485) | `TODO` | TODO: missing from vanilla |


#### `cl_dll/vgui_TeamFortressViewport.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [2193](cl_dll/vgui_TeamFortressViewport.cpp#L2193) | `TODO` | TODO: not written by Op4 |


#### `cl_dll/vgui_TeamFortressViewport.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [28](cl_dll/vgui_TeamFortressViewport.h#L28) | `TODO` | TODO: this is a real mess |


#### `cl_dll/view.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [180](cl_dll/view.cpp#L180) | `TODO` | TODO: bobtime will eventually become a value so large that it will no longer behave properly. |
| [531](cl_dll/view.cpp#L531) | `FIXME` | FIXME, we send origin at 1/128 now, change this? |
| [706](cl_dll/view.cpp#L706) | `FIXME` | FIXME		I_Error ("steptime < 0"); |
| [1072](cl_dll/view.cpp#L1072) | `HACK` | HACK, if player is dead don't clip against his dead body, can't check this |


### `common/`


#### `common/PlatformHeaders.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [61](common/PlatformHeaders.h#L61) | `TODO` | TODO: ARRAYSIZE should be replaced with std::size, which is a superior replacement |


### `dlls/`


#### `dlls/COFSquadTalkMonster.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [259](dlls/COFSquadTalkMonster.cpp#L259) | `TODO` | TODO: pEnemy could be null here |


#### `dlls/animating.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [148](dlls/animating.cpp#L148) | `FIXME` | FIXME: I have to do this or some events get missed, and this is probably causing the problem below |


#### `dlls/baby_voltigore.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [205](dlls/baby_voltigore.cpp#L205) | `TODO` | TODO: use a filter based on attacker to identify self harm |


#### `dlls/basemonster.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [104](dlls/basemonster.h#L104) | `HACK` | Vector m_HackedGunPos; // HACK until we can query end of gun |


#### `dlls/blowercannon.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [57](dlls/blowercannon.cpp#L57) | `TODO` | TODO: probably shadowing CBaseDelay |
| [99](dlls/blowercannon.cpp#L99) | `TODO` | TODO: should call base |
| [141](dlls/blowercannon.cpp#L141) | `TODO` | TODO: can crash if target has been removed |


#### `dlls/bmodels.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [183](dlls/bmodels.cpp#L183) | `HACK` | HACKHACK - This is to allow for some special effects |
| [197](dlls/bmodels.cpp#L197) | `HACK` | HACKHACK -- This is ugly, but encode the speed in the rendercolor to avoid adding more data to the network stream |


#### `dlls/buttons.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [1267](dlls/buttons.cpp#L1267) | `BUG` | BUGBUG: This design causes a latentcy.  When the button is retriggered, the first impulse |
| [1337](dlls/buttons.cpp#L1337) | `HACK` | HACKHACK -- If we're going slow, we'll get multiple player packets per frame, bump nexthink on each one to avoid stal... |


#### `dlls/cbase.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [356](dlls/cbase.cpp#L356) | `HACK` | HACKHACK - reset the save pointers, we're going to restore for real this time |


#### `dlls/cleansuit_scientist.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [338](dlls/cleansuit_scientist.cpp#L338) | `FIXME` | {TASK_SET_ACTIVITY, (float)ACT_CROUCHIDLE}, // FIXME: This looks lame |


#### `dlls/client.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [436](dlls/client.cpp#L436) | `TODO` | TODO: clamp cvar value so it can't be negative |
| [754](dlls/client.cpp#L754) | `TODO` | TODO: in vanilla Op4 this code incorrectly skips the above validation logic if the player is already in a team |
| [1513](dlls/client.cpp#L1513) | `HACK` | HACK:  Somewhat... |


#### `dlls/combat.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [41](dlls/combat.cpp#L41) | `HACK` | HACKHACK -- The gib velocity equations don't work |
| [963](dlls/combat.cpp#L963) | `HACK` | HACKHACK Don't kill monsters in a script.  Let them break their scripts first |


#### `dlls/controller.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [183](dlls/controller.cpp#L183) | `HACK` | HACK HACK -- until we fix this. |


#### `dlls/ctf/CHUDIconTrigger.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [153](dlls/ctf/CHUDIconTrigger.cpp#L153) | `TODO` | TODO: this will break when an index is larger than 31 or a negative value |


#### `dlls/ctf/CItemAcceleratorCTF.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [39](dlls/ctf/CItemAcceleratorCTF.cpp#L39) | `TODO` | TODO: is this actually used? |


#### `dlls/ctf/CItemBackpackCTF.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [84](dlls/ctf/CItemBackpackCTF.cpp#L84) | `TODO` | TODO: precache calls should be in Precache |
| [93](dlls/ctf/CItemBackpackCTF.cpp#L93) | `TODO` | TODO: shouldn't this be using pev->model? |


#### `dlls/ctf/CItemCTF.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [37](dlls/ctf/CItemCTF.cpp#L37) | `TODO` | TODO: should invoke base class KeyValue here |
| [90](dlls/ctf/CItemCTF.cpp#L90) | `TODO` | TODO: already done above |
| [240](dlls/ctf/CItemCTF.cpp#L240) | `TODO` | TODO: really shouldn't be using the index here tbh |


#### `dlls/ctf/CItemLongJumpCTF.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [78](dlls/ctf/CItemLongJumpCTF.cpp#L78) | `TODO` | TODO: precache calls should be in Precache |
| [87](dlls/ctf/CItemLongJumpCTF.cpp#L87) | `TODO` | TODO: shouldn't this be using pev->model? |


#### `dlls/ctf/CItemPortableHEVCTF.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [81](dlls/ctf/CItemPortableHEVCTF.cpp#L81) | `TODO` | TODO: shouldn't this be using pev->model? |


#### `dlls/ctf/CItemRegenerationCTF.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [80](dlls/ctf/CItemRegenerationCTF.cpp#L80) | `TODO` | TODO: precache calls should be in Precache |
| [89](dlls/ctf/CItemRegenerationCTF.cpp#L89) | `TODO` | TODO: shouldn't this be using pev->model? |


#### `dlls/ctf/ctfplay_gamerules.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [70](dlls/ctf/ctfplay_gamerules.cpp#L70) | `TODO` | TODO: rework so it's 2 separate lists |
| [129](dlls/ctf/ctfplay_gamerules.cpp#L129) | `TODO` | TODO: doesn't really make sense, if team 0 is losing the score difference is 0 |
| [438](dlls/ctf/ctfplay_gamerules.cpp#L438) | `TODO` | TODO: this can probably be optimized by finding the last item that the player is carrying and only sending that |
| [505](dlls/ctf/ctfplay_gamerules.cpp#L505) | `TODO` | TODO: checks against an index that may not have been sent |
| [702](dlls/ctf/ctfplay_gamerules.cpp#L702) | `TODO` | TODO: doesn't seem to be used |
| [1770](dlls/ctf/ctfplay_gamerules.cpp#L1770) | `TODO` | TODO: player count is always 0 |


#### `dlls/effects.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [107](dlls/effects.cpp#L107) | `HACK` | HACKHACK!!! - Speed in rendercolor |


#### `dlls/explode.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [260](dlls/explode.cpp#L260) | `HACK` | HACKHACK -- create one of these and fake a keyvalue to get the right explosion setup |


#### `dlls/func_break.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [181](dlls/func_break.cpp#L181) | `HACK` | HACK:  matGlass can receive decals, we need the client to know about this |
| [774](dlls/func_break.cpp#L774) | `BUG` | BUGBUG -- can only find 256 entities on a breakable -- should be enough |


#### `dlls/func_tank.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [491](dlls/func_tank.cpp#L491) | `TODO` | TODO: bring back the controllers current weapon |
| [525](dlls/func_tank.cpp#L525) | `HACK` | HACKHACK -- make some noise (that the AI can hear) |


#### `dlls/func_tank_of.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [143](dlls/func_tank_of.cpp#L143) | `TODO` | TODO: could be exploited to make a tank change targets |
| [400](dlls/func_tank_of.cpp#L400) | `TODO` | TODO: bring back the controllers current weapon |
| [435](dlls/func_tank_of.cpp#L435) | `HACK` | HACKHACK -- make some noise (that the AI can hear) |


#### `dlls/gargantua.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [981](dlls/gargantua.cpp#L981) | `HACK` | HACKHACK!!! |
| [1065](dlls/gargantua.cpp#L1065) | `HACK` | HACKHACK - turn off the flames if they are on and garg goes scripted / dead |
| [1342](dlls/gargantua.cpp#L1342) | `HACK` | HACKHACK Cut and pasted from explode.cpp |


#### `dlls/geneworm.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [166](dlls/geneworm.cpp#L166) | `TODO` | TODO: the original code looks like it may be ignoring the modulo, verify this |
| [1137](dlls/geneworm.cpp#L1137) | `TODO` | TODO: this really shouldn't be hardcoded |
| [1202](dlls/geneworm.cpp#L1202) | `TODO` | TODO: maybe determine direction of velocity to apply? |
| [1336](dlls/geneworm.cpp#L1336) | `TODO` | TODO: never used? |


#### `dlls/gonome.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [88](dlls/gonome.cpp#L88) | `TODO` | TODO: probably shouldn't be assinging to x every time |
| [248](dlls/gonome.cpp#L248) | `TODO` | TODO: needs to be EHANDLE, save/restored or a save during a windup will cause problems |
| [376](dlls/gonome.cpp#L376) | `HACK` | HACK HACK -- until we fix this. |


#### `dlls/h_battery.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [143](dlls/h_battery.cpp#L143) | `TODO` | TODO: useless, it's accessed earlier on. |
| [151](dlls/h_battery.cpp#L151) | `TODO` | TODO: put this check at the top. |


#### `dlls/handgrenade.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [71](dlls/handgrenade.cpp#L71) | `TODO` | TODO: not sure how useful this is given that the player has to have this weapon for this method to be called |


#### `dlls/headcrab.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [417](dlls/headcrab.cpp#L417) | `BUG` | BUGBUG: Why is this code here?  There is no ACT_RANGE_ATTACK2 animation.  I've disabled it for now. |


#### `dlls/healthkit.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [81](dlls/healthkit.cpp#L81) | `TODO` | TODO: incorrect check here, but won't respawn due to respawn delay being -1 in singleplayer |


#### `dlls/hgrunt_ally.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [573](dlls/hgrunt_ally.cpp#L573) | `TODO` | TODO: kinda odd that this doesn't use GetGunPosition like the original |
| [733](dlls/hgrunt_ally.cpp#L733) | `TODO` | TODO: disabled for ally |
| [1252](dlls/hgrunt_ally.cpp#L1252) | `TODO` | TODO: probably also needs this for head HGruntAllyHead::BeretBlack |


#### `dlls/hgrunt_medic.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [405](dlls/hgrunt_medic.cpp#L405) | `TODO` | TODO: probably the wrong logic, but it was in the original |
| [586](dlls/hgrunt_medic.cpp#L586) | `TODO` | TODO: kinda odd that this doesn't use GetGunPosition like the original |
| [746](dlls/hgrunt_medic.cpp#L746) | `TODO` | TODO: disabled for ally |
| [2937](dlls/hgrunt_medic.cpp#L2937) | `TODO` | TODO: missing from medic? |
| [2961](dlls/hgrunt_medic.cpp#L2961) | `TODO` | TODO: not suited for multiplayer |
| [3039](dlls/hgrunt_medic.cpp#L3039) | `TODO` | TODO: could just change the type of pTarget since this is the only type passed in |


#### `dlls/hgrunt_torch.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [551](dlls/hgrunt_torch.cpp#L551) | `TODO` | TODO: kinda odd that this doesn't use GetGunPosition like the original |
| [718](dlls/hgrunt_torch.cpp#L718) | `TODO` | TODO: disabled for ally |
| [2736](dlls/hgrunt_torch.cpp#L2736) | `BUG` | TODO: looks like a bug to me, shouldn't be bitwise inverting |


#### `dlls/male_assassin.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [829](dlls/male_assassin.cpp#L829) | `TODO` | TODO: why is this 556? is 762 too damaging? |
| [2022](dlls/male_assassin.cpp#L2022) | `TODO` | TODO: probably don't want this here, but it's still present in op4 |


#### `dlls/monsters.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [811](dlls/monsters.cpp#L811) | `TODO` | TODO: verify this this needs to be a comparison and not a bit check |
| [826](dlls/monsters.cpp#L826) | `BUG` | BUGBUG: this doesn't work 100% yet |


#### `dlls/monsters.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [114](dlls/monsters.h#L114) | `HACK` | #define bits_MEMORY_KILLED (1 << 7)		   // HACKHACK -- remember that I've already called my Killed() |


#### `dlls/multiplay_gamerules.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [358](dlls/multiplay_gamerules.cpp#L358) | `TODO` | TODO: really shouldn't be modifying this directly |
| [426](dlls/multiplay_gamerules.cpp#L426) | `FIXME` | FIXME:  Probably don't need to cast this just to read m_iDeaths |
| [974](dlls/multiplay_gamerules.cpp#L974) | `TODO` | TODO: make this go direct to console |


#### `dlls/nuclearbomb.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [217](dlls/nuclearbomb.cpp#L217) | `TODO` | TODO: set the classname members for both entities |


#### `dlls/op4mortar.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [188](dlls/op4mortar.cpp#L188) | `TODO` | TODO: never used? |


#### `dlls/otis.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [639](dlls/otis.cpp#L639) | `TODO` | TODO: Otis doesn't have a helmet, probably don't want his dome being bulletproof |


#### `dlls/pathcorner.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [358](dlls/pathcorner.cpp#L358) | `HACK` | if (dist == originalDist) // HACK -- up against a dead end |


#### `dlls/penguin_grenade.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [127](dlls/penguin_grenade.cpp#L127) | `TODO` | TODO: set to null earlier on, so this can never be valid |
| [253](dlls/penguin_grenade.cpp#L253) | `TODO` | TODO: shouldn't use index |


#### `dlls/pitdrone.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [153](dlls/pitdrone.cpp#L153) | `TODO` | TODO: maybe stick it on any entity that reports FL_WORLDBRUSH too? |
| [734](dlls/pitdrone.cpp#L734) | `TODO` | TODO: constrain value to valid range |


#### `dlls/pitworm_up.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [669](dlls/pitworm_up.cpp#L669) | `TODO` | TODO: maybe determine direction of velocity to apply? |
| [899](dlls/pitworm_up.cpp#L899) | `TODO` | TODO: missing an ApplyMultiDamage call here |
| [982](dlls/pitworm_up.cpp#L982) | `TODO` | TODO: never used? |


#### `dlls/plats.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [1766](dlls/plats.cpp#L1766) | `HACK` | HACKHACK -- This is bugly, but the train can actually stop moving at a different node depending on it's speed |


#### `dlls/player.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [406](dlls/player.cpp#L406) | `TODO` | TODO: this is a pretty bad way to handle damage increase |
| [1525](dlls/player.cpp#L1525) | `TODO` | TODO: Send HUD Update |
| [1642](dlls/player.cpp#L1642) | `BUG` | else if ((m_afButtonReleased & IN_USE) != 0 && (pObject->ObjectCaps() & FCAP_ONOFF_USE) != 0) // BUGBUG This is an "o... |
| [2131](dlls/player.cpp#L2131) | `HACK` | HACKHACK - Just look for the func_tracktrain classname |
| [2818](dlls/player.cpp#L2818) | `BUG` | BUG - this happens all the time in water, especially when |
| [2819](dlls/player.cpp#L2819) | `BUG` | BUG - water has current force |
| [3381](dlls/player.cpp#L3381) | `HACK` | HACK:	This variable is saved/restored in CBaseMonster as a time variable, but we're using it |
| [3927](dlls/player.cpp#L3927) | `TODO` | TODO: not given |
| [4076](dlls/player.cpp#L4076) | `FIXME` | FIXME: remove anyway for deathmatch testing |
| [4130](dlls/player.cpp#L4130) | `FIXME` | FIXME: remove anyway for deathmatch testing |
| [4442](dlls/player.cpp#L4442) | `HACK` | HACKHACK -- send the message to display the game title |
| [4443](dlls/player.cpp#L4443) | `TODO` | TODO: will not work properly in multiplayer |


#### `dlls/rope/CRope.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [632](dlls/rope/CRope.cpp#L632) | `TODO` | TODO move to common header - Solokiller |
| [819](dlls/rope/CRope.cpp#L819) | `TODO` | TODO: maybe only set/unset the nodraw flag |


#### `dlls/rope/CRopeSample.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [38](dlls/rope/CRopeSample.cpp#L38) | `TODO` | TODO: needed? |


#### `dlls/satchel.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [86](dlls/satchel.cpp#L86) | `HACK` | HACKHACK - On ground isn't always set, so look for ground underneath |


#### `dlls/schedule.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [83](dlls/schedule.cpp#L83) | `TODO` | TODO: not the correct way to check for missing classname, is like this in vanilla Op4 |


#### `dlls/scientist.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [354](dlls/scientist.cpp#L354) | `FIXME` | {TASK_SET_ACTIVITY, (float)ACT_CROUCHIDLE}, // FIXME: This looks lame |


#### `dlls/scripted.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [667](dlls/scripted.cpp#L667) | `BUG` | BUGBUG -- This doesn't call Killed() |
| [871](dlls/scripted.cpp#L871) | `BUG` | SetUse(NULL);	// BUGBUG -- This doesn't call Killed() |


#### `dlls/shockroach.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [443](dlls/shockroach.cpp#L443) | `BUG` | BUGBUG: Why is this code here?  There is no ACT_RANGE_ATTACK2 animation.  I've disabled it for now. |


#### `dlls/shocktrooper.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [345](dlls/shocktrooper.cpp#L345) | `TODO` | TODO: change body group |
| [1138](dlls/shocktrooper.cpp#L1138) | `TODO` | TODO: the directory names should be lowercase |


#### `dlls/sound.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [156](dlls/sound.cpp#L156) | `HACK` | HACKHACK - This is not really in the spirit of the save/restore design, but save this |
| [625](dlls/sound.cpp#L625) | `HACK` | HACKHACK - this makes the code in Precache() work properly after a save/restore |


#### `dlls/squeakgrenade.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [501](dlls/squeakgrenade.cpp#L501) | `HACK` | HACK HACK:  Ugly hacks to handle change in origin based on new physics code for players |


#### `dlls/teamplay_gamerules.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [29](dlls/teamplay_gamerules.cpp#L29) | `TODO` | TODO: these should be members of CHalfLifeTeamplay |


#### `dlls/tentacle.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [75](dlls/tentacle.cpp#L75) | `TODO` | TODO: should override base, but has different signature |


#### `dlls/triggers.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [1024](dlls/triggers.cpp#L1024) | `HACK` | HACKHACK -- In multiplayer, players touch this based on packet receipt. |
| [1042](dlls/triggers.cpp#L1042) | `BUG` | BUGBUG - There can be only 32 players! |
| [1060](dlls/triggers.cpp#L1060) | `BUG` | BUGBUG - There can be only 32 players! |
| [2592](dlls/triggers.cpp#L2592) | `TODO` | TODO: not made for multiplayer |
| [2623](dlls/triggers.cpp#L2623) | `TODO` | TODO: this needs to be removed in order to function |
| [2888](dlls/triggers.cpp#L2888) | `TODO` | TODO: constrain team_no input to valid values |


#### `dlls/tripmine.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [97](dlls/tripmine.cpp#L97) | `TODO` | TODO: define constant |
| [261](dlls/tripmine.cpp#L261) | `HACK` | HACKHACK Set simple box using this really nice global! |
| [371](dlls/tripmine.cpp#L371) | `HACK` | HACK: force the body to the first person view by default so it doesn't show up as a huge tripmine for a second. |


#### `dlls/turret.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [22](dlls/turret.cpp#L22) | `TODO` | TODO: |


#### `dlls/util.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [1412](dlls/util.cpp#L1412) | `TODO` | TODO: define constants |
| [2579](dlls/util.cpp#L2579) | `TODO` | TODO: need to refactor save game stuff to make this cleaner and reusable |


#### `dlls/voltigore.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [476](dlls/voltigore.cpp#L476) | `TODO` | TODO: use a filter based on attacker to identify self harm |


#### `dlls/weapons.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [1033](dlls/weapons.cpp#L1033) | `TODO` | TODO: should handle -1 return as well (only return true if ammo was taken) |


#### `dlls/weapons.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [787](dlls/weapons.h#L787) | `TODO` | int m_fInReload; //TODO: not used, remove |


#### `dlls/weapons/CDisplacerBall.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [35](dlls/weapons/CDisplacerBall.cpp#L35) | `TODO` | TODO: can probably be smarter - Solokiller |
| [320](dlls/weapons/CDisplacerBall.cpp#L320) | `TODO` | TODO: no next think? - Solokiller |


#### `dlls/weapons/CEagle.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [370](dlls/weapons/CEagle.cpp#L370) | `TODO` | TODO: could probably use a better model |


#### `dlls/weapons/CGrappleTip.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [33](dlls/weapons/CGrappleTip.cpp#L33) | `TODO` | TODO: this should be handled differently. A method that returns an overall size, another whether it's fixed, etc. - S... |
| [165](dlls/weapons/CGrappleTip.cpp#L165) | `TODO` | TODO: should probably clamp at sv_maxvelocity to prevent the tip from going off course. - Solokiller |


#### `dlls/weapons/CKnife.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [167](dlls/weapons/CKnife.cpp#L167) | `TODO` | TODO: This code assumes the target is a player and not some NPC. Rework it to support NPC backstabbing. |


#### `dlls/weapons/CPenguin.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [24](dlls/weapons/CPenguin.cpp#L24) | `TODO` | TODO: this isn't in vanilla Op4 so it won't save properly there |


#### `dlls/weapons/CSniperRifle.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [142](dlls/weapons/CSniperRifle.cpp#L142) | `TODO` | TODO: 8192 constant should be defined somewhere - Solokiller |
| [165](dlls/weapons/CSniperRifle.cpp#L165) | `TODO` | TODO: this doesn't really make sense |


### `engine/`


#### `engine/studio.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [28](engine/studio.h#L28) | `TODO` | #define MAXSTUDIOTRIANGLES 20000 // TODO: tune this |
| [29](engine/studio.h#L29) | `TODO` | #define MAXSTUDIOVERTS 2048		 // TODO: tune this |


### `external/`


#### `external/SDL2/SDL_config_winrt.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [124](external/SDL2/SDL_config_winrt.h#L124) | `TODO` | #define HAVE__STRLWR 1	// TODO, WinRT: consider using _strlwr_s instead |
| [128](external/SDL2/SDL_config_winrt.h#L128) | `TODO` | #define HAVE_ITOA 1   // TODO, WinRT: consider using _itoa_s instead |
| [129](external/SDL2/SDL_config_winrt.h#L129) | `TODO` | #define HAVE__LTOA 1	// TODO, WinRT: consider using _ltoa_s instead |
| [130](external/SDL2/SDL_config_winrt.h#L130) | `TODO` | #define HAVE__ULTOA 1	// TODO, WinRT: consider using _ultoa_s instead |
| [142](external/SDL2/SDL_config_winrt.h#L142) | `TODO` | #define HAVE_SSCANF 1	// TODO, WinRT: consider using sscanf_s instead |


### `pm_shared/`


#### `pm_shared/pm_math.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [285](pm_shared/pm_math.cpp#L285) | `FIXME` | length = sqrt(length); // FIXME |
| [302](pm_shared/pm_math.cpp#L302) | `FIXME` | length = sqrt(length); // FIXME |


#### `pm_shared/pm_shared.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [301](pm_shared/pm_shared.cpp#L301) | `FIXME` | FIXME mp_footsteps needs to be a movevar |
| [313](pm_shared/pm_shared.cpp#L313) | `FIXME` | FIXME, move to player state |
| [1489](pm_shared/pm_shared.cpp#L1489) | `FIXME` | if (0 == trace.startsolid && 0 == trace.allsolid) // FIXME: check steep slope? |
| [1806](pm_shared/pm_shared.cpp#L1806) | `TODO` | TODO: not really necessary to have separate arrays for client and server since the code is separate anyway. |
| [2142](pm_shared/pm_shared.cpp#L2142) | `HACK` | HACKHACK - Fudge for collision bug - no time to fix this properly |
| [2657](pm_shared/pm_shared.cpp#L2657) | `HACK` | HACK HACK HACK |


### `utils/`


#### `utils/common/cmdlib.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [833](utils/common/cmdlib.cpp#L833) | `FIXME` | FIXME: byte swap? |


#### `utils/common/mathlib.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [31](utils/common/mathlib.cpp#L31) | `FIXME` | length = sqrt(length); // FIXME |
| [270](utils/common/mathlib.cpp#L270) | `FIXME` | FIXME: rescale the inputs to 1/2 angle |


#### `utils/common/wadlib.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [296](utils/common/wadlib.cpp#L296) | `FIXME` | FIXME: do compression |


#### `utils/makefont/makefont.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [305](utils/makefont/makefont.cpp#L305) | `FIXME` | FIXME:  Enable true palette support in engine? |
| [333](utils/makefont/makefont.cpp#L333) | `FIXME` | FIXME:  Enable true palette support in engine? |


#### `utils/mdlviewer/studio_render.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [513](utils/mdlviewer/studio_render.cpp#L513) | `TODO` | TODO: only do it for bones that actually have textures |


#### `utils/qbsp2/bsp5.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [61](utils/qbsp2/bsp5.h#L61) | `FIXME` | vec3_t pts[MAXEDGES]; // FIXME: change to use winding_t |


#### `utils/qrad/lightmap.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [859](utils/qrad/lightmap.cpp#L859) | `BUG` | VectorFill(p->totallight, 0); // all sent now  // BUGBUG for progressive refinement runs |
| [1490](utils/qrad/lightmap.cpp#L1490) | `BUG` | vec3_t v; // BUGBUG: Use a weighted average instead? |


#### `utils/qrad/qrad.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [314](utils/qrad/qrad.cpp#L314) | `FIXME` | sum[i] += mt[][x][y][i] // FIXME later |


#### `utils/serverctrl/ServerCtrlDlg.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [273](utils/serverctrl/ServerCtrlDlg.cpp#L273) | `TODO` | TODO: Add extra initialization here |
| [474](utils/serverctrl/ServerCtrlDlg.cpp#L474) | `FIXME` | FIXME:  You'll want to fill in your executable path here, of course. |


#### `utils/sprgen/spritegn.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [61](utils/sprgen/spritegn.h#L61) | `TODO` | TODO: shorten these? |


#### `utils/studiomdl/studiomdl.cpp`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [1486](utils/studiomdl/studiomdl.cpp#L1486) | `FIXME` | FIXME losing texture coord resultion! |
| [1647](utils/studiomdl/studiomdl.cpp#L1647) | `TODO` | TODO: process the texture and flag it if fullbright or transparent are used. |
| [1648](utils/studiomdl/studiomdl.cpp#L1648) | `TODO` | TODO: only save as many palette entries as are actually used. |
| [1836](utils/studiomdl/studiomdl.cpp#L1836) | `FIXME` | FIXME : Hey, it's orthogical so inv(A) == transpose(A) |


#### `utils/studiomdl/studiomdl.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [290](utils/studiomdl/studiomdl.h#L290) | `FIXME` | FIXME: what about texture overrides inline with loading models |


#### `utils/vgui/include/VGUI.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [44](utils/vgui/include/VGUI.h#L44) | `TODO` | TODO: Look and Feel support |
| [57](utils/vgui/include/VGUI.h#L57) | `TODO` | TODO: Figure out the 'Valve' Look and Feel and implement that instead of a the Java one |
| [58](utils/vgui/include/VGUI.h#L58) | `TODO` | TODO: Determine ownership policy for Borders, Layouts, etc.. |
| [59](utils/vgui/include/VGUI.h#L59) | `TODO` | TODO: tooltips support |
| [60](utils/vgui/include/VGUI.h#L60) | `TODO` | TODO: ComboKey (hot key support) |
| [61](utils/vgui/include/VGUI.h#L61) | `TODO` | TODO: add Background.cpp, remove paintBackground from all components |
| [64](utils/vgui/include/VGUI.h#L64) | `TODO` | TODO: Builtin components should never overide paintBackground, only paint |
| [65](utils/vgui/include/VGUI.h#L65) | `TODO` | TODO: All protected members should be converted to private |
| [66](utils/vgui/include/VGUI.h#L66) | `TODO` | TODO: All member variables should be moved to the top of the class prototype |
| [67](utils/vgui/include/VGUI.h#L67) | `TODO` | TODO: All private methods should be prepended with private |
| [68](utils/vgui/include/VGUI.h#L68) | `TODO` | TODO: Use of word internal in method names is not consistent and confusing |
| [69](utils/vgui/include/VGUI.h#L69) | `TODO` | TODO: Cleanup so bullshit publics are properly named, maybe even figure out |
| [71](utils/vgui/include/VGUI.h#L71) | `TODO` | TODO: Breakup InputSignal into logical pieces |
| [72](utils/vgui/include/VGUI.h#L72) | `TODO` | TODO: Button is in a state of disarray, it should have ButtonModel support |
| [73](utils/vgui/include/VGUI.h#L73) | `TODO` | TODO: get rid of all the stupid strdup laziness, convert to vgui_strdup |
| [74](utils/vgui/include/VGUI.h#L74) | `TODO` | TODO: actually figure out policy on String and implement it consistently |
| [75](utils/vgui/include/VGUI.h#L75) | `TODO` | TODO: implement createLayoutInfo for other Layouts than need it |
| [76](utils/vgui/include/VGUI.h#L76) | `TODO` | TODO: BorderLayout should have option for a null LayoutInfo defaulting to center |
| [77](utils/vgui/include/VGUI.h#L77) | `TODO` | TODO: SurfaceBase should go away, put it in Surface |
| [78](utils/vgui/include/VGUI.h#L78) | `TODO` | TODO: ActionSignals and other Signals should just set a flag when they fire. |
| [80](utils/vgui/include/VGUI.h#L80) | `TODO` | TODO: Change all method naming to starting with a capital letter. |


#### `utils/vgui/include/VGUI_App.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [32](utils/vgui/include/VGUI_App.h#L32) | `TODO` | TODO: the public and public bullshit are all messed up, need to organize |
| [33](utils/vgui/include/VGUI_App.h#L33) | `TODO` | TODO: actually all of the access needs to be properly thought out while you are at it |


#### `utils/vgui/include/VGUI_Border.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [13](utils/vgui/include/VGUI_Border.h#L13) | `TODO` | TODO: all borders should be titled |


#### `utils/vgui/include/VGUI_Button.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [22](utils/vgui/include/VGUI_Button.h#L22) | `TODO` | TODO: Button should be derived from an AbstractButton |


#### `utils/vgui/include/VGUI_Color.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [13](utils/vgui/include/VGUI_Color.h#L13) | `TODO` | TODO: rename getColor(r,g,b,a) to getRGBA(r,g,b,a) |
| [14](utils/vgui/include/VGUI_Color.h#L14) | `TODO` | TODO: rename setColor(r,g,b,a) to setRGBA(r,g,b,a) |
| [15](utils/vgui/include/VGUI_Color.h#L15) | `TODO` | TODO: rename getColor(sc) to getSchemeColor(sc) |
| [16](utils/vgui/include/VGUI_Color.h#L16) | `TODO` | TODO: rename setColor(sc) to setSchemeColor(sc) |


#### `utils/vgui/include/VGUI_FileInputStream.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [10](utils/vgui/include/VGUI_FileInputStream.h#L10) | `TODO` | TODO : figure out how to get stdio out of here, I think std namespace is broken for FILE for forward declaring does n... |


#### `utils/vgui/include/VGUI_Font.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [17](utils/vgui/include/VGUI_Font.h#L17) | `TODO` | TODO: cursors and fonts should work like gl binds |


#### `utils/vgui/include/VGUI_Image.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [14](utils/vgui/include/VGUI_Image.h#L14) | `TODO` | TODO:: needs concept of insets |


#### `utils/vgui/include/VGUI_Label.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [15](utils/vgui/include/VGUI_Label.h#L15) | `TODO` | TODO: this should use a TextImage for the text |


#### `utils/vgui/include/VGUI_ListPanel.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [19](utils/vgui/include/VGUI_ListPanel.h#L19) | `TODO` | TODO: make a ScrollPanel and use a constrained one for _vpanel in ListPanel |


#### `utils/vgui/include/VGUI_TextImage.h`

| Zeile | Typ | Kommentar |
|-------|-----|-----------|
| [14](utils/vgui/include/VGUI_TextImage.h#L14) | `TODO` | TODO: need to add wrapping flag instead of being arbitrary about when wrapping and auto-resizing actually happens |
