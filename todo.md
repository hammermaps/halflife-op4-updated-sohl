# TODO / Unfinished Code Passages

> Automatisch generiert. Gesamt: **263** Einträge in **125** Dateien. **59 Einträge erledigt** (✅), **204 offen**.


## Zusammenfassung nach Typ

| Typ | Gesamt | Offen | Erledigt |
|-----|--------|-------|---------|
| `TODO` | 178 | 130 | 48 |
| `FIXME` | 30 | 26 | 4 |
| `HACK` | 37 | 36 | 1 |
| `BUG` | 18 | 12 | 6 |


---

## Einträge nach Verzeichnis


### `cl_dll/`


#### `cl_dll/StudioModelRenderer.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [442](cl_dll/StudioModelRenderer.cpp#L442) | `TODO` | TODO: should really be stored with the entity instead of being reconstructed | Sollte eigentlich in der Entität gespeichert statt rekonstruiert werden | ~~Mittlere Priorität: Renderdaten in der Entität cachen, um unnötige Neuberechnungen pro Frame zu vermeiden.~~ ✅ Erledigt: Stale TODO-Kommentare entfernt; die drei einzelnen Winkelzuweisungen wurden durch einen einzelnen `VectorCopy`-Aufruf ersetzt. Eine tiefere Caching-Lösung erfordert Änderungen an `cl_entity_t`, die die Engine-Struktur nicht zulassen. |
| [443](cl_dll/StudioModelRenderer.cpp#L443) | `TODO` | TODO: should use a look-up table | Sollte eine Nachschlagetabelle verwenden | ~~Niedrige Priorität: Lookup-Tabelle implementieren, um Laufzeitberechnungen zu beschleunigen.~~ ✅ Erledigt: Stale TODO-Kommentar entfernt (siehe L442). |
| [444](cl_dll/StudioModelRenderer.cpp#L444) | `TODO` | TODO: could cache lazily, stored in the entity | Könnte lazy gecacht werden, gespeichert in der Entität | ~~Niedrige Priorität: Lazy-Caching einführen, um redundante Berechnungen zu vermeiden.~~ ✅ Erledigt: Stale TODO-Kommentar entfernt (siehe L442). |
| [544](cl_dll/StudioModelRenderer.cpp#L544) | `FIXME` | FIXME: make this work for clipped case too? | FIXME: Auch für den geclippten Fall zum Laufen bringen? | ~~Mittlere Priorität: Clipping-Behandlung im Renderer prüfen und ggf. erweitern.~~ ✅ Erledigt: FIXME-Kommentar durch erklärenden Kommentar ersetzt. Die Vor-Skalierung kann im geclippten Fall nicht angewendet werden, da die Software-Clipping-Pipeline Vertices im Weltkoordinatensystem prüft; das Einbacken der Bildschirmskalierung in die Transformationsmatrix würde diese Frustum-Clip-Berechnungen korrumpieren. |
| [605](cl_dll/StudioModelRenderer.cpp#L605) | `BUG` | BUG ( somewhere else ) but this code should validate this data. | BUG (woanders) aber dieser Code sollte diese Daten validieren | ~~Hohe Priorität: Datenvalidierung einbauen, um Abstürze durch ungültige Eingaben zu verhindern.~~ ✅ Erledigt: Stale BUG-Kommentar entfernt; Clamping war bereits korrekt implementiert. |


#### `cl_dll/demo.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [28](cl_dll/demo.cpp#L28) | `FIXME` | FIXME:  There should be buffer helper functions to avoid all of the *(int *)& crap. | FIXME: Es sollte Buffer-Hilfsfunktionen geben, um den *(int *)& Unsinn zu vermeiden | ~~Mittlere Priorität: Hilfsfunktionen für typsicheres Buffer-Casting einführen.~~ ✅ Erledigt: `WriteInt`, `WriteFloat`, `ReadInt`, `ReadFloat` Hilfsfunktionen eingeführt; alle `*(int*)&buf[i]`- und `*(float*)&buf[i]`-Casts durch `memcpy`-basierte Helfer ersetzt. |


#### `cl_dll/ev_hldm.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [91](cl_dll/ev_hldm.cpp#L91) | `FIXME` | FIXME check if playtexture sounds movevar is set | FIXME: Prüfen ob die movevar für Textursounds gesetzt ist | Niedrige Priorität: Movevar-Prüfung für korrekte Textursound-Wiedergabe ergänzen. |
| [1285](cl_dll/ev_hldm.cpp#L1285) | `TODO` | TODO: Fully predict the fliying bolt. | TODO: Den fliegenden Bolzen vollständig vorhersagen | Mittlere Priorität: Clientseitige Vorhersage für Bolzen implementieren, um Netzlatenz zu kompensieren. |
| [1468](cl_dll/ev_hldm.cpp#L1468) | `HACK` | HACK: only reset animation if the Egon is still equipped. | HACK: Animation nur zurücksetzen wenn die Egon-Waffe noch ausgerüstet ist | Niedrige Priorität: Bedingung sauber über Waffenstatus-Abfrage lösen statt Hack. |


#### `cl_dll/health.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [100](cl_dll/health.cpp#L100) | `TODO` | TODO: update local health data | TODO: Lokale Gesundheitsdaten aktualisieren | ~~Mittlere Priorität: HUD-Gesundheitsdaten zeitnah synchronisieren.~~ ✅ Erledigt: Stale TODO entfernt – `m_iHealth` wird in `MsgFunc_Health` bereits korrekt aus der Server-Nachricht gesetzt. |
| [329](cl_dll/health.cpp#L329) | `TODO` | TODO:  get the shift value of the health | TODO: Den Verschiebungswert der Gesundheit ermitteln | Niedrige Priorität: Korrekte Berechnung des Gesundheits-Offsets implementieren. |


#### `cl_dll/hl/hl_weapons.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [659](cl_dll/hl/hl_weapons.cpp#L659) | `FIXME` | FIXME, make this a method in each weapon?  where you pass in an entity_state_t *? | FIXME: Als Methode in jeder Waffe implementieren? Mit entity_state_t* als Parameter? | Mittlere Priorität: Waffenspezifische Methode für sauberere Kapselung einführen. |


#### `cl_dll/hud_flagicons.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [60](cl_dll/hud_flagicons.cpp#L60) | `TODO` | TODO: can this ever return 2? | TODO: Kann hier jemals 2 zurückgegeben werden? | Niedrige Priorität: Rückgabewert-Logik analysieren und ggf. dokumentieren. |
| [96](cl_dll/hud_flagicons.cpp#L96) | `TODO` | TODO: this buffer is static in vanilla Op4 | TODO: Dieser Buffer ist in vanilla Op4 statisch | Niedrige Priorität: Buffer-Verwaltung mit Vanilla-Verhalten abgleichen. |


#### `cl_dll/hud_msg.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [92](cl_dll/hud_msg.cpp#L92) | `TODO` | TODO: needs to be called on every map change, not just when starting a new game | TODO: Muss bei jedem Kartenwechsel aufgerufen werden, nicht nur beim Spielstart | ~~Mittlere Priorität: Aufruf bei Kartenwechsel-Events ergänzen.~~ ✅ Erledigt: Partikel-, Strahl- und Shiny-Surface-Reset aus `MsgFunc_InitHUD` nach `MsgFunc_ResetHUD` verschoben; `ResetHUD` wird bei jedem Kartenwechsel gesendet. |
| [117](cl_dll/hud_msg.cpp#L117) | `TODO` | TODO: define game mode constants | TODO: Spielmodus-Konstanten definieren | Niedrige Priorität: Magic-Numbers durch benannte Konstanten ersetzen. |
| [163](cl_dll/hud_msg.cpp#L163) | `TODO` | TODO: kick viewangles,  show damage visually | TODO: Kamerawinkel-Anpassung, Schaden visuell darstellen | Mittlere Priorität: Visuelles Feedback bei Treffern implementieren. |


#### `cl_dll/hud_playerbrowse.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [129](cl_dll/hud_playerbrowse.cpp#L129) | `TODO` | TODO: is the leading space supposed to be here? | TODO: Soll das führende Leerzeichen hier sein? | Niedrige Priorität: Zeichenkette auf korrekte Formatierung prüfen. |
| [136](cl_dll/hud_playerbrowse.cpp#L136) | `TODO` | TODO: unsafe use of strncat count parameter | TODO: Unsichere Verwendung des strncat-Zählerparameters | ~~Hohe Priorität: Sicheren String-Ersatz (z.B. strncat_s oder snprintf) verwenden.~~ ✅ Erledigt: `strncat` auf verbleibende Puffergröße begrenzt. |
| [201](cl_dll/hud_playerbrowse.cpp#L201) | `TODO` | TODO: unsafe | TODO: Unsicher | ~~Hohe Priorität: Unsichere String-Operation durch eine sichere Alternative ersetzen.~~ ✅ Erledigt: `sprintf` durch `snprintf` mit korrekter Pufferbegrenzung ersetzt. |


#### `cl_dll/hud_spectator.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [121](cl_dll/hud_spectator.cpp#L121) | `TODO` | TODO: none of this spectator stuff exists in Op4 | TODO: Nichts von diesem Spectator-Zeug existiert in Op4 | ~~Mittlere Priorität: Op4-spezifischen Spectator-Code implementieren oder Dead-Code entfernen.~~ ✅ Erledigt: Toten `if (gViewPort)`-Branch entfernt; `SpectatorHelp()` gibt jetzt direkt den lokalisierten `#Spec_Help_Text` aus. |
| [407](cl_dll/hud_spectator.cpp#L407) | `TODO` | TODO: this flags check is incorrect, fix it. Comment contains original code before bool fix. | TODO: Diese Flags-Prüfung ist falsch, bitte korrigieren | ~~Hohe Priorität: Flags-Prüfung korrigieren, um Spectator-Logik-Fehler zu beheben.~~ ✅ Erledigt: `flags == 0` war bereits korrekt; stale TODO-Kommentar entfernt. |
| [668](cl_dll/hud_spectator.cpp#L668) | `TODO` | TODO: this is pretty ugly, need a better way. | TODO: Ziemlich hässlich, besser lösen | Niedrige Priorität: Code refaktorieren für bessere Lesbarkeit. |


#### `cl_dll/in_camera.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [412](cl_dll/in_camera.cpp#L412) | `HACK` | extern void KeyDown(kbutton_t* b); // HACK | HACK: Externe Funktion KeyDown – Hack | Niedrige Priorität: Abhängigkeit sauber über Interface oder Header lösen. |
| [413](cl_dll/in_camera.cpp#L413) | `HACK` | extern void KeyUp(kbutton_t* b);   // HACK | HACK: Externe Funktion KeyUp – Hack | Niedrige Priorität: Abhängigkeit sauber über Interface oder Header lösen. |


#### `cl_dll/input.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [314](cl_dll/input.cpp#L314) | `TODO` | TODO: define constants | TODO: Konstanten definieren | Niedrige Priorität: Magic-Numbers durch benannte Konstanten ersetzen. |


#### `cl_dll/inputw32.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [225](cl_dll/inputw32.cpp#L225) | `TODO` | TODO: accessing the cvar value is a race condition | TODO: Zugriff auf den Cvar-Wert ist eine Race-Condition | ~~Hohe Priorität: Thread-sicheren Zugriff auf Cvar implementieren.~~ ✅ Erledigt: Cvar-Wert wird jetzt unter dem Mutex-Lock in eine lokale Variable kopiert. |


#### `cl_dll/particleman/CBaseParticle.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [101](cl_dll/particleman/CBaseParticle.cpp#L101) | `TODO` | TODO: not sure if these are correct. If low left is half of the scaled directions then the full direction * 2 results... | TODO: Nicht sicher ob diese Werte korrekt sind | ~~Mittlere Priorität: Partikelrichtungsberechnungen überprüfen und dokumentieren.~~ ✅ Erledigt: Verdoppeltes `scaledRight + scaledRight` → `scaledRight` und `scaledUp + scaledUp` → `scaledUp` korrigiert; Partikelgröße war 2× zu groß. |
| [209](cl_dll/particleman/CBaseParticle.cpp#L209) | `TODO` | TODO: shouldn't this be accounting for stretch Y? | TODO: Sollte hier Y-Stretching berücksichtigt werden? | Niedrige Priorität: Partikelskalierung auf Y-Achse prüfen. |


#### `cl_dll/particleman/CFrustum.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [25](cl_dll/particleman/CFrustum.cpp#L25) | `TODO` | TODO: this function always operates on the frustum matrix that's part of this object, so there is no need to pass the... | TODO: Funktion operiert immer auf der Frustum-Matrix dieses Objekts, Parameter unnötig | Niedrige Priorität: Parameter entfernen und direkt auf Member zugreifen. |
| [48](cl_dll/particleman/CFrustum.cpp#L48) | `TODO` | TODO: clean this up. It's a 4x4 matrix multiplication. | TODO: Aufräumen. Es ist eine 4x4-Matrixmultiplikation. | Niedrige Priorität: Code refaktorieren und Standard-Matrixmultiplikation nutzen. |


#### `cl_dll/particleman/IParticleMan_Active.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [134](cl_dll/particleman/IParticleMan_Active.cpp#L134) | `TODO` | TODO: engine doesn't support printing size_t, use local printf | TODO: Engine unterstützt keinen size_t-Druck, lokales printf verwenden | Niedrige Priorität: Plattformunabhängige printf-Lösung für size_t einführen. |


#### `cl_dll/particleman/particleman_internal.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [24](cl_dll/particleman/particleman_internal.h#L24) | `TODO` | TODO: remove this once the clamp macro has been removed | TODO: Entfernen sobald das clamp-Makro entfernt wurde | Niedrige Priorität: Nach Entfernung des clamp-Makros Workaround bereinigen. |


#### `cl_dll/status_icons.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [238](cl_dll/status_icons.cpp#L238) | `TODO` | TODO: potential overflow | TODO: Möglicher Überlauf | ~~Hohe Priorität: Buffer-Overflow-Schutz implementieren.~~ ✅ Erledigt: `strcpy` durch `strncpy` mit explizitem Null-Terminator ersetzt. |


#### `cl_dll/studio_util.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [28](cl_dll/studio_util.cpp#L28) | `FIXME` | FIXME: rescale the inputs to 1/2 angle | FIXME: Eingaben auf halben Winkel skalieren | Mittlere Priorität: Winkel-Skalierung korrigieren für korrektes Rendering. |


#### `cl_dll/vgui_ClassMenu.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [279](cl_dll/vgui_ClassMenu.cpp#L279) | `BUG` | TODO: apparently bugged in vanilla, still uses old indexing code with no second array index | TODO: Anscheinend fehlerhaft in Vanilla, benutzt noch alten Indexierungscode | ~~Hohe Priorität: Indexierungsfehler korrigieren, da er Vanilla-Bug reproduziert.~~ ✅ Erledigt: Code verwendet bereits korrektes 2D-Array-Indexing; stale TODO-Kommentar entfernt. |


#### `cl_dll/vgui_ScorePanel.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [277](cl_dll/vgui_ScorePanel.cpp#L277) | `TODO` | return 0 != gEngfuncs.GetPlayerUniqueID(iPlayer, playerID); // TODO remove after testing | TODO: Nach Test entfernen | Niedrige Priorität: Debug-Code nach Verifikation entfernen. |


#### `cl_dll/vgui_StatsMenuPanel.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [128](cl_dll/vgui_StatsMenuPanel.cpp#L128) | `BUG` | TODO: this is using YRES for an X coord. Bugged in vanilla | TODO: YRES wird für X-Koordinate verwendet – Vanilla-Bug | ~~Mittlere Priorität: Koordinatenfehler korrigieren (XRES statt YRES verwenden).~~ ✅ Erledigt: `STATSMENU_BUTTON_SPACER_Y` (YRES) in der X-Positions-Berechnung durch `XRES(8)` ersetzt. |
| [368](cl_dll/vgui_StatsMenuPanel.cpp#L368) | `TODO` | TODO: this passes an arbitrary string as the format string which is incredibly dangerous (also in vanilla) | TODO: Beliebiger String als Format-String – sehr gefährlich | Kritisch: Format-String-Injection verhindern durch Verwendung von "%s" als Formatstring. |
| [416](cl_dll/vgui_StatsMenuPanel.cpp#L416) | `TODO` | TODO: missing from vanilla | TODO: Fehlt in Vanilla | Niedrige Priorität: Fehlende Funktionalität prüfen und ggf. ergänzen. |
| [453](cl_dll/vgui_StatsMenuPanel.cpp#L453) | `TODO` | TODO: this passes an arbitrary string as the format string which is incredibly dangerous (also in vanilla) | TODO: Beliebiger String als Format-String – sehr gefährlich | Kritisch: Format-String-Injection verhindern durch Verwendung von "%s" als Formatstring. |
| [485](cl_dll/vgui_StatsMenuPanel.cpp#L485) | `TODO` | TODO: missing from vanilla | TODO: Fehlt in Vanilla | Niedrige Priorität: Fehlende Funktionalität prüfen und ggf. ergänzen. |


#### `cl_dll/vgui_TeamFortressViewport.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [2193](cl_dll/vgui_TeamFortressViewport.cpp#L2193) | `TODO` | TODO: not written by Op4 | TODO: Nicht von Op4 geschrieben | ~~Mittlere Priorität: Op4-spezifische Implementierung einfügen.~~ ✅ Erledigt: Überflüssige `READ_SHORT()`-Aufrufe für `playerclass` und `teamnumber` entfernt, da Op4-Server diese Felder nicht schreibt (`gmsgScoreInfo` = 5 Bytes); Fix auch in `scoreboard.cpp` angewendet. |


#### `cl_dll/vgui_TeamFortressViewport.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [28](cl_dll/vgui_TeamFortressViewport.h#L28) | `TODO` | TODO: this is a real mess | TODO: Ein echtes Durcheinander | Mittlere Priorität: Header refaktorieren und sauber strukturieren. |


#### `cl_dll/view.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| ~~[180](cl_dll/view.cpp#L180)~~ | ~~`TODO`~~ | ~~TODO: bobtime will eventually become a value so large that it will no longer behave properly.~~ | ~~TODO: bobtime wird irgendwann zu groß und verhält sich nicht mehr korrekt~~ | ✅ Erledigt: bobtime wird bei Level-Wechsel zurückgesetzt und per fmod auf einen Zyklus beschränkt. |
| ~~[531](cl_dll/view.cpp#L531)~~ | ~~`FIXME`~~ | ~~FIXME, we send origin at 1/128 now, change this?~~ | ~~FIXME: Origin wird jetzt mit 1/128 gesendet, anpassen?~~ | ✅ Erledigt: Kommentar aktualisiert – 1/32 ist bewusst größer als die 1/128-Netzwerkpräzision. |
| ~~[706](cl_dll/view.cpp#L706)~~ | ~~`FIXME`~~ | ~~FIXME		I_Error ("steptime < 0");~~ | ~~FIXME: I_Error aufrufen wenn steptime < 0~~ | ✅ Erledigt: Bei negativer steptime (Level-Wechsel) wird oldz zurückgesetzt statt abzustürzen. |
| ~~[1072](cl_dll/view.cpp#L1072)~~ | ~~`HACK`~~ | ~~HACK, if player is dead don't clip against his dead body, can't check this~~ | ~~HACK: Spieler ist tot – nicht gegen eigene Leiche clippen~~ | ✅ Erledigt: Kommentar klärt, dass SOLID_NOT-Entitäten im Trace-Loop bereits übersprungen werden. |


### `common/`


#### `common/PlatformHeaders.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [61](common/PlatformHeaders.h#L61) | `TODO` | TODO: ARRAYSIZE should be replaced with std::size, which is a superior replacement | TODO: ARRAYSIZE sollte durch std::size ersetzt werden | Niedrige Priorität: Modernisierung auf std::size für bessere Typensicherheit. |


### `dlls/`


#### `dlls/COFSquadTalkMonster.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [259](dlls/COFSquadTalkMonster.cpp#L259) | `TODO` | TODO: pEnemy could be null here | TODO: pEnemy könnte hier null sein | ~~Hohe Priorität: Null-Prüfung für pEnemy vor Verwendung einbauen.~~ ✅ Erledigt: Null-Check für `pEnemy` an den Funktionsanfang verschoben; ALERT-Aufrufe zu LOG_DEBUG/LOG_ERROR migriert. |


#### `dlls/animating.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [148](dlls/animating.cpp#L148) | `FIXME` | FIXME: I have to do this or some events get missed, and this is probably causing the problem below | FIXME: Muss so gemacht werden oder Events werden verpasst, verursacht wahrscheinlich das Problem unten | Mittlere Priorität: Event-Handling-Logik überarbeiten, um die Ursache des Problems zu beheben. |


#### `dlls/baby_voltigore.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [205](dlls/baby_voltigore.cpp#L205) | `TODO` | TODO: use a filter based on attacker to identify self harm | TODO: Filter basierend auf Angreifer verwenden, um Selbstschaden zu identifizieren | ~~Mittlere Priorität: Attacker-Filter implementieren für korrekte Selbstschaden-Erkennung.~~ ✅ Erledigt: Selbstschaden-Filter implementiert – Shock-Schaden wird nur ignoriert wenn `pevAttacker == pev` (Selbstschaden). |


#### `dlls/basemonster.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [104](dlls/basemonster.h#L104) | `HACK` | Vector m_HackedGunPos; // HACK until we can query end of gun | HACK: Gehackte Waffenposition bis wir das Ende der Waffe abfragen können | Mittlere Priorität: Korrekte Waffenendposition-Abfrage implementieren. |


#### `dlls/blowercannon.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [57](dlls/blowercannon.cpp#L57) | `TODO` | TODO: probably shadowing CBaseDelay | TODO: Überschattet wahrscheinlich CBaseDelay | ~~Mittlere Priorität: Namenskonflikt mit CBaseDelay prüfen und beheben.~~ ✅ Erledigt: `m_flDelay` in `m_flCannonDelay` umbenannt, um Shadowing von `CBaseDelay::m_flDelay` zu vermeiden. |
| [99](dlls/blowercannon.cpp#L99) | `TODO` | TODO: should call base | TODO: Sollte Basis aufrufen | ~~Mittlere Priorität: super/base-Aufruf ergänzen für korrekte Vererbungskette.~~ ✅ Erledigt: `CBaseToggle::KeyValue(pkvd)` wird jetzt als Fallback aufgerufen. |
| [141](dlls/blowercannon.cpp#L141) | `TODO` | TODO: can crash if target has been removed | TODO: Kann abstürzen wenn das Ziel entfernt wurde | ~~Hohe Priorität: Gültigkeit des Ziels vor Verwendung prüfen (EHANDLE oder Null-Check).~~ ✅ Erledigt: Null-Check für `GetNextTarget()` vor Dereferenzierung hinzugefügt. |


#### `dlls/bmodels.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [183](dlls/bmodels.cpp#L183) | `HACK` | HACKHACK - This is to allow for some special effects | HACKHACK: Ermöglicht einige Spezialeffekte | Niedrige Priorität: Spezialeffekt-Code sauber dokumentieren oder in eigene Methode auslagern. |
| [197](dlls/bmodels.cpp#L197) | `HACK` | HACKHACK -- This is ugly, but encode the speed in the rendercolor to avoid adding more data to the network stream | HACKHACK: Hässlich, aber Geschwindigkeit wird in rendercolor kodiert um Netzwerkstream zu sparen | Mittlere Priorität: Eigenes Netzwerkfeld für Geschwindigkeit einführen. |


#### `dlls/buttons.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [1267](dlls/buttons.cpp#L1267) | `BUG` | BUGBUG: This design causes a latentcy.  When the button is retriggered, the first impulse | BUGBUG: Dieses Design verursacht Latenz beim Auslösen des Knopfes | Mittlere Priorität: Button-Trigger-Logik überarbeiten, um Latenz zu eliminieren. |
| [1337](dlls/buttons.cpp#L1337) | `HACK` | HACKHACK -- If we're going slow, we'll get multiple player packets per frame, bump nexthink on each one to avoid stal... | HACKHACK: Bei langsamer Bewegung mehrere Pakete pro Frame, nexthink stoßen | Niedrige Priorität: nexthink-Berechnung robuster gestalten. |


#### `dlls/cbase.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [356](dlls/cbase.cpp#L356) | `HACK` | HACKHACK - reset the save pointers, we're going to restore for real this time | HACKHACK: Save-Pointer zurücksetzen für echte Wiederherstellung | Niedrige Priorität: Save/Restore-Logik überarbeiten für saubereren Datenfluss. |


#### `dlls/cleansuit_scientist.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [338](dlls/cleansuit_scientist.cpp#L338) | `FIXME` | {TASK_SET_ACTIVITY, (float)ACT_CROUCHIDLE}, // FIXME: This looks lame | FIXME: ACT_CROUCHIDLE sieht lahm aus | Niedrige Priorität: Bessere Crouch-Idle-Animation verwenden oder Übergänge verbessern. |


#### `dlls/client.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [436](dlls/client.cpp#L436) | `TODO` | TODO: clamp cvar value so it can't be negative | TODO: Cvar-Wert begrenzen, darf nicht negativ sein | ~~Mittlere Priorität: Cvar-Wert-Clamp einbauen, um ungültige Werte zu verhindern.~~ ✅ Erledigt: `V_max(0.0f, spamdelay.value)` verhindert negative Chat-Timer. |
| [754](dlls/client.cpp#L754) | `TODO` | TODO: in vanilla Op4 this code incorrectly skips the above validation logic if the player is already in a team | TODO: In Vanilla Op4 wird obige Validierungslogik fälschlicherweise übersprungen wenn Spieler bereits in einem Team ist | ~~Hohe Priorität: Validierungslogik immer ausführen, unabhängig vom Team-Status.~~ ✅ Erledigt: Code war bereits korrekt; stale TODO-Kommentar entfernt. |
| [1513](dlls/client.cpp#L1513) | `HACK` | HACK:  Somewhat... | HACK: Etwas... [unvollständiger Kommentar] | Niedrige Priorität: Hack-Kommentar vervollständigen oder Lösung sauber implementieren. |


#### `dlls/combat.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [41](dlls/combat.cpp#L41) | `HACK` | HACKHACK -- The gib velocity equations don't work | HACKHACK: Die Gib-Geschwindigkeitsgleichungen funktionieren nicht | Mittlere Priorität: Physik-Gleichungen für Gibs überarbeiten. |
| [963](dlls/combat.cpp#L963) | `HACK` | HACKHACK Don't kill monsters in a script.  Let them break their scripts first | HACKHACK: Monster in Skripten nicht töten – Skript zuerst abbrechen lassen | Mittlere Priorität: Saubere Skript-Abbruch-Logik vor dem Töten implementieren. |


#### `dlls/controller.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [183](dlls/controller.cpp#L183) | `HACK` | HACK HACK -- until we fix this. | HACK HACK: Bis wir das reparieren | Niedrige Priorität: Workaround durch saubere Implementierung ersetzen. |


#### `dlls/ctf/CHUDIconTrigger.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [153](dlls/ctf/CHUDIconTrigger.cpp#L153) | `TODO` | TODO: this will break when an index is larger than 31 or a negative value | TODO: Bricht wenn ein Index größer als 31 oder negativ ist | ~~Hohe Priorität: Bitfeld auf 32-Bit begrenzen oder dynamische Datenstruktur verwenden.~~ ✅ Erledigt: `m_nCustomIndex` wird jetzt auf den Bereich [0, 31] geprüft bevor der Bit-Shift ausgeführt wird. |


#### `dlls/ctf/CItemAcceleratorCTF.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [39](dlls/ctf/CItemAcceleratorCTF.cpp#L39) | `TODO` | TODO: is this actually used? | TODO: Wird das überhaupt verwendet? | Niedrige Priorität: Toten Code identifizieren und ggf. entfernen. |


#### `dlls/ctf/CItemBackpackCTF.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [84](dlls/ctf/CItemBackpackCTF.cpp#L84) | `TODO` | TODO: precache calls should be in Precache | TODO: Precache-Aufrufe sollten in Precache() stehen | ~~Mittlere Priorität: Precache-Aufrufe in die richtige Methode verschieben.~~ ✅ Erledigt: Precache-Aufrufe in `Precache()` verschoben mit `CItemCTF::Precache()`-Kettenaufruf. |
| [93](dlls/ctf/CItemBackpackCTF.cpp#L93) | `TODO` | TODO: shouldn't this be using pev->model? | TODO: Sollte pev->model verwenden? | ~~Mittlere Priorität: Modellreferenz einheitlich über pev->model setzen.~~ ✅ Erledigt: `SetModel` nutzt jetzt `pev->model` mit Fallback auf Default-Modell. |


#### `dlls/ctf/CItemCTF.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [37](dlls/ctf/CItemCTF.cpp#L37) | `TODO` | TODO: should invoke base class KeyValue here | TODO: Basisklassen-KeyValue hier aufrufen | ~~Mittlere Priorität: Basis-KeyValue-Aufruf ergänzen für vollständige Schlüsselwert-Verarbeitung.~~ ✅ Erledigt: `CBaseEntity::KeyValue(pkvd)` wird jetzt als Fallback aufgerufen. |
| [90](dlls/ctf/CItemCTF.cpp#L90) | `TODO` | TODO: already done above | TODO: Wurde bereits oben erledigt | Niedrige Priorität: Duplizierten Code entfernen. |
| [240](dlls/ctf/CItemCTF.cpp#L240) | `TODO` | TODO: really shouldn't be using the index here tbh | TODO: Sollte Index hier wirklich nicht verwenden | Mittlere Priorität: Index-basierten Zugriff durch sicheren Bezeichner ersetzen. |


#### `dlls/ctf/CItemLongJumpCTF.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [78](dlls/ctf/CItemLongJumpCTF.cpp#L78) | `TODO` | TODO: precache calls should be in Precache | TODO: Precache-Aufrufe sollten in Precache() stehen | ~~Mittlere Priorität: Precache-Aufrufe in die richtige Methode verschieben.~~ ✅ Erledigt: Precache-Aufrufe in `Precache()` verschoben mit `CItemCTF::Precache()`-Kettenaufruf. |
| [87](dlls/ctf/CItemLongJumpCTF.cpp#L87) | `TODO` | TODO: shouldn't this be using pev->model? | TODO: Sollte pev->model verwenden? | ~~Mittlere Priorität: Modellreferenz einheitlich über pev->model setzen.~~ ✅ Erledigt: `SetModel` nutzt jetzt `pev->model` mit Fallback auf Default-Modell. |


#### `dlls/ctf/CItemPortableHEVCTF.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [81](dlls/ctf/CItemPortableHEVCTF.cpp#L81) | `TODO` | TODO: shouldn't this be using pev->model? | TODO: Sollte pev->model verwenden? | ~~Mittlere Priorität: Modellreferenz einheitlich über pev->model setzen.~~ ✅ Erledigt: `SetModel` nutzt jetzt `pev->model` mit Fallback auf Default-Modell. |


#### `dlls/ctf/CItemRegenerationCTF.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [80](dlls/ctf/CItemRegenerationCTF.cpp#L80) | `TODO` | TODO: precache calls should be in Precache | TODO: Precache-Aufrufe sollten in Precache() stehen | ~~Mittlere Priorität: Precache-Aufrufe in die richtige Methode verschieben.~~ ✅ Erledigt: Precache-Aufrufe in `Precache()` verschoben mit `CItemCTF::Precache()`-Kettenaufruf. |
| [89](dlls/ctf/CItemRegenerationCTF.cpp#L89) | `TODO` | TODO: shouldn't this be using pev->model? | TODO: Sollte pev->model verwenden? | ~~Mittlere Priorität: Modellreferenz einheitlich über pev->model setzen.~~ ✅ Erledigt: `SetModel` nutzt jetzt `pev->model` mit Fallback auf Default-Modell. |


#### `dlls/ctf/ctfplay_gamerules.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [70](dlls/ctf/ctfplay_gamerules.cpp#L70) | `TODO` | TODO: rework so it's 2 separate lists | TODO: Als 2 separate Listen umstrukturieren | Mittlere Priorität: Datenstruktur aufteilen für bessere Übersichtlichkeit. |
| [129](dlls/ctf/ctfplay_gamerules.cpp#L129) | `TODO` | TODO: doesn't really make sense, if team 0 is losing the score difference is 0 | TODO: Macht wenig Sinn – wenn Team 0 verliert ist der Punktunterschied 0 | ~~Mittlere Priorität: Verlier-Logik überarbeiten für korrekte Auswertung.~~ ✅ Erledigt: `GetLosingTeam()` erkennt jetzt korrekt sowohl BlackMesa als auch OpposingForce als verlierende Teams. |
| [438](dlls/ctf/ctfplay_gamerules.cpp#L438) | `TODO` | TODO: this can probably be optimized by finding the last item that the player is carrying and only sending that | TODO: Kann optimiert werden – nur letztes getragenes Item des Spielers senden | Niedrige Priorität: Netzwerkeffizienz durch inkrementelle Item-Updates verbessern. |
| [505](dlls/ctf/ctfplay_gamerules.cpp#L505) | `TODO` | TODO: checks against an index that may not have been sent | TODO: Prüft Index der möglicherweise nicht gesendet wurde | Hohe Priorität: Sicherheitscheck für ungesendete Indizes einbauen. |
| [702](dlls/ctf/ctfplay_gamerules.cpp#L702) | `TODO` | TODO: doesn't seem to be used | TODO: Scheint nicht verwendet zu werden | Niedrige Priorität: Toten Code entfernen. |
| [1770](dlls/ctf/ctfplay_gamerules.cpp#L1770) | `TODO` | TODO: player count is always 0 | TODO: Spieleranzahl ist immer 0 | ~~Hohe Priorität: Spieleranzahl korrekt initialisieren/aktualisieren.~~ ✅ Erledigt: Spieler werden jetzt per Team gezählt und korrekt im End-Game-Log ausgegeben. |


#### `dlls/effects.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [107](dlls/effects.cpp#L107) | `HACK` | HACKHACK!!! - Speed in rendercolor | HACKHACK!!! – Geschwindigkeit in rendercolor codiert | Mittlere Priorität: Eigenes Datenfeld für Geschwindigkeit verwenden. |


#### `dlls/explode.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [260](dlls/explode.cpp#L260) | `HACK` | HACKHACK -- create one of these and fake a keyvalue to get the right explosion setup | HACKHACK: Fake-Keyvalue für korrekte Explosions-Einrichtung | Mittlere Priorität: Saubere Explosions-Initialisierung über Parameter implementieren. |


#### `dlls/func_break.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [181](dlls/func_break.cpp#L181) | `HACK` | HACK:  matGlass can receive decals, we need the client to know about this | HACK: matGlass kann Decals empfangen, Client muss es wissen | Niedrige Priorität: Material-Flag sauber über Netzwerknachricht kommunizieren. |
| [774](dlls/func_break.cpp#L774) | `BUG` | BUGBUG -- can only find 256 entities on a breakable -- should be enough | BUGBUG: Kann nur 256 Entitäten an einem Breakable finden | Mittlere Priorität: Limit erhöhen oder dynamische Liste verwenden. |


#### `dlls/func_tank.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [491](dlls/func_tank.cpp#L491) | `TODO` | TODO: bring back the controllers current weapon | TODO: Aktuelle Waffe des Controllers wiederherstellen | Mittlere Priorität: Waffenzustand beim Verlassen des Tanks wiederherstellen. |
| [525](dlls/func_tank.cpp#L525) | `HACK` | HACKHACK -- make some noise (that the AI can hear) | HACKHACK: Lärm erzeugen, den KI hören kann | Niedrige Priorität: Geräuschsystem sauber über Sound-Events triggern. |


#### `dlls/func_tank_of.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [143](dlls/func_tank_of.cpp#L143) | `TODO` | TODO: could be exploited to make a tank change targets | TODO: Könnte ausgenutzt werden, um Tank-Ziel zu wechseln | Hohe Priorität: Exploit-Möglichkeit prüfen und durch Zugriffskontrolle absichern. |
| [400](dlls/func_tank_of.cpp#L400) | `TODO` | TODO: bring back the controllers current weapon | TODO: Aktuelle Waffe des Controllers wiederherstellen | Mittlere Priorität: Waffenzustand beim Verlassen des Tanks wiederherstellen. |
| [435](dlls/func_tank_of.cpp#L435) | `HACK` | HACKHACK -- make some noise (that the AI can hear) | HACKHACK: Lärm erzeugen, den KI hören kann | Niedrige Priorität: Geräuschsystem sauber über Sound-Events triggern. |


#### `dlls/gargantua.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [981](dlls/gargantua.cpp#L981) | `HACK` | HACKHACK!!! | HACKHACK!!! | Niedrige Priorität: Hack dokumentieren oder sauber implementieren. |
| [1065](dlls/gargantua.cpp#L1065) | `HACK` | HACKHACK - turn off the flames if they are on and garg goes scripted / dead | HACKHACK: Flammen ausschalten wenn Gargantua skriptet/stirbt | Mittlere Priorität: Flammen-Status mit Entitäts-Lebenszyklus koppeln. |
| [1342](dlls/gargantua.cpp#L1342) | `HACK` | HACKHACK Cut and pasted from explode.cpp | HACKHACK: Aus explode.cpp kopiert | Mittlere Priorität: Gemeinsamen Code in Hilfsfunktion auslagern. |


#### `dlls/geneworm.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [166](dlls/geneworm.cpp#L166) | `TODO` | TODO: the original code looks like it may be ignoring the modulo, verify this | TODO: Original-Code ignoriert möglicherweise den Modulo, prüfen | Mittlere Priorität: Modulo-Verhalten prüfen und angleichen. |
| [1137](dlls/geneworm.cpp#L1137) | `TODO` | TODO: this really shouldn't be hardcoded | TODO: Sollte nicht hart codiert sein | ~~Mittlere Priorität: Hardcoded-Wert als Konstante oder Konfigurationsparameter definieren.~~ ✅ Erledigt: `"GeneWormTeleport"` als `GENEWORM_TELEPORT_TARGET`-Konstante extrahiert; `ALERT` → `LOG_DEBUG`. |
| [1202](dlls/geneworm.cpp#L1202) | `TODO` | TODO: maybe determine direction of velocity to apply? | TODO: Vielleicht Richtung der anzuwendenden Geschwindigkeit ermitteln? | Niedrige Priorität: Physikalisch korrekte Geschwindigkeitsrichtung berechnen. |
| [1336](dlls/geneworm.cpp#L1336) | `TODO` | TODO: never used? | TODO: Niemals verwendet? | Niedrige Priorität: Toten Code prüfen und ggf. entfernen. |


#### `dlls/gonome.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [88](dlls/gonome.cpp#L88) | `TODO` | TODO: probably shouldn't be assinging to x every time | TODO: Sollte nicht jedes Mal x zuweisen | Niedrige Priorität: Mehrfache x-Zuweisung zu einer einzigen zusammenfassen. |
| [248](dlls/gonome.cpp#L248) | `TODO` | TODO: needs to be EHANDLE, save/restored or a save during a windup will cause problems | TODO: Muss EHANDLE sein und gespeichert/wiederhergestellt werden | ~~Hohe Priorität: EHANDLE verwenden um Absturz bei Save/Load-Zyklen zu verhindern.~~ ✅ Erledigt: `m_pGonomeGuts` in `EHANDLE m_hGonomeGuts` umgewandelt und in `TYPEDESCRIPTION` für Save/Restore eingetragen. |
| [376](dlls/gonome.cpp#L376) | `HACK` | HACK HACK -- until we fix this. | HACK HACK: Bis wir das reparieren | Niedrige Priorität: Workaround durch saubere Implementierung ersetzen. |


#### `dlls/h_battery.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [143](dlls/h_battery.cpp#L143) | `TODO` | TODO: useless, it's accessed earlier on. | TODO: Nutzlos, wurde früher bereits zugegriffen | Niedrige Priorität: Überflüssige Prüfung entfernen. |
| [151](dlls/h_battery.cpp#L151) | `TODO` | TODO: put this check at the top. | TODO: Diese Prüfung nach oben verschieben | Niedrige Priorität: Guard-Clause an den Anfang der Funktion verschieben. |


#### `dlls/handgrenade.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [71](dlls/handgrenade.cpp#L71) | `TODO` | TODO: not sure how useful this is given that the player has to have this weapon for this method to be called | TODO: Nicht sicher wie nützlich das ist, da der Spieler die Waffe haben muss | Niedrige Priorität: Usefulness der Prüfung analysieren. |


#### `dlls/headcrab.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [417](dlls/headcrab.cpp#L417) | `BUG` | BUGBUG: Why is this code here?  There is no ACT_RANGE_ATTACK2 animation.  I've disabled it for now. | BUGBUG: Warum ist dieser Code hier? Es gibt keine ACT_RANGE_ATTACK2-Animation | ~~Mittlere Priorität: Toten Animationscode entfernen oder passende Animation ergänzen.~~ ✅ Erledigt: Toter `#if 0`-Block für `ACT_RANGE_ATTACK2` entfernt; Funktion gibt direkt `false` zurück. |


#### `dlls/healthkit.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [81](dlls/healthkit.cpp#L81) | `TODO` | TODO: incorrect check here, but won't respawn due to respawn delay being -1 in singleplayer | TODO: Falsche Prüfung, aber kein Respawn wegen Respawn-Delay -1 in Singleplayer | Mittlere Priorität: Prüfung korrigieren für konsistentes Verhalten in allen Spielmodi. |


#### `dlls/hgrunt_ally.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [573](dlls/hgrunt_ally.cpp#L573) | `TODO` | TODO: kinda odd that this doesn't use GetGunPosition like the original | TODO: Eigenartig, dass GetGunPosition nicht wie im Original verwendet wird | Niedrige Priorität: GetGunPosition einheitlich wie im Original nutzen. |
| [733](dlls/hgrunt_ally.cpp#L733) | `TODO` | TODO: disabled for ally | TODO: Für Ally deaktiviert | Niedrige Priorität: Funktion für Ally-Variante prüfen und ggf. aktivieren. |
| [1252](dlls/hgrunt_ally.cpp#L1252) | `TODO` | TODO: probably also needs this for head HGruntAllyHead::BeretBlack | TODO: Wahrscheinlich auch für HGruntAllyHead::BeretBlack nötig | Niedrige Priorität: Konsistente Kopf-Varianten-Unterstützung sicherstellen. |


#### `dlls/hgrunt_medic.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [405](dlls/hgrunt_medic.cpp#L405) | `TODO` | TODO: probably the wrong logic, but it was in the original | TODO: Wahrscheinlich falsche Logik, war aber im Original | ~~Mittlere Priorität: Logik überprüfen und korrigieren.~~ ✅ Erledigt: Nicht initialisierte Variable `pGun` mit `nullptr` initialisiert, um UB zu vermeiden wenn keines der Waffen-Flags gesetzt ist. |
| [586](dlls/hgrunt_medic.cpp#L586) | `TODO` | TODO: kinda odd that this doesn't use GetGunPosition like the original | TODO: Eigenartig, dass GetGunPosition nicht wie im Original verwendet wird | Niedrige Priorität: GetGunPosition einheitlich nutzen. |
| [746](dlls/hgrunt_medic.cpp#L746) | `TODO` | TODO: disabled for ally | TODO: Für Ally deaktiviert | Niedrige Priorität: Funktion für Ally-Variante prüfen. |
| [2937](dlls/hgrunt_medic.cpp#L2937) | `TODO` | TODO: missing from medic? | TODO: Fehlt beim Medic? | ~~Mittlere Priorität: Fehlende Funktionalität für Medic ergänzen.~~ ✅ Erledigt: Provokations-Check (`bits_MEMORY_PROVOKED`, `StopFollowing`) in `Killed()` aktiviert, analog zu anderen Talk-Monstern. |
| [2961](dlls/hgrunt_medic.cpp#L2961) | `TODO` | TODO: not suited for multiplayer | TODO: Nicht für Multiplayer geeignet | Mittlere Priorität: Multiplayer-Kompatibilität implementieren. |
| [3039](dlls/hgrunt_medic.cpp#L3039) | `TODO` | TODO: could just change the type of pTarget since this is the only type passed in | TODO: Könnte einfach den Typ von pTarget ändern, da das der einzige übergebene Typ ist | Niedrige Priorität: Typumwandlung vereinfachen. |


#### `dlls/hgrunt_torch.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [551](dlls/hgrunt_torch.cpp#L551) | `TODO` | TODO: kinda odd that this doesn't use GetGunPosition like the original | TODO: Eigenartig, dass GetGunPosition nicht wie im Original verwendet wird | Niedrige Priorität: GetGunPosition einheitlich nutzen. |
| [718](dlls/hgrunt_torch.cpp#L718) | `TODO` | TODO: disabled for ally | TODO: Für Ally deaktiviert | Niedrige Priorität: Funktion für Ally-Variante prüfen. |
| [2736](dlls/hgrunt_torch.cpp#L2736) | `BUG` | TODO: looks like a bug to me, shouldn't be bitwise inverting | TODO: Sieht wie ein Bug aus, sollte nicht bitweise invertieren | ~~Hohe Priorität: Bitweise-Invertierung prüfen und ggf. durch logische Negation ersetzen.~~ ✅ Erledigt: `|= ~SF_BEAM_SPARKEND` (bitweise NOT, setzte alle anderen Flags) durch `|= SF_BEAM_SPARKEND` ersetzt. |


#### `dlls/male_assassin.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [829](dlls/male_assassin.cpp#L829) | `TODO` | TODO: why is this 556? is 762 too damaging? | TODO: Warum 556? Ist 762 zu schadenstark? | Niedrige Priorität: Schadensbalancierung dokumentieren oder überarbeiten. |
| [2022](dlls/male_assassin.cpp#L2022) | `TODO` | TODO: probably don't want this here, but it's still present in op4 | TODO: Wahrscheinlich nicht hier gewollt, aber noch in op4 vorhanden | Niedrige Priorität: Unerwünschten Code prüfen und ggf. entfernen. |


#### `dlls/monsters.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [811](dlls/monsters.cpp#L811) | `TODO` | TODO: verify this this needs to be a comparison and not a bit check | TODO: Prüfen ob das ein Vergleich oder ein Bit-Check sein muss | ~~Mittlere Priorität: Bitoperation vs. Vergleich analysieren und korrigieren.~~ ✅ Erledigt: Gleichheitsvergleich `routeType == bits_MF_TO_PATHCORNER` durch Bit-Check `(routeType & bits_MF_TO_PATHCORNER) != 0` ersetzt. |
| [826](dlls/monsters.cpp#L826) | `BUG` | BUGBUG: this doesn't work 100% yet | BUGBUG: Funktioniert noch nicht 100% | Hohe Priorität: Monster-Logik-Fehler identifizieren und vollständig beheben. |


#### `dlls/monsters.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [114](dlls/monsters.h#L114) | `HACK` | #define bits_MEMORY_KILLED (1 << 7)		   // HACKHACK -- remember that I've already called my Killed() | HACKHACK: Merken dass Killed() bereits aufgerufen wurde | Niedrige Priorität: Saubere State-Machine für Killed-Status implementieren. |


#### `dlls/multiplay_gamerules.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [358](dlls/multiplay_gamerules.cpp#L358) | `TODO` | TODO: really shouldn't be modifying this directly | TODO: Sollte das nicht direkt modifizieren | Mittlere Priorität: Accessor-Methode verwenden statt direkten Member-Zugriff. |
| [426](dlls/multiplay_gamerules.cpp#L426) | `FIXME` | FIXME:  Probably don't need to cast this just to read m_iDeaths | FIXME: Cast nur zum Lesen von m_iDeaths wahrscheinlich unnötig | Niedrige Priorität: Unnötigen Cast entfernen. |
| [974](dlls/multiplay_gamerules.cpp#L974) | `TODO` | TODO: make this go direct to console | TODO: Direkt an Konsole ausgeben | Niedrige Priorität: Ausgabe direkt an Konsole statt über Zwischenpuffer. |


#### `dlls/nuclearbomb.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [217](dlls/nuclearbomb.cpp#L217) | `TODO` | TODO: set the classname members for both entities | TODO: Classname-Member für beide Entitäten setzen | ~~Mittlere Priorität: Klassennamen für korrekte Entitätsidentifikation setzen.~~ ✅ Erledigt: `pev->classname` für `COFNuclearBombTimer` und `COFNuclearBombButton` wird jetzt bei `GetClassPtr` gesetzt; `ALERT` → `LOG_ERROR`. |


#### `dlls/op4mortar.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [188](dlls/op4mortar.cpp#L188) | `TODO` | TODO: never used? | TODO: Niemals verwendet? | Niedrige Priorität: Toten Code prüfen und ggf. entfernen. |


#### `dlls/otis.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [639](dlls/otis.cpp#L639) | `TODO` | TODO: Otis doesn't have a helmet, probably don't want his dome being bulletproof | TODO: Otis hat keinen Helm, sein Kopf sollte nicht kugelsicher sein | ~~Mittlere Priorität: Kopfschutz-Eigenschaft für Otis entfernen.~~ ✅ Erledigt: Kugelsicherer Kopf-Case (`case 10`) für Otis vollständig entfernt. |


#### `dlls/pathcorner.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [358](dlls/pathcorner.cpp#L358) | `HACK` | if (dist == originalDist) // HACK -- up against a dead end | HACK: Gegen eine Sackgasse gelaufen | Mittlere Priorität: Sackgassen-Erkennung und -Behandlung sauber implementieren. |


#### `dlls/penguin_grenade.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [127](dlls/penguin_grenade.cpp#L127) | `TODO` | TODO: set to null earlier on, so this can never be valid | TODO: Früher auf null gesetzt, kann daher nie gültig sein | ~~Hohe Priorität: Null-Pointer-Dereference vermeiden.~~ ✅ Erledigt: Null-Check für `owner` vor `IsPlayer()`-Aufruf hinzugefügt. |
| [253](dlls/penguin_grenade.cpp#L253) | `TODO` | TODO: shouldn't use index | TODO: Sollte keinen Index verwenden | Mittlere Priorität: Index-basierten Zugriff durch sicheren Bezeichner ersetzen. |


#### `dlls/pitdrone.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [153](dlls/pitdrone.cpp#L153) | `TODO` | TODO: maybe stick it on any entity that reports FL_WORLDBRUSH too? | TODO: Auch auf Entitäten mit FL_WORLDBRUSH anwenden? | Niedrige Priorität: Flag-Behandlung für Weltbürsten-Entitäten prüfen. |
| [734](dlls/pitdrone.cpp#L734) | `TODO` | TODO: constrain value to valid range | TODO: Wert auf gültigen Bereich begrenzen | Mittlere Priorität: Wertebereich-Validierung einbauen. |


#### `dlls/pitworm_up.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [669](dlls/pitworm_up.cpp#L669) | `TODO` | TODO: maybe determine direction of velocity to apply? | TODO: Vielleicht Richtung der Geschwindigkeit ermitteln? | Niedrige Priorität: Physikalisch korrekte Geschwindigkeitsrichtung berechnen. |
| [899](dlls/pitworm_up.cpp#L899) | `TODO` | TODO: missing an ApplyMultiDamage call here | TODO: Fehlender ApplyMultiDamage-Aufruf | ~~Hohe Priorität: Schaden korrekt anwenden, fehlenden Aufruf ergänzen.~~ ✅ Erledigt: `TakeDamage`-Doppelaufruf durch korrektes `ClearMultiDamage` → `TraceAttack` → `ApplyMultiDamage`-Muster ersetzt. |
| [982](dlls/pitworm_up.cpp#L982) | `TODO` | TODO: never used? | TODO: Niemals verwendet? | Niedrige Priorität: Toten Code prüfen und ggf. entfernen. |


#### `dlls/plats.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [1766](dlls/plats.cpp#L1766) | `HACK` | HACKHACK -- This is bugly, but the train can actually stop moving at a different node depending on it's speed | HACKHACK: Zug kann je nach Geschwindigkeit an verschiedenen Knoten stoppen | Mittlere Priorität: Halte-Logik des Zuges präzisieren. |


#### `dlls/player.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [406](dlls/player.cpp#L406) | `TODO` | TODO: this is a pretty bad way to handle damage increase | TODO: Ziemlich schlechte Art, Schadenserhöhung zu handhaben | Mittlere Priorität: Schadensmodifier-System überarbeiten. |
| [1525](dlls/player.cpp#L1525) | `TODO` | TODO: Send HUD Update | TODO: HUD-Aktualisierung senden | Mittlere Priorität: HUD-Update-Nachricht implementieren. |
| [1642](dlls/player.cpp#L1642) | `BUG` | else if ((m_afButtonReleased & IN_USE) != 0 && (pObject->ObjectCaps() & FCAP_ONOFF_USE) != 0) // BUGBUG This is an "o... | BUGBUG: 'once' USE-Objekt kann erneut ausgelöst werden | Hohe Priorität: Use-Objekt-Logik korrigieren. |
| [2131](dlls/player.cpp#L2131) | `HACK` | HACKHACK - Just look for the func_tracktrain classname | HACKHACK: Sucht nach func_tracktrain Classname | Niedrige Priorität: Sauberere Methode zur Classname-Identifikation verwenden. |
| [2818](dlls/player.cpp#L2818) | `BUG` | BUG - this happens all the time in water, especially when | BUG: Passiert ständig im Wasser | Mittlere Priorität: Wasser-Physik-Bug identifizieren und beheben. |
| [2819](dlls/player.cpp#L2819) | `BUG` | BUG - water has current force | BUG: Wasser hat Strömungskraft | Mittlere Priorität: Strömungseinfluss korrekt berechnen. |
| [3381](dlls/player.cpp#L3381) | `HACK` | HACK:	This variable is saved/restored in CBaseMonster as a time variable, but we're using it | HACK: Variable als Zeit-Variable gespeichert/wiederhergestellt, wird aber anders verwendet | Mittlere Priorität: Separate Variable für diesen Zweck einführen. |
| [3927](dlls/player.cpp#L3927) | `TODO` | TODO: not given | TODO: Nicht angegeben | Niedrige Priorität: Fehlenden Kommentar und Implementierung ergänzen. |
| [4076](dlls/player.cpp#L4076) | `FIXME` | FIXME: remove anyway for deathmatch testing | FIXME: Für Deathmatch-Tests entfernen | Niedrige Priorität: Debug-Code nach Abschluss der Tests entfernen. |
| [4130](dlls/player.cpp#L4130) | `FIXME` | FIXME: remove anyway for deathmatch testing | FIXME: Für Deathmatch-Tests entfernen | Niedrige Priorität: Debug-Code nach Abschluss der Tests entfernen. |
| [4442](dlls/player.cpp#L4442) | `HACK` | HACKHACK -- send the message to display the game title | HACKHACK: Nachricht zum Anzeigen des Spieltitels senden | Niedrige Priorität: Spieltitel-Anzeige über sauberes Event-System implementieren. |
| [4443](dlls/player.cpp#L4443) | `TODO` | TODO: will not work properly in multiplayer | TODO: Funktioniert im Multiplayer nicht korrekt | Mittlere Priorität: Multiplayer-Kompatibilität für Spieltitel-Anzeige implementieren. |


#### `dlls/rope/CRope.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [632](dlls/rope/CRope.cpp#L632) | `TODO` | TODO move to common header - Solokiller | TODO: In gemeinsamen Header verschieben – Solokiller | Niedrige Priorität: Konstante in gemeinsamen Header auslagern. |
| [819](dlls/rope/CRope.cpp#L819) | `TODO` | TODO: maybe only set/unset the nodraw flag | TODO: Vielleicht nur nodraw-Flag setzen/entfernen | Niedrige Priorität: Effizientere Sichtbarkeitssteuerung implementieren. |


#### `dlls/rope/CRopeSample.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [38](dlls/rope/CRopeSample.cpp#L38) | `TODO` | TODO: needed? | TODO: Benötigt? | Niedrige Priorität: Prüfen ob dieser Code noch benötigt wird. |


#### `dlls/satchel.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [86](dlls/satchel.cpp#L86) | `HACK` | HACKHACK - On ground isn't always set, so look for ground underneath | HACKHACK: On-Ground nicht immer gesetzt, daher Boden suchen | Mittlere Priorität: Zuverlässige Bodenerkennung implementieren. |


#### `dlls/schedule.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [83](dlls/schedule.cpp#L83) | `TODO` | TODO: not the correct way to check for missing classname, is like this in vanilla Op4 | TODO: Falsche Methode zur Prüfung auf fehlenden Classname | ~~Mittlere Priorität: Korrekte Classname-Prüfung implementieren.~~ ✅ Erledigt: Raw-Pointer-Truthiness durch `FStringNull(pev->classname)` ersetzt; `ALERT` → `LOG_DEBUG`. |


#### `dlls/scientist.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [354](dlls/scientist.cpp#L354) | `FIXME` | {TASK_SET_ACTIVITY, (float)ACT_CROUCHIDLE}, // FIXME: This looks lame | FIXME: ACT_CROUCHIDLE sieht lahm aus | Niedrige Priorität: Bessere Crouch-Idle-Animation verwenden. |


#### `dlls/scripted.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [667](dlls/scripted.cpp#L667) | `BUG` | BUGBUG -- This doesn't call Killed() | BUGBUG: Ruft Killed() nicht auf | Hohe Priorität: Killed()-Aufruf für korrekte Aufräum-Logik ergänzen. |
| [871](dlls/scripted.cpp#L871) | `BUG` | SetUse(NULL);	// BUGBUG -- This doesn't call Killed() | BUGBUG: SetUse(NULL) ruft Killed() nicht auf | Hohe Priorität: Killed()-Aufruf sicherstellen. |


#### `dlls/shockroach.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [443](dlls/shockroach.cpp#L443) | `BUG` | BUGBUG: Why is this code here?  There is no ACT_RANGE_ATTACK2 animation.  I've disabled it for now. | BUGBUG: Warum ist dieser Code hier? Es gibt keine ACT_RANGE_ATTACK2-Animation | ~~Mittlere Priorität: Toten Animationscode entfernen oder passende Animation ergänzen.~~ ✅ Erledigt: Toter `#if 0`-Block für `ACT_RANGE_ATTACK2` entfernt; Funktion gibt direkt `false` zurück. |


#### `dlls/shocktrooper.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [345](dlls/shocktrooper.cpp#L345) | `TODO` | TODO: change body group | TODO: Bodygroup wechseln | Mittlere Priorität: Bodygroup-Wechsel für Schocktruppen-Varianten implementieren. |
| [1138](dlls/shocktrooper.cpp#L1138) | `TODO` | TODO: the directory names should be lowercase | TODO: Verzeichnisnamen sollten kleingeschrieben sein | Niedrige Priorität: Verzeichnisnamen-Konvention einheitlich in Kleinbuchstaben umstellen. |


#### `dlls/sound.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [156](dlls/sound.cpp#L156) | `HACK` | HACKHACK - This is not really in the spirit of the save/restore design, but save this | HACKHACK: Nicht wirklich im Sinne des Save/Restore-Designs | Mittlere Priorität: Save/Restore-Logik überarbeiten. |
| [625](dlls/sound.cpp#L625) | `HACK` | HACKHACK - this makes the code in Precache() work properly after a save/restore | HACKHACK: Damit Precache() nach Save/Restore korrekt funktioniert | Mittlere Priorität: Saubere Precache-Wiederherstellung nach Save/Restore implementieren. |


#### `dlls/squeakgrenade.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [501](dlls/squeakgrenade.cpp#L501) | `HACK` | HACK HACK:  Ugly hacks to handle change in origin based on new physics code for players | HACK HACK: Hässliche Hacks für Ursprungsänderung durch neue Spieler-Physik | Mittlere Priorität: Ursprungsberechnung an neue Physik anpassen. |


#### `dlls/teamplay_gamerules.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [29](dlls/teamplay_gamerules.cpp#L29) | `TODO` | TODO: these should be members of CHalfLifeTeamplay | TODO: Sollten Member von CHalfLifeTeamplay sein | Mittlere Priorität: Globale Variablen in Klassenmember umwandeln. |


#### `dlls/tentacle.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [75](dlls/tentacle.cpp#L75) | `TODO` | TODO: should override base, but has different signature | TODO: Sollte Basis überschreiben, hat aber andere Signatur | Mittlere Priorität: Methoden-Signatur anpassen oder Adapter implementieren. |


#### `dlls/triggers.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [1024](dlls/triggers.cpp#L1024) | `HACK` | HACKHACK -- In multiplayer, players touch this based on packet receipt. | HACKHACK: Im Multiplayer berühren Spieler dies basierend auf Paketempfang | Mittlere Priorität: Deterministische Touch-Auslösung implementieren. |
| [1042](dlls/triggers.cpp#L1042) | `BUG` | BUGBUG - There can be only 32 players! | BUGBUG: Es kann nur 32 Spieler geben! | Mittlere Priorität: Spieleranzahl-Limit dokumentieren oder erhöhen. |
| [1060](dlls/triggers.cpp#L1060) | `BUG` | BUGBUG - There can be only 32 players! | BUGBUG: Es kann nur 32 Spieler geben! | Mittlere Priorität: Spieleranzahl-Limit dokumentieren oder erhöhen. |
| [2592](dlls/triggers.cpp#L2592) | `TODO` | TODO: not made for multiplayer | TODO: Nicht für Multiplayer gemacht | Mittlere Priorität: Multiplayer-Unterstützung implementieren. |
| [2623](dlls/triggers.cpp#L2623) | `TODO` | TODO: this needs to be removed in order to function | TODO: Muss entfernt werden um zu funktionieren | ~~Hohe Priorität: Blockierenden Code entfernen.~~ ✅ Erledigt: `pev->solid = SOLID_NOT` aus `CTriggerKillNoGib::Spawn()` entfernt, Touch-Callback funktioniert jetzt. |
| [2888](dlls/triggers.cpp#L2888) | `TODO` | TODO: constrain team_no input to valid values | TODO: team_no-Eingabe auf gültige Werte begrenzen | ~~Mittlere Priorität: Eingabe-Validierung für Team-Nummer einbauen.~~ ✅ Erledigt: Explizite Bereichsprüfung `[1, MaxTeams]` vor Array-Zugriff auf `teamscores`. |


#### `dlls/tripmine.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [97](dlls/tripmine.cpp#L97) | `TODO` | TODO: define constant | TODO: Konstante definieren | Niedrige Priorität: Magic-Number durch benannte Konstante ersetzen. |
| [261](dlls/tripmine.cpp#L261) | `HACK` | HACKHACK Set simple box using this really nice global! | HACKHACK: Einfache Box mit diesem globalen Wert setzen | Niedrige Priorität: Globale Variable durch Parameter oder Methode ersetzen. |
| [371](dlls/tripmine.cpp#L371) | `HACK` | HACK: force the body to the first person view by default so it doesn't show up as a huge tripmine for a second. | HACK: Körper in Ego-Ansicht erzwingen damit er nicht kurz als riesige Tretmine erscheint | Niedrige Priorität: Initialisierung des Körpers sauber implementieren. |


#### `dlls/turret.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [22](dlls/turret.cpp#L22) | `TODO` | TODO: | TODO: [Kein Kommentar] | Niedrige Priorität: TODO-Kommentar mit konkreter Aufgabe ergänzen. |


#### `dlls/util.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [1412](dlls/util.cpp#L1412) | `TODO` | TODO: define constants | TODO: Konstanten definieren | Niedrige Priorität: Magic-Numbers durch benannte Konstanten ersetzen. |
| [2579](dlls/util.cpp#L2579) | `TODO` | TODO: need to refactor save game stuff to make this cleaner and reusable | TODO: Save-Game-Code refaktorieren für sauberere Wiederverwendung | Mittlere Priorität: Save-Game-Hilfsfunktionen auslagern und wiederverwenden. |


#### `dlls/voltigore.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [476](dlls/voltigore.cpp#L476) | `TODO` | TODO: use a filter based on attacker to identify self harm | TODO: Filter basierend auf Angreifer für Selbstschaden verwenden | ~~Mittlere Priorität: Attacker-Filter implementieren für korrekte Selbstschaden-Erkennung.~~ ✅ Erledigt: Selbstschaden-Filter implementiert – Shock-Schaden wird nur ignoriert wenn `pevAttacker == pev` (Selbstschaden). |


#### `dlls/weapons.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [1033](dlls/weapons.cpp#L1033) | `TODO` | TODO: should handle -1 return as well (only return true if ammo was taken) | TODO: Rückgabewert -1 behandeln (nur true wenn Munition genommen wurde) | ~~Mittlere Priorität: Rückgabewert-Logik für Munitionsentnahme korrigieren.~~ ✅ Erledigt: `GiveAmmo() != 0` → `> 0` – Rückgabewert `-1` (Munition voll) wird jetzt korrekt als Fehlschlag behandelt. |


#### `dlls/weapons.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [787](dlls/weapons.h#L787) | `TODO` | int m_fInReload; //TODO: not used, remove | TODO: m_fInReload wird nicht verwendet, entfernen | Niedrige Priorität: Totes Member-Variable entfernen. |


#### `dlls/weapons/CDisplacerBall.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [35](dlls/weapons/CDisplacerBall.cpp#L35) | `TODO` | TODO: can probably be smarter - Solokiller | TODO: Kann wahrscheinlich intelligenter gemacht werden – Solokiller | Niedrige Priorität: Algorithmus überarbeiten. |
| [320](dlls/weapons/CDisplacerBall.cpp#L320) | `TODO` | TODO: no next think? - Solokiller | TODO: Kein nächster Think? – Solokiller | ~~Mittlere Priorität: Think-Aufruf ergänzen für korrekte Zustandsmaschine.~~ ✅ Erledigt: `SetNextThink(0)` hinzugefügt, damit der `SUB_Remove`-Think tatsächlich ausgeführt wird. |


#### `dlls/weapons/CEagle.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [370](dlls/weapons/CEagle.cpp#L370) | `TODO` | TODO: could probably use a better model | TODO: Könnte wahrscheinlich ein besseres Modell verwenden | Niedrige Priorität: Modell für Eagle-Waffe verbessern. |


#### `dlls/weapons/CGrappleTip.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [33](dlls/weapons/CGrappleTip.cpp#L33) | `TODO` | TODO: this should be handled differently. A method that returns an overall size, another whether it's fixed, etc. - S... | TODO: Anders behandeln. Methode die Gesamtgröße zurückgibt, andere ob sie fest ist – Solokiller | Mittlere Priorität: Interface für Größenabfrage implementieren. |
| [165](dlls/weapons/CGrappleTip.cpp#L165) | `TODO` | TODO: should probably clamp at sv_maxvelocity to prevent the tip from going off course. - Solokiller | TODO: Auf sv_maxvelocity begrenzen, damit Tipp nicht vom Kurs abweicht – Solokiller | Mittlere Priorität: Geschwindigkeitsbegrenzung implementieren. |


#### `dlls/weapons/CKnife.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [167](dlls/weapons/CKnife.cpp#L167) | `TODO` | TODO: This code assumes the target is a player and not some NPC. Rework it to support NPC backstabbing. | TODO: Code nimmt an, Ziel ist ein Spieler und kein NPC. Für NPC-Rückstoß überarbeiten | ~~Mittlere Priorität: NPC-Backstab-Unterstützung implementieren.~~ ✅ Erledigt: Backstab nutzt `pev->v_angle` für Spieler und `pev->angles` für NPCs; `IsMultiplayer()`-Beschränkung entfernt. |


#### `dlls/weapons/CPenguin.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [24](dlls/weapons/CPenguin.cpp#L24) | `TODO` | TODO: this isn't in vanilla Op4 so it won't save properly there | TODO: Nicht in Vanilla Op4, wird daher nicht korrekt gespeichert | Mittlere Priorität: Save/Restore-Kompatibilität mit Vanilla Op4 sicherstellen. |


#### `dlls/weapons/CSniperRifle.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [142](dlls/weapons/CSniperRifle.cpp#L142) | `TODO` | TODO: 8192 constant should be defined somewhere - Solokiller | TODO: Konstante 8192 sollte definiert sein – Solokiller | Niedrige Priorität: Magic-Number als Konstante definieren. |
| [165](dlls/weapons/CSniperRifle.cpp#L165) | `TODO` | TODO: this doesn't really make sense | TODO: Macht eigentlich keinen Sinn | Mittlere Priorität: Logik überprüfen und überarbeiten. |


### `engine/`


#### `engine/studio.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [28](engine/studio.h#L28) | `TODO` | #define MAXSTUDIOTRIANGLES 20000 // TODO: tune this | TODO: Diesen Wert abstimmen | Niedrige Priorität: Limit nach Profiling-Ergebnissen anpassen. |
| [29](engine/studio.h#L29) | `TODO` | #define MAXSTUDIOVERTS 2048		 // TODO: tune this | TODO: Diesen Wert abstimmen | Niedrige Priorität: Limit nach Profiling-Ergebnissen anpassen. |


### `external/`


#### `external/SDL2/SDL_config_winrt.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [124](external/SDL2/SDL_config_winrt.h#L124) | `TODO` | #define HAVE__STRLWR 1	// TODO, WinRT: consider using _strlwr_s instead | TODO WinRT: _strlwr_s statt _strlwr in Betracht ziehen | Niedrige Priorität: Sicherere String-Funktion verwenden. |
| [128](external/SDL2/SDL_config_winrt.h#L128) | `TODO` | #define HAVE_ITOA 1   // TODO, WinRT: consider using _itoa_s instead | TODO WinRT: _itoa_s statt itoa in Betracht ziehen | Niedrige Priorität: Sicherere Konvertierungsfunktion verwenden. |
| [129](external/SDL2/SDL_config_winrt.h#L129) | `TODO` | #define HAVE__LTOA 1	// TODO, WinRT: consider using _ltoa_s instead | TODO WinRT: _ltoa_s statt _ltoa in Betracht ziehen | Niedrige Priorität: Sicherere Konvertierungsfunktion verwenden. |
| [130](external/SDL2/SDL_config_winrt.h#L130) | `TODO` | #define HAVE__ULTOA 1	// TODO, WinRT: consider using _ultoa_s instead | TODO WinRT: _ultoa_s statt _ultoa in Betracht ziehen | Niedrige Priorität: Sicherere Konvertierungsfunktion verwenden. |
| [142](external/SDL2/SDL_config_winrt.h#L142) | `TODO` | #define HAVE_SSCANF 1	// TODO, WinRT: consider using sscanf_s instead | TODO WinRT: sscanf_s statt sscanf in Betracht ziehen | Niedrige Priorität: Sicherere Scan-Funktion verwenden. |


### `pm_shared/`


#### `pm_shared/pm_math.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [285](pm_shared/pm_math.cpp#L285) | `FIXME` | length = sqrt(length); // FIXME | FIXME: sqrt(length) verwenden | Niedrige Priorität: Mathematische Funktion überprüfen und korrigieren. |
| [302](pm_shared/pm_math.cpp#L302) | `FIXME` | length = sqrt(length); // FIXME | FIXME: sqrt(length) verwenden | Niedrige Priorität: Mathematische Funktion überprüfen und korrigieren. |


#### `pm_shared/pm_shared.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [301](pm_shared/pm_shared.cpp#L301) | `FIXME` | FIXME mp_footsteps needs to be a movevar | FIXME: mp_footsteps muss eine movevar sein | Mittlere Priorität: Footstep-Variable als movevar implementieren. |
| [313](pm_shared/pm_shared.cpp#L313) | `FIXME` | FIXME, move to player state | FIXME: In player state verschieben | Mittlere Priorität: Variable in den Spieler-Zustand auslagern. |
| [1489](pm_shared/pm_shared.cpp#L1489) | `FIXME` | if (0 == trace.startsolid && 0 == trace.allsolid) // FIXME: check steep slope? | FIXME: Steile Hänge prüfen? | Niedrige Priorität: Hangwinkel-Prüfung in Kollisionsberechnung einbauen. |
| [1806](pm_shared/pm_shared.cpp#L1806) | `TODO` | TODO: not really necessary to have separate arrays for client and server since the code is separate anyway. | TODO: Separate Arrays für Client und Server nicht wirklich nötig | Niedrige Priorität: Code vereinfachen durch gemeinsame Array-Nutzung. |
| [2142](pm_shared/pm_shared.cpp#L2142) | `HACK` | HACKHACK - Fudge for collision bug - no time to fix this properly | HACKHACK: Fudge-Faktor für Kollisions-Bug – keine Zeit für saubere Lösung | Mittlere Priorität: Kollisions-Bug grundlegend analysieren und beheben. |
| [2657](pm_shared/pm_shared.cpp#L2657) | `HACK` | HACK HACK HACK | HACK HACK HACK | Niedrige Priorität: Hack dokumentieren oder sauber implementieren. |


### `utils/`


#### `utils/common/cmdlib.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [833](utils/common/cmdlib.cpp#L833) | `FIXME` | FIXME: byte swap? | FIXME: Byte-Swap? | Niedrige Priorität: Byte-Reihenfolge prüfen für Plattformkompatibilität. |


#### `utils/common/mathlib.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [31](utils/common/mathlib.cpp#L31) | `FIXME` | length = sqrt(length); // FIXME | FIXME: sqrt(length) verwenden | Niedrige Priorität: Mathematische Funktion überprüfen. |
| [270](utils/common/mathlib.cpp#L270) | `FIXME` | FIXME: rescale the inputs to 1/2 angle | FIXME: Eingaben auf halben Winkel skalieren | Mittlere Priorität: Winkel-Skalierung korrigieren. |


#### `utils/common/wadlib.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [296](utils/common/wadlib.cpp#L296) | `FIXME` | FIXME: do compression | FIXME: Kompression implementieren | Mittlere Priorität: Kompressionsalgorithmus für WAD-Daten implementieren. |


#### `utils/makefont/makefont.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [305](utils/makefont/makefont.cpp#L305) | `FIXME` | FIXME:  Enable true palette support in engine? | FIXME: Echte Palettenunterstützung in der Engine aktivieren? | Niedrige Priorität: Palettenunterstützung für Engine-Fonts prüfen. |
| [333](utils/makefont/makefont.cpp#L333) | `FIXME` | FIXME:  Enable true palette support in engine? | FIXME: Echte Palettenunterstützung in der Engine aktivieren? | Niedrige Priorität: Palettenunterstützung für Engine-Fonts prüfen. |


#### `utils/mdlviewer/studio_render.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [513](utils/mdlviewer/studio_render.cpp#L513) | `TODO` | TODO: only do it for bones that actually have textures | TODO: Nur für Knochen mit Texturen ausführen | Niedrige Priorität: Render-Loop optimieren durch Textur-Filter. |


#### `utils/qbsp2/bsp5.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [61](utils/qbsp2/bsp5.h#L61) | `FIXME` | vec3_t pts[MAXEDGES]; // FIXME: change to use winding_t | FIXME: Auf winding_t umstellen | Niedrige Priorität: Datenstruktur modernisieren. |


#### `utils/qrad/lightmap.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [859](utils/qrad/lightmap.cpp#L859) | `BUG` | VectorFill(p->totallight, 0); // all sent now  // BUGBUG for progressive refinement runs | BUGBUG: Für progressive Verfeinerungsläufe problematisch | Mittlere Priorität: Lichtmap-Akkumulation für progressive Läufe korrigieren. |
| [1490](utils/qrad/lightmap.cpp#L1490) | `BUG` | vec3_t v; // BUGBUG: Use a weighted average instead? | BUGBUG: Gewichteten Durchschnitt verwenden? | Mittlere Priorität: Lichtmap-Berechnung mit gewichtetem Durchschnitt verbessern. |


#### `utils/qrad/qrad.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [314](utils/qrad/qrad.cpp#L314) | `FIXME` | sum[i] += mt[][x][y][i] // FIXME later | FIXME: Später korrigieren | Niedrige Priorität: Radiosity-Berechnung überarbeiten. |


#### `utils/serverctrl/ServerCtrlDlg.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [273](utils/serverctrl/ServerCtrlDlg.cpp#L273) | `TODO` | TODO: Add extra initialization here | TODO: Zusätzliche Initialisierung hier einfügen | Niedrige Priorität: Dialog-Initialisierung vervollständigen. |
| [474](utils/serverctrl/ServerCtrlDlg.cpp#L474) | `FIXME` | FIXME:  You'll want to fill in your executable path here, of course. | FIXME: Ausführbaren Pfad hier einsetzen | Niedrige Priorität: Pfad-Konfiguration implementieren. |


#### `utils/sprgen/spritegn.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [61](utils/sprgen/spritegn.h#L61) | `TODO` | TODO: shorten these? | TODO: Diese kürzen? | Niedrige Priorität: Konstantennamen kürzen für bessere Lesbarkeit. |


#### `utils/studiomdl/studiomdl.cpp`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [1486](utils/studiomdl/studiomdl.cpp#L1486) | `FIXME` | FIXME losing texture coord resultion! | FIXME: Texturkoordinaten-Auflösung geht verloren! | Hohe Priorität: Texturkoordinaten-Präzision beim Kompilieren erhalten. |
| [1647](utils/studiomdl/studiomdl.cpp#L1647) | `TODO` | TODO: process the texture and flag it if fullbright or transparent are used. | TODO: Textur verarbeiten und als fullbright oder transparent markieren | Mittlere Priorität: Textur-Eigenschaften automatisch erkennen und setzen. |
| [1648](utils/studiomdl/studiomdl.cpp#L1648) | `TODO` | TODO: only save as many palette entries as are actually used. | TODO: Nur verwendete Paletteneinträge speichern | Niedrige Priorität: Palette optimieren um Speicher zu sparen. |
| [1836](utils/studiomdl/studiomdl.cpp#L1836) | `FIXME` | FIXME : Hey, it's orthogical so inv(A) == transpose(A) | FIXME: Orthogonale Matrix – inv(A) == transpose(A) | Mittlere Priorität: Matrixinversion durch Transposition ersetzen für Effizienz. |


#### `utils/studiomdl/studiomdl.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [290](utils/studiomdl/studiomdl.h#L290) | `FIXME` | FIXME: what about texture overrides inline with loading models | FIXME: Was ist mit Textur-Overrides beim Laden von Modellen? | Mittlere Priorität: Textur-Override-Logik beim Modell-Laden implementieren. |


#### `utils/vgui/include/VGUI.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [44](utils/vgui/include/VGUI.h#L44) | `TODO` | TODO: Look and Feel support | TODO: Look and Feel Unterstützung | Niedrige Priorität: Look-and-Feel-System für VGUI implementieren. |
| [57](utils/vgui/include/VGUI.h#L57) | `TODO` | TODO: Figure out the 'Valve' Look and Feel and implement that instead of a the Java one | TODO: Valve Look and Feel herausfinden und implementieren statt Java-LnF | Niedrige Priorität: Valve-spezifisches UI-Design implementieren. |
| [58](utils/vgui/include/VGUI.h#L58) | `TODO` | TODO: Determine ownership policy for Borders, Layouts, etc.. | TODO: Eigentumsrichtlinie für Borders, Layouts usw. bestimmen | Niedrige Priorität: Speicherverwaltung für UI-Komponenten definieren. |
| [59](utils/vgui/include/VGUI.h#L59) | `TODO` | TODO: tooltips support | TODO: Tooltip-Unterstützung | Niedrige Priorität: Tooltip-System implementieren. |
| [60](utils/vgui/include/VGUI.h#L60) | `TODO` | TODO: ComboKey (hot key support) | TODO: ComboKey (Hotkey-Unterstützung) | Niedrige Priorität: Hotkey-System implementieren. |
| [61](utils/vgui/include/VGUI.h#L61) | `TODO` | TODO: add Background.cpp, remove paintBackground from all components | TODO: Background.cpp hinzufügen, paintBackground aus allen Komponenten entfernen | Niedrige Priorität: Hintergrund-Rendering zentralisieren. |
| [64](utils/vgui/include/VGUI.h#L64) | `TODO` | TODO: Builtin components should never overide paintBackground, only paint | TODO: Eingebaute Komponenten sollen nie paintBackground überschreiben, nur paint | Niedrige Priorität: Render-Hierarchie für UI-Komponenten einhalten. |
| [65](utils/vgui/include/VGUI.h#L65) | `TODO` | TODO: All protected members should be converted to private | TODO: Alle protected Members in private umwandeln | Niedrige Priorität: Kapselung durch private Member verbessern. |
| [66](utils/vgui/include/VGUI.h#L66) | `TODO` | TODO: All member variables should be moved to the top of the class prototype | TODO: Alle Member-Variablen nach oben in das Klassen-Prototyp verschieben | Niedrige Priorität: Code-Organisation verbessern. |
| [67](utils/vgui/include/VGUI.h#L67) | `TODO` | TODO: All private methods should be prepended with private | TODO: Alle privaten Methoden mit private voranstellen | Niedrige Priorität: Namenskonvention für private Methoden einhalten. |
| [68](utils/vgui/include/VGUI.h#L68) | `TODO` | TODO: Use of word internal in method names is not consistent and confusing | TODO: Verwendung von 'internal' in Methodennamen ist inkonsistent und verwirrend | Niedrige Priorität: Namenskonvention für interne Methoden vereinheitlichen. |
| [69](utils/vgui/include/VGUI.h#L69) | `TODO` | TODO: Cleanup so bullshit publics are properly named, maybe even figure out | TODO: Unsinnige öffentliche Methoden korrekt benennen | Niedrige Priorität: API bereinigen und korrekt benennen. |
| [71](utils/vgui/include/VGUI.h#L71) | `TODO` | TODO: Breakup InputSignal into logical pieces | TODO: InputSignal in logische Teile aufteilen | Niedrige Priorität: InputSignal-Interface refaktorieren. |
| [72](utils/vgui/include/VGUI.h#L72) | `TODO` | TODO: Button is in a state of disarray, it should have ButtonModel support | TODO: Button sollte ButtonModel-Unterstützung haben | Niedrige Priorität: Button-Modell-Architektur implementieren. |
| [73](utils/vgui/include/VGUI.h#L73) | `TODO` | TODO: get rid of all the stupid strdup laziness, convert to vgui_strdup | TODO: strdup-Faulheit loswerden, zu vgui_strdup konvertieren | Niedrige Priorität: Speicherverwaltung durch VGUI-eigene String-Kopie verbessern. |
| [74](utils/vgui/include/VGUI.h#L74) | `TODO` | TODO: actually figure out policy on String and implement it consistently | TODO: Richtlinie für String tatsächlich festlegen und konsistent umsetzen | Niedrige Priorität: String-Handling-Richtlinie definieren und umsetzen. |
| [75](utils/vgui/include/VGUI.h#L75) | `TODO` | TODO: implement createLayoutInfo for other Layouts than need it | TODO: createLayoutInfo für andere Layouts implementieren | Niedrige Priorität: Layout-Factory-Methode vervollständigen. |
| [76](utils/vgui/include/VGUI.h#L76) | `TODO` | TODO: BorderLayout should have option for a null LayoutInfo defaulting to center | TODO: BorderLayout sollte null-LayoutInfo mit Standardzentrierung unterstützen | Niedrige Priorität: Standard-LayoutInfo für BorderLayout implementieren. |
| [77](utils/vgui/include/VGUI.h#L77) | `TODO` | TODO: SurfaceBase should go away, put it in Surface | TODO: SurfaceBase sollte verschwinden, in Surface integriert werden | Niedrige Priorität: Klassen-Hierarchie vereinfachen. |
| [78](utils/vgui/include/VGUI.h#L78) | `TODO` | TODO: ActionSignals and other Signals should just set a flag when they fire. | TODO: ActionSignals sollen nur ein Flag setzen wenn sie feuern | Niedrige Priorität: Signal-Architektur vereinfachen. |
| [80](utils/vgui/include/VGUI.h#L80) | `TODO` | TODO: Change all method naming to starting with a capital letter. | TODO: Alle Methodennamen mit Großbuchstaben beginnen | Niedrige Priorität: Namenskonvention einheitlich umsetzen. |


#### `utils/vgui/include/VGUI_App.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [32](utils/vgui/include/VGUI_App.h#L32) | `TODO` | TODO: the public and public bullshit are all messed up, need to organize | TODO: Public und Bullshit-Public sind durcheinander, organisieren | Niedrige Priorität: Zugriffsmodifikatoren korrekt organisieren. |
| [33](utils/vgui/include/VGUI_App.h#L33) | `TODO` | TODO: actually all of the access needs to be properly thought out while you are at it | TODO: Zugriffsrechte überdenken | Niedrige Priorität: API-Design überarbeiten. |


#### `utils/vgui/include/VGUI_Border.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [13](utils/vgui/include/VGUI_Border.h#L13) | `TODO` | TODO: all borders should be titled | TODO: Alle Borders sollten einen Titel haben | Niedrige Priorität: Titel-Unterstützung für alle Border-Typen implementieren. |


#### `utils/vgui/include/VGUI_Button.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [22](utils/vgui/include/VGUI_Button.h#L22) | `TODO` | TODO: Button should be derived from an AbstractButton | TODO: Button sollte von AbstractButton abgeleitet sein | Niedrige Priorität: Klassenhierarchie für bessere Abstraktion überarbeiten. |


#### `utils/vgui/include/VGUI_Color.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [13](utils/vgui/include/VGUI_Color.h#L13) | `TODO` | TODO: rename getColor(r,g,b,a) to getRGBA(r,g,b,a) | TODO: getColor(r,g,b,a) in getRGBA(r,g,b,a) umbenennen | Niedrige Priorität: API-Konsistenz durch aussagekräftigere Methodennamen verbessern. |
| [14](utils/vgui/include/VGUI_Color.h#L14) | `TODO` | TODO: rename setColor(r,g,b,a) to setRGBA(r,g,b,a) | TODO: setColor(r,g,b,a) in setRGBA(r,g,b,a) umbenennen | Niedrige Priorität: API-Konsistenz durch aussagekräftigere Methodennamen verbessern. |
| [15](utils/vgui/include/VGUI_Color.h#L15) | `TODO` | TODO: rename getColor(sc) to getSchemeColor(sc) | TODO: getColor(sc) in getSchemeColor(sc) umbenennen | Niedrige Priorität: API-Konsistenz durch aussagekräftigere Methodennamen verbessern. |
| [16](utils/vgui/include/VGUI_Color.h#L16) | `TODO` | TODO: rename setColor(sc) to setSchemeColor(sc) | TODO: setColor(sc) in setSchemeColor(sc) umbenennen | Niedrige Priorität: API-Konsistenz durch aussagekräftigere Methodennamen verbessern. |


#### `utils/vgui/include/VGUI_FileInputStream.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [10](utils/vgui/include/VGUI_FileInputStream.h#L10) | `TODO` | TODO : figure out how to get stdio out of here, I think std namespace is broken for FILE for forward declaring does n... | TODO: stdio aus Header heraushalten | Niedrige Priorität: Header-Abhängigkeiten reduzieren. |


#### `utils/vgui/include/VGUI_Font.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [17](utils/vgui/include/VGUI_Font.h#L17) | `TODO` | TODO: cursors and fonts should work like gl binds | TODO: Cursor und Fonts sollten wie GL-Binds funktionieren | Niedrige Priorität: Ressource-Binding-System implementieren. |


#### `utils/vgui/include/VGUI_Image.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [14](utils/vgui/include/VGUI_Image.h#L14) | `TODO` | TODO:: needs concept of insets | TODO: Braucht Konzept für Insets | Niedrige Priorität: Inset-Unterstützung für Images implementieren. |


#### `utils/vgui/include/VGUI_Label.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [15](utils/vgui/include/VGUI_Label.h#L15) | `TODO` | TODO: this should use a TextImage for the text | TODO: Sollte TextImage für den Text verwenden | Niedrige Priorität: Label auf TextImage-Basis umstellen. |


#### `utils/vgui/include/VGUI_ListPanel.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [19](utils/vgui/include/VGUI_ListPanel.h#L19) | `TODO` | TODO: make a ScrollPanel and use a constrained one for _vpanel in ListPanel | TODO: ScrollPanel erstellen und im ListPanel verwenden | Niedrige Priorität: Scroll-Funktionalität in eigene Komponente auslagern. |


#### `utils/vgui/include/VGUI_TextImage.h`

| Zeile | Typ | Kommentar | Übersetzung (DE) | Empfehlung |
|-------|-----|-----------|------------------|------------|
| [14](utils/vgui/include/VGUI_TextImage.h#L14) | `TODO` | TODO: need to add wrapping flag instead of being arbitrary about when wrapping and auto-resizing actually happens | TODO: Umbruch-Flag hinzufügen statt willkürlich zu umbrechen | Niedrige Priorität: Text-Umbruch-Logik durch explizites Flag steuern. |
