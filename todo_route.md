# TODO Routemap – Priorisierte Abarbeitungsreihenfolge

> Erstellt auf Basis von `todo.md` und `agent.md`.  
> Nur **offene** Einträge werden hier aufgeführt (✅-markierte Einträge sind bereits erledigt).  
> Reihenfolge: Kritisch → Hoch → Mittel → Niedrig. Innerhalb jeder Stufe: Server-Code (`dlls/`) vor Client-Code (`cl_dll/`) vor Shared-Code (`pm_shared/`, `common/`) vor Tools (`utils/`).  
> **Out-of-scope**-Einträge (Engine-Interna, externe Libraries, VGUI-Architektur) sind am Ende gelistet.

---

## Stufe 1 – KRITISCH (Sicherheit / garantierte Crashes)

| # | Datei | Zeile | Typ | Problem | Maßnahme |
|---|-------|-------|-----|---------|----------|
| 1 | `cl_dll/vgui_StatsMenuPanel.cpp` | 368 | `TODO` | Beliebiger String als `printf`-Formatstring → Format-String-Injection | `DrawSetTextChar(str)` → `"%s", str` (oder entsprechende Wrapper-Funktion) |
| 2 | `cl_dll/vgui_StatsMenuPanel.cpp` | 453 | `TODO` | Identisches Problem an zweiter Stelle | Gleiche Maßnahme wie #1 |

---

## Stufe 2 – HOCH (Game-Breaking Bugs / Exploits / Abstürze)

| # | Datei | Zeile | Typ | Problem | Maßnahme |
|---|-------|-------|-----|---------|----------|
| 3 | `dlls/scripted.cpp` | 667 | `BUG` | `Killed()` wird nicht aufgerufen → Ressourcen-Leak / falsche Entitäts-Zustand | `Killed()`-Aufruf vor Entfernung ergänzen |
| 4 | `dlls/scripted.cpp` | 871 | `BUG` | `SetUse(NULL)` ohne `Killed()`-Aufruf | Gleiche Maßnahme wie #3 |
| 5 | `dlls/monsters.cpp` | 826 | `BUG` | Monster-Logik funktioniert noch nicht zu 100 % | Vollständige Analyse und Korrektur des BUGBUG-Pfads |
| 6 | `dlls/player.cpp` | 1642 | `BUG` | `FCAP_ONOFF_USE`-Objekte können nach Button-Release erneut ausgelöst werden | Use-Logik so anpassen, dass ein Objekt nur einmal pro Press-Release-Zyklus auslöst |
| 7 | `dlls/func_tank_of.cpp` | 143 | `TODO` | Exploit: Tank kann durch manipuliertes Signal das Ziel wechseln | Zugriffskontrolle / Sender-Validierung für Tank-Zielwechsel einbauen |
| 8 | `dlls/ctf/ctfplay_gamerules.cpp` | 505 | `TODO` | Index-Prüfung gegen nicht gesendeten Index → potenzieller Out-of-Bounds | Gesendete Indizes verfolgen und Prüfung vor Zugriff sicherstellen |
| 9 | `utils/studiomdl/studiomdl.cpp` | 1486 | `FIXME` | Texturkoordinaten-Auflösung geht beim Modell-Kompilieren verloren | Präzisions-Verlust in der Texturkoordinaten-Pipeline beheben |

---

## Stufe 3 – MITTEL (Korrektheitsfehler, fehlende Funktionalität in dlls/)

### 3A – Server-Side Monster & NPC

| # | Datei | Zeile | Typ | Problem | Maßnahme |
|---|-------|-------|-----|---------|----------|
| 10 | `dlls/otis.cpp` | 639 | `TODO` | ✅ ~~Otis hat keinen Helm, aber sein Kopf ist kugelsicher~~ | ~~Kopfschutz-Flag für Otis entfernen~~ Erledigt |
| 11 | `dlls/baby_voltigore.cpp` | 205 | `TODO` | ✅ ~~Selbstschaden-Erkennung fehlt (kein Attacker-Filter)~~ | ~~Attacker-Filter in `TraceAttack` einbauen~~ Erledigt |
| 12 | `dlls/voltigore.cpp` | 476 | `TODO` | ✅ ~~Identisches Problem wie #11~~ | ~~Gleiche Maßnahme~~ Erledigt |
| 13 | `dlls/hgrunt_medic.cpp` | 405 | `TODO` | ✅ ~~Wahrscheinlich falsche Logik (übernommen aus Original)~~ | ~~Logik prüfen und ggf. korrigieren~~ Erledigt: pGun initialisiert |
| 14 | `dlls/hgrunt_medic.cpp` | 2937 | `TODO` | ✅ ~~Funktionalität fehlt beim Medic-Grunt~~ | ~~Fehlende Funktion identifizieren und ergänzen~~ Erledigt: Provokations-Check aktiviert |
| 15 | `dlls/hgrunt_medic.cpp` | 2961 | `TODO` | Nicht für Multiplayer geeignet | Multiplayer-Kompatibilität herstellen |
| 16 | `dlls/geneworm.cpp` | 166 | `TODO` | Modulo-Verhalten im Original möglicherweise ignoriert | Original-Code analysieren, Abweichung korrigieren |
| 17 | `dlls/geneworm.cpp` | 1137 | `TODO` | ✅ ~~Hardcoded-Wert~~ | ~~Als benannte Konstante auslagern~~ Erledigt |
| 18 | `dlls/headcrab.cpp` | 417 | `BUG` | ✅ ~~`ACT_RANGE_ATTACK2`-Animation existiert nicht – toter Code~~ | ~~Toten Animations-Code entfernen oder passende Animation ergänzen~~ Erledigt |
| 19 | `dlls/shockroach.cpp` | 443 | `BUG` | ✅ ~~Identisches Problem wie #18~~ | ~~Gleiche Maßnahme~~ Erledigt |
| 20 | `dlls/monsters.cpp` | 811 | `TODO` | ✅ ~~Bit-Check vs. Vergleich unklar~~ | ~~Korrekte Operation (Bit-Check oder Vergleich) verifizieren~~ Erledigt |
| 21 | `dlls/schedule.cpp` | 83 | `TODO` | ✅ ~~Falsche Classname-Prüfmethode~~ | ~~Korrekte Classname-Prüfung implementieren (wie in `FClassnameIs`)~~ Erledigt |

### 3B – Server-Side Waffen

| # | Datei | Zeile | Typ | Problem | Maßnahme |
|---|-------|-------|-----|---------|----------|
| 22 | `dlls/weapons/CKnife.cpp` | 167 | `TODO` | ✅ ~~Backstab geht nur gegen Spieler, nicht gegen NPCs~~ | ~~NPC-Backstab-Unterstützung implementieren~~ Erledigt |
| 23 | `dlls/weapons/CGrappleTip.cpp` | 165 | `TODO` | Grapple-Spitze ignoriert `sv_maxvelocity` | Geschwindigkeit auf `sv_maxvelocity` begrenzen |
| 24 | `dlls/weapons/CDisplacerBall.cpp` | 320 | `TODO` | ✅ ~~Kein `SetNextThink` nach bestimmtem State~~ | ~~Think-Zeitpunkt setzen für korrekte Zustandsmaschine~~ Erledigt |
| 25 | `dlls/weapons/CPenguin.cpp` | 24 | `TODO` | Penguin-Variable wird nicht korrekt gespeichert (Vanilla-Inkompatibilität) | Save/Restore-Kompatibilität mit Vanilla Op4 sicherstellen |
| 26 | `dlls/weapons.cpp` | 1033 | `TODO` | ✅ ~~Rückgabewert `-1` bei Munitionsentnahme wird nicht behandelt~~ | ~~Rückgabewert-Logik korrigieren~~ Erledigt |
| 27 | `dlls/weapons/CSniperRifle.cpp` | 165 | `TODO` | Logik macht keinen Sinn | Überarbeiten und dokumentieren |

### 3C – Server-Side Entities & Triggers

| # | Datei | Zeile | Typ | Problem | Maßnahme |
|---|-------|-------|-----|---------|----------|
| 28 | `dlls/func_tank.cpp` | 491 | `TODO` | Controller-Waffe wird beim Verlassen des Tanks nicht wiederhergestellt | Waffenzustand speichern/wiederherstellen |
| 29 | `dlls/func_tank_of.cpp` | 400 | `TODO` | Identisches Problem wie #28 | Gleiche Maßnahme |
| 30 | `dlls/blowercannon.cpp` | 57 | `TODO` | ✅ ~~Wahrscheinliches Name-Shadowing gegenüber `CBaseDelay`~~ | ~~Namenskonflikt prüfen und beheben~~ Erledigt |
| 31 | `dlls/blowercannon.cpp` | 99 | `TODO` | ✅ ~~`Spawn()`/`Precache()` ruft Basisklasse nicht auf~~ | ~~`CBaseDelay::Spawn()` o.ä. aufrufen~~ Erledigt |
| 32 | `dlls/triggers.cpp` | 2888 | `TODO` | ✅ ~~`team_no`-Eingabe wird nicht validiert~~ | ~~Bereichsprüfung vor Verwendung einbauen~~ Erledigt |
| 33 | `dlls/nuclearbomb.cpp` | 217 | `TODO` | ✅ ~~Classname-Member für beide Entitäten fehlen~~ | ~~Classnames korrekt setzen~~ Erledigt |
| 34 | `dlls/tentacle.cpp` | 75 | `TODO` | Methoden-Signatur weicht von Basisklasse ab | Signatur anpassen oder Adapter implementieren |
| 35 | `dlls/pathcorner.cpp` | 358 | `HACK` | Sackgassen-Erkennung ist ein Hack | Sauberere Logik für Sackgassen-Behandlung im Pfadsystem |
| 36 | `dlls/buttons.cpp` | 1267 | `BUG` | Button-Trigger verursacht Latenz beim Wiederauslösen | Trigger-Logik überarbeiten |

### 3D – Server-Side Spieler & Spielregeln

| # | Datei | Zeile | Typ | Problem | Maßnahme |
|---|-------|-------|-----|---------|----------|
| 37 | `dlls/player.cpp` | 406 | `TODO` | Schadenserhöhung wird auf schlechte Art gehandhabt | Schadensmodifier-System überarbeiten |
| 38 | `dlls/player.cpp` | 1525 | `TODO` | HUD-Update wird nicht gesendet | `gmsgHUDColor` o.ä. Nachricht beim nötigen Zeitpunkt senden |
| 39 | `dlls/player.cpp` | 2818 | `BUG` | Wasser-Physik-Bug (tritt ständig im Wasser auf) | Bug analysieren und beheben |
| 40 | `dlls/player.cpp` | 2819 | `BUG` | Strömungskraft des Wassers wird falsch berechnet | Korrekte Strömungseinfluss-Berechnung |
| 41 | `dlls/player.cpp` | 4443 | `TODO` | Spieltitel-Anzeige funktioniert im Multiplayer nicht | Multiplayer-Pfad implementieren |
| 42 | `dlls/client.cpp` | 436 | `TODO` | ✅ ~~Cvar-Wert kann negativ werden~~ | ~~Clamping auf ≥ 0 einbauen~~ Erledigt |
| 43 | `dlls/multiplay_gamerules.cpp` | 358 | `TODO` | Member wird direkt statt per Accessor modifiziert | Accessor-Methode verwenden |
| 44 | `dlls/teamplay_gamerules.cpp` | 29 | `TODO` | Globale Variablen gehören als Member in `CHalfLifeTeamplay` | In Klassenmember umwandeln |

### 3E – CTF

| # | Datei | Zeile | Typ | Problem | Maßnahme |
|---|-------|-------|-----|---------|----------|
| 45 | `dlls/ctf/CItemCTF.cpp` | 37 | `TODO` | ✅ ~~Basisklassen-`KeyValue` wird nicht aufgerufen~~ | ~~`CBaseEntity::KeyValue(pkvd)` aufrufen~~ Erledigt |
| 46 | `dlls/ctf/CItemCTF.cpp` | 240 | `TODO` | Index-basierter Zugriff unsicher | Durch sicheren Bezeichner ersetzen |
| 47 | `dlls/ctf/CItemBackpackCTF.cpp` | 84 | `TODO` | ✅ ~~Precache-Aufrufe stehen nicht in `Precache()`~~ | ~~In `Precache()`-Methode verschieben~~ Erledigt |
| 48 | `dlls/ctf/CItemBackpackCTF.cpp` | 93 | `TODO` | ✅ ~~Modell wird nicht über `pev->model` gesetzt~~ | ~~`pev->model` einheitlich verwenden~~ Erledigt |
| 49 | `dlls/ctf/CItemLongJumpCTF.cpp` | 78 | `TODO` | ✅ ~~Precache-Aufrufe nicht in `Precache()`~~ | ~~In `Precache()`-Methode verschieben~~ Erledigt |
| 50 | `dlls/ctf/CItemLongJumpCTF.cpp` | 87 | `TODO` | ✅ ~~Modell nicht über `pev->model`~~ | ~~`pev->model` verwenden~~ Erledigt |
| 51 | `dlls/ctf/CItemPortableHEVCTF.cpp` | 81 | `TODO` | ✅ ~~Modell nicht über `pev->model`~~ | ~~`pev->model` verwenden~~ Erledigt |
| 52 | `dlls/ctf/CItemRegenerationCTF.cpp` | 80 | `TODO` | ✅ ~~Precache-Aufrufe nicht in `Precache()`~~ | ~~In `Precache()`-Methode verschieben~~ Erledigt |
| 53 | `dlls/ctf/CItemRegenerationCTF.cpp` | 89 | `TODO` | ✅ ~~Modell nicht über `pev->model`~~ | ~~`pev->model` verwenden~~ Erledigt |
| 54 | `dlls/ctf/ctfplay_gamerules.cpp` | 129 | `TODO` | ✅ ~~Verlier-Logik für Team 0 ergibt keinen Sinn~~ | ~~Korrekte Auswertung implementieren~~ Erledigt |

### 3F – Client-Side HUD & UI

| # | Datei | Zeile | Typ | Problem | Maßnahme |
|---|-------|-------|-----|---------|----------|
| 55 | `cl_dll/vgui_StatsMenuPanel.cpp` | 128 | `BUG` | YRES wird für X-Koordinate verwendet | Durch XRES ersetzen |
| 56 | `cl_dll/hud_msg.cpp` | 92 | `TODO` | Reset nicht bei Kartenwechsel | Bei `HUD_RESET`-Event aufrufen |
| 57 | `cl_dll/hud_msg.cpp` | 163 | `TODO` | Visuelles Treffer-Feedback fehlt | Kamera-Kick und Schadens-Visualisierung implementieren |
| 58 | `cl_dll/health.cpp` | 100 | `TODO` | Lokale Gesundheitsdaten werden nicht aktualisiert | Synchronisation mit Server-Health-Daten einbauen |
| 59 | `cl_dll/hud_spectator.cpp` | 121 | `TODO` | Spectator-Code für Op4 fehlt oder ist Dead-Code | Op4-Spectator implementieren oder toten Code entfernen |
| 60 | `cl_dll/ev_hldm.cpp` | 1285 | `TODO` | Clientseitige Vorhersage für fliegenden Bolzen fehlt | Bolzen-Vorhersage implementieren |
| 61 | `cl_dll/vgui_TeamFortressViewport.cpp` | 2193 | `TODO` | Op4-spezifische Implementierung fehlt | Implementierung ergänzen |
| 62 | `cl_dll/demo.cpp` | 28 | `FIXME` | Kein typsicheres Buffer-Casting | Hilfsfunktionen für Buffer-Casting einführen |
| 63 | `cl_dll/hl/hl_weapons.cpp` | 659 | `FIXME` | Waffenlogik sollte als Methode je Waffe implementiert sein | Waffenspezifische Methode mit `entity_state_t*` einführen |
| 64 | `cl_dll/particleman/CBaseParticle.cpp` | 101 | `TODO` | Partikelrichtungs-Berechnung möglicherweise falsch | Berechnung verifizieren und dokumentieren |

### 3G – Shared Player-Movement & Sound

| # | Datei | Zeile | Typ | Problem | Maßnahme |
|---|-------|-------|-----|---------|----------|
| 65 | `pm_shared/pm_shared.cpp` | 301 | `FIXME` | `mp_footsteps` ist keine movevar | Als movevar registrieren |
| 66 | `pm_shared/pm_shared.cpp` | 313 | `FIXME` | Variable gehört in Player-State | In `player_state` verschieben |
| 67 | `pm_shared/pm_shared.cpp` | 2142 | `HACK` | Kollisions-Bug mit Fudge-Faktor umgangen | Bug analysieren und sauber lösen |
| 68 | `dlls/sound.cpp` | 156 | `HACK` | Save/Restore-Design verletzt | Save/Restore-Logik für Sound überarbeiten |
| 69 | `dlls/sound.cpp` | 625 | `HACK` | Precache nach Save/Restore funktioniert nur durch Hack | Saubere Precache-Wiederherstellung implementieren |

### 3H – Build-Tools (in-scope: Modell-Compiler)

| # | Datei | Zeile | Typ | Problem | Maßnahme |
|---|-------|-------|-----|---------|----------|
| 70 | `utils/studiomdl/studiomdl.cpp` | 1647 | `TODO` | Fullbright/Transparent-Erkennung fehlt | Textur-Eigenschaften automatisch setzen |
| 71 | `utils/studiomdl/studiomdl.cpp` | 1836 | `FIXME` | Matrixinversion statt effizienterer Transposition | Transposition für orthogonale Matrizen verwenden |
| 72 | `utils/studiomdl/studiomdl.h` | 290 | `FIXME` | Textur-Override-Logik beim Modell-Laden fehlt | Override-Unterstützung implementieren |
| 73 | `utils/common/wadlib.cpp` | 296 | `FIXME` | WAD-Kompression nicht implementiert | Kompressionsalgorithmus ergänzen |

---

## Stufe 4 – NIEDRIG (Refactoring, Cleanup, Code-Qualität)

> Diese Einträge sind funktional nicht kritisch. Reihenfolge: Zuerst Einträge mit breiteren Auswirkungen (Shared-Code, häufig genutzte Dateien), dann spezifische einzelne Stellen.

### 4A – Wiederkehrende Muster (mehrere Dateien, am effizientesten gebündelt)

| # | Problem | Betroffene Dateien |
|---|---------|-------------------|
| 74 | Magic-Numbers durch benannte Konstanten ersetzen | `dlls/util.cpp:1412`, `dlls/tripmine.cpp:97`, `cl_dll/input.cpp:314`, `cl_dll/hud_msg.cpp:117`, `dlls/weapons/CSniperRifle.cpp:142` |
| 75 | Toten Code / ungenutzte Variablen entfernen | `dlls/weapons.h:787` (`m_fInReload`), `dlls/geneworm.cpp:1336`, `dlls/pitworm_up.cpp:982`, `dlls/ctf/CItemAcceleratorCTF.cpp:39`, `dlls/ctf/ctfplay_gamerules.cpp:702`, `dlls/op4mortar.cpp:188`, `dlls/rope/CRopeSample.cpp:38` |
| 76 | Doppelten Code in Hilfsfunktion auslagern | `dlls/gargantua.cpp:1342` (kopiert aus `explode.cpp`) |
| 77 | GetGunPosition einheitlich nutzen | `dlls/hgrunt_ally.cpp:573`, `dlls/hgrunt_medic.cpp:586`, `dlls/hgrunt_torch.cpp:551` |
| 78 | Debug-Code nach Tests entfernen | `dlls/player.cpp:4076`, `dlls/player.cpp:4130`, `cl_dll/vgui_ScorePanel.cpp:277` |

### 4B – Einzelne Stellen

| # | Datei | Zeile | Problem | Maßnahme |
|---|-------|-------|---------|----------|
| 79 | `common/PlatformHeaders.h` | 61 | `ARRAYSIZE` durch `std::size` ersetzen | Modernisierung auf C++17-Standard |
| 80 | `cl_dll/particleman/particleman_internal.h` | 24 | Workaround nach Entfernung des clamp-Makros bereinigen | clamp-Makro zuerst entfernen, dann Workaround löschen |
| 81 | `cl_dll/particleman/CFrustum.cpp` | 25, 48 | Unnötiger Parameter, 4×4-Matrix-Multiplikation aufräumen | Parameter entfernen, Standard-Matrix-Op nutzen |
| 82 | `cl_dll/particleman/CBaseParticle.cpp` | 209 | Y-Stretch-Skalierung fehlt | Partikelskalierung auf Y-Achse prüfen |
| 83 | `cl_dll/hud_flagicons.cpp` | 60, 96 | Rückgabewert-Logik unklar, statischer Buffer | Analysieren und mit Vanilla abgleichen |
| 84 | `cl_dll/hud_spectator.cpp` | 668 | Hässlicher Code | Refaktorieren |
| 85 | `cl_dll/health.cpp` | 329 | Gesundheits-Offset-Berechnung fehlt | Korrekte Berechnung implementieren |
| 86 | `cl_dll/vgui_TeamFortressViewport.h` | 28 | Header-Struktur chaotisch | Refaktorieren und strukturieren |
| 87 | `dlls/h_battery.cpp` | 143, 151 | Überflüssige Prüfung und falsch platzierter Guard | Guard nach oben verschieben, Duplikat entfernen |
| 88 | `dlls/healthkit.cpp` | 81 | Falsche Prüfung (kein Respawn, aber Logik ist inkorrekt) | Prüfung für konsistentes Verhalten korrigieren |
| 89 | `dlls/handgrenade.cpp` | 71 | Fragwürdige Prüfung | Usefulness analysieren und dokumentieren |
| 90 | `dlls/shocktrooper.cpp` | 345 | Bodygroup-Wechsel fehlt | Bodygroup für Shocktruppen-Varianten implementieren |
| 91 | `dlls/shocktrooper.cpp` | 1138 | Verzeichnisnamen nicht kleingeschrieben | Pfade auf Kleinbuchstaben normalisieren |
| 92 | `dlls/gonome.cpp` | 88 | Mehrfache x-Zuweisung | Zu einer Zuweisung zusammenfassen |
| 93 | `dlls/geneworm.cpp` | 1202 | Physikalisch korrekte Geschwindigkeitsrichtung fehlt | Richtungsberechnung ergänzen |
| 94 | `dlls/pitdrone.cpp` | 153, 734 | FL_WORLDBRUSH fehlt, Wertebereich nicht begrenzt | Beides ergänzen |
| 95 | `dlls/pitworm_up.cpp` | 669 | Physikalisch korrekte Geschwindigkeitsrichtung fehlt | Richtungsberechnung ergänzen |
| 96 | `dlls/ctf/CItemCTF.cpp` | 90 | Duplizierten Code entfernen | Redundanten Aufruf löschen |
| 97 | `dlls/ctf/ctfplay_gamerules.cpp` | 70 | Zwei separate Listen statt einer | Datenstruktur aufteilen |
| 98 | `dlls/multiplay_gamerules.cpp` | 426, 974 | Unnötiger Cast, Ausgabe nicht direkt an Konsole | Cast entfernen, direkten Konsolen-Output nutzen |
| 99 | `dlls/util.cpp` | 2579 | Save-Game-Code nicht wiederverwendbar | Hilfsfunktionen auslagern |
| 100 | `dlls/weapons/CGrappleTip.cpp` | 33 | Interface für Größenabfrage fehlt | Sauberes Interface definieren |
| 101 | `dlls/weapons/CDisplacerBall.cpp` | 35 | Algorithmus kann verbessert werden | Refaktorieren |
| 102 | `dlls/rope/CRope.cpp` | 632, 819 | Konstante und nodraw-Flag-Steuerung | In gemeinsamen Header verschieben, nodraw-Flag effizienter nutzen |
| 103 | `dlls/hgrunt_ally.cpp` | 733, 1252 | Deaktivierte Funktion und fehlende Head-Variante | Prüfen, ob Aktivierung und Beret-Unterstützung sinnvoll |
| 104 | `dlls/hgrunt_medic.cpp` | 3039 | Typumwandlung vereinfachen | Direkten Typ verwenden |
| 105 | `dlls/squeakgrenade.cpp` | 501 | Ursprungsberechnung ist ein Hack | An neue Physik anpassen |
| 106 | `dlls/plats.cpp` | 1766 | Zug-Stopp-Logik ungenau | Halte-Logik präzisieren |
| 107 | `dlls/func_break.cpp` | 774 | Max. 256 Entitäten auf Breakable | Limit erhöhen oder dynamische Liste |
| 108 | `dlls/triggers.cpp` | 1042, 1060 | 32-Spieler-Limit im Bitfeld | Limit dokumentieren oder höheres Limit einführen |
| 109 | `dlls/triggers.cpp` | 2592 | Trigger nicht für Multiplayer gemacht | Multiplayer-Pfad ergänzen |
| 110 | `dlls/male_assassin.cpp` | 829, 2022 | Schaden-Balancing undokumentiert, unerwünschter Code | Dokumentieren bzw. entfernen |
| 111 | `dlls/gargantua.cpp` | 1065 | Flammen-Status nicht mit Entity-Lifecycle verknüpft | Kopplung implementieren |
| 112 | `pm_shared/pm_math.cpp` | 285, 302 | `sqrt(length)` statt `sqrt(length²)` möglicherweise falsch | Mathematische Formel verifizieren |
| 113 | `pm_shared/pm_shared.cpp` | 1489, 1806 | Hangwinkel-Prüfung fehlt, unnötige doppelte Arrays | Ergänzen bzw. vereinfachen |
| 114 | `cl_dll/ev_hldm.cpp` | 91 | Movevar-Prüfung für Textursounds fehlt | Prüfung ergänzen |
| 115 | `engine/studio.h` | 28, 29 | `MAXSTUDIOTRIANGLES`/`MAXSTUDIOVERTS` nach Profiling anpassen | Profiling durchführen, Limit justieren |
| 116 | `utils/studiomdl/studiomdl.cpp` | 1648 | Palette-Optimierung | Nur verwendete Einträge speichern |
| 117 | `utils/common/mathlib.cpp` | 31, 270 | sqrt-Formel und Winkel-Skalierung | Mathematik verifizieren und korrigieren |

---

## Außerhalb des Scope (nicht zu bearbeiten)

Die folgenden Kategorien sind laut `agent.md` **out of scope** (externe Bibliotheken, Engine-Interna, VGUI-Architektur-Redesigns) oder betreffen Third-Party-Code ohne direkte Auswirkung auf das Spiel:

- `external/SDL2/SDL_config_winrt.h` – SDL2-Bibliothek, nicht zu ändern
- `utils/vgui/include/VGUI*.h` – VGUI-Architektur-Redesign (Valve-Original), nur kosmetisch relevant
- `utils/vgui/include/VGUI_App.h` – VGUI-API-Redesign
- `utils/qbsp2/bsp5.h`, `utils/qrad/lightmap.cpp`, `utils/qrad/qrad.cpp` – BSP/Rad-Tools, Engine-nahe Build-Tools (fraglich ob Änderungen sinnvoll)
- `utils/serverctrl/ServerCtrlDlg.cpp` – Hilfstool, niedrige Relevanz
- `utils/makefont/makefont.cpp` – Hilfstool, Engine-Palette (nicht änderbar)
- `utils/mdlviewer/studio_render.cpp` – Viewer-Tool
- `utils/sprgen/spritegn.h` – Hilfstool
- `utils/common/cmdlib.cpp` – Byte-Order-Plattformfrage, Engine-nah

---

## Empfohlene Arbeitsreihenfolge (Zusammenfassung)

```
Stufe 1  →  Stufe 2  →  Stufe 3A  →  3B  →  3C  →  3D  →  3E  →  3F  →  3G  →  3H  →  Stufe 4
 (2 Items)   (7 Items)   (12 Items)  (6)    (11)    (8)    (10)    (10)    (5)    (4)    (44 Items)
```

Nach jeder erledigten Stufe: **Build auf Linux und Windows verifizieren** (`cd linux && make COMPILER=g++ CFG=release -j$(nproc)`).
