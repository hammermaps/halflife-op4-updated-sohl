# SOHL 1.4 → 1.5 Migrationsanleitung

Diese Anleitung beschreibt Schritt für Schritt, wie ein bestehendes **Spirit of Half-Life 1.4** Projekt auf **SOHL 1.5 alpha4** aktualisiert werden kann.

> **Hinweis:** Diese Anleitung basiert auf dem Commit `d0e2a63` ("Update to SOHL 1.5") und beschreibt die notwendigen Code-Änderungen sortiert nach Abhängigkeiten.

---

## Inhaltsverzeichnis

1. [Voraussetzungen](#1-voraussetzungen)
2. [Projekt-Konfiguration](#2-projekt-konfiguration)
3. [Basis-Änderungen (cbase)](#3-basis-änderungen-cbase)
4. [Server: Wettersystem (Regen)](#4-server-wettersystem-regen)
5. [Server: Inventar-System](#5-server-inventar-system)
6. [Server: Custom-Kill-Nachrichten](#6-server-custom-kill-nachrichten)
7. [Server: Waffen-Änderungen](#7-server-waffen-änderungen)
8. [Server: Entity-Fixes und Verbesserungen](#8-server-entity-fixes-und-verbesserungen)
9. [Server: Entfernte Dateien](#9-server-entfernte-dateien)
10. [Client: Wettersystem (Regen-Rendering)](#10-client-wettersystem-regen-rendering)
11. [Client: Kamera-System](#11-client-kamera-system)
12. [Client: Custom-Menu und Inventar-UI](#12-client-custom-menu-und-inventar-ui)
13. [Client: HUD-Änderungen](#13-client-hud-änderungen)
14. [Client: Rendering-Verbesserungen](#14-client-rendering-verbesserungen)
15. [Client: Sonstige Änderungen](#15-client-sonstige-änderungen)
16. [Zusammenfassung der neuen CVars und Befehle](#16-zusammenfassung-der-neuen-cvars-und-befehle)

---

## 1. Voraussetzungen

- Ein funktionierendes **SOHL 1.4** Projekt (kompilierbar)
- Visual Studio 6.0 oder kompatible IDE
- Backup des gesamten Projekts vor Beginn der Migration

**Empfohlene Reihenfolge:** Führe die Schritte in der angegebenen Reihenfolge durch, da spätere Änderungen auf früheren aufbauen.

---

## 2. Projekt-Konfiguration

### 2.1 Versionsstring aktualisieren

**Datei:** `dlls/gamerules.h`

```cpp
// ALT:
#define GAME_NAME "Spirit of Half-Life 1.4"

// NEU:
#define GAME_NAME "Spirit of Half-Life 1.5 alpha4"
```

### 2.2 Projektdateien aktualisieren

**Datei:** `cl_dll/cl_dll.dsp` – Neue Quelldateien zum Projekt hinzufügen:
- `CustomMenu.cpp`
- `rain.cpp`

**Datei:** `dlls/hl.dsp` – Build-Pfade und neue Quelldateien aktualisieren.

**Optional:** Neue Workspace-Datei `hl.dsw` aus SOHL 1.5 übernehmen.

### 2.3 IDE-Dateien bereinigen

Folgende Dateien können aus dem Repository entfernt werden (sie werden von der IDE automatisch generiert):
- `cl_dll/cl_dll.ncb`, `cl_dll/cl_dll.opt`, `cl_dll/cl_dll.plg`
- `dlls/hl.ncb`, `dlls/hl.opt`
- `cl_dll/hl/hl_baseentity.~cp`
- `dlls/Makefile.old`

---

## 3. Basis-Änderungen (cbase)

Diese Änderungen betreffen grundlegende Strukturen und müssen zuerst angewendet werden.

### 3.1 MAX_ITEMS erweitern

**Datei:** `dlls/cbase.h`

```cpp
// ALT:
#define MAX_ITEMS 5

// NEU:
#define MAX_ITEMS 10
```

### 3.2 Custom-Kill-Felder hinzufügen

**Datei:** `dlls/cbase.h` – In der `CBaseEntity`-Klasse:

```cpp
// NEU hinzufügen:
string_t killname;   // Custom Kill-Name für Todesnachrichten
string_t killmethod; // Custom Kill-Methode für Todesnachrichten
```

### 3.3 RADIATION_DURATION ändern

**Datei:** `dlls/cbase.h`

```cpp
// ALT:
#define RADIATION_DURATION 2

// NEU:
#define RADIATION_DURATION 10
```

### 3.4 TakeHealth Rückgabewert

**Datei:** `dlls/cbase.cpp` – `CBaseEntity::TakeHealth()` so ändern, dass die tatsächlich geheilte Menge zurückgegeben wird statt nur 0/1.

---

## 4. Server: Wettersystem (Regen)

### 4.1 Regen-Entities hinzufügen

**Datei:** `dlls/effects.cpp` – Folgende neue Klassen am Ende der Datei hinzufügen:

```cpp
// CRainSettings - Definiert Regen-Distanz und Modus
class CRainSettings : public CBaseEntity
{
public:
    void Spawn(void);
    void KeyValue(KeyValueData *pkvd);
    void Use(CBaseEntity *pActivator, CBaseEntity *pCaller,
             USE_TYPE useType, float value);

    int ObjectCaps(void) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }

    virtual int Save(CSave &save);
    virtual int Restore(CRestore &restore);
    static TYPEDESCRIPTION m_SaveData[];

    // Regen-Parameter
    int m_iDripsPerSecond;
    float m_flDistFromPlayer;
    float m_flWindX, m_flWindY;
    float m_flRandX, m_flRandY;
    int m_iWeatherMode; // 0 = Regen, 1 = Schnee
    float m_flGlobalHeight;
};
```

```cpp
// CRainModify - Steuert dynamische Regen-Parameter-Änderungen
class CRainModify : public CBaseEntity
{
public:
    void Spawn(void);
    void KeyValue(KeyValueData *pkvd);
    void Use(CBaseEntity *pActivator, CBaseEntity *pCaller,
             USE_TYPE useType, float value);

    int ObjectCaps(void) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }

    virtual int Save(CSave &save);
    virtual int Restore(CRestore &restore);
    static TYPEDESCRIPTION m_SaveData[];

    int m_iDripsPerSecond;
    float m_flWindX, m_flWindY;
    float m_flRandX, m_flRandY;
    float m_flFadeTime;
};
```

**Datei:** `dlls/effects.h` – Deklarationen für die Regen-Klassen hinzufügen.

### 4.2 Regen-Netzwerknachricht registrieren

**Datei:** `dlls/player.cpp` – Nachricht für Rain-Daten definieren:

```cpp
int gmsgRainData = 0;

// In der Message-Registrierung:
gmsgRainData = REG_USER_MSG("RainData", -1);
```

### 4.3 Regen-Variablen im Spieler hinzufügen

**Datei:** `dlls/player.h` – Neue Member zur `CBasePlayer`-Klasse:

```cpp
// Regen-Parameter
float Rain_dripsPerSecond;
float Rain_windX, Rain_windY;
float Rain_randX, Rain_randY;
int   Rain_weatherMode;
float Rain_globalHeight;

// Für Fade-Übergänge
float Rain_ideal_dripsPerSecond;
float Rain_ideal_windX, Rain_ideal_windY;
float Rain_ideal_randX, Rain_ideal_randY;
float Rain_endFade;
float Rain_nextFadeUpdate;
```

**Datei:** `dlls/player.cpp` – In `UpdateClientData()` die Regen-Daten an den Client senden:

```cpp
// pev ist hier der entvars_t* des Spielers (CBasePlayer::UpdateClientData)
MESSAGE_BEGIN(MSG_ONE, gmsgRainData, NULL, pev);
    WRITE_SHORT(Rain_dripsPerSecond);
    WRITE_COORD(raindistance);   // Entfernung zum Spieler
    WRITE_COORD(Rain_windX);
    WRITE_COORD(Rain_windY);
    WRITE_COORD(Rain_randX);
    WRITE_COORD(Rain_randY);
    WRITE_SHORT(Rain_weatherMode);
    WRITE_COORD(Rain_globalHeight);
MESSAGE_END();
```

---

## 5. Server: Inventar-System

### 5.1 Inventar-Items implementieren

**Datei:** `dlls/items.cpp` – Neue `I_Precache()` Funktion und folgende Klassen hinzufügen:

- `CItemMedicalKit` – Tragbares Medkit mit Ladesystem
- `CItemAntiRad` – Anti-Strahlungs-Spritze
- `CItemAntidote` – Gegengift (zu eigenständiger Klasse refaktorisieren)
- `CItemFlare` – Leuchtfackel mit anpassbaren Licht-Eigenschaften
- `CItemCamera` – Fernkamera (mit Platzieren/Betrachten/Zerstören)

**Datei:** `dlls/items.h` – Klassen-Deklarationen und Includes hinzufügen:

```cpp
#define MAX_ITEMS 10

class CItemMedicalKit : public CItem { /* ... */ };
class CItemAntiRad : public CItem { /* ... */ };
class CItemAntidote : public CItem { /* ... */ };
class CItemFlare : public CItem { /* ... */ };
class CItemCamera : public CItem { /* ... */ };

void I_Precache(void);
```

### 5.2 Inventar-Nachricht registrieren

**Datei:** `dlls/player.cpp`:

```cpp
int gmsgInventory = 0;

// In der Message-Registrierung:
gmsgInventory = REG_USER_MSG("Inventory", -1);
```

### 5.3 Spieler-Inventar-Variablen

**Datei:** `dlls/player.h` – Zur `CBasePlayer`-Klasse hinzufügen:

```cpp
CItemCamera *m_pItemCamera; // Aktive Kamera
int m_rgItems[MAX_ITEMS];   // Inventar-Slots (auf 10 erweitern)
```

### 5.4 Inventar in World-Precache einbinden

**Datei:** `dlls/world.cpp` – In `W_Precache()`:

```cpp
I_Precache(); // Alle Inventar-Items vorladen
```

### 5.5 Neue CVars registrieren

**Datei:** `dlls/game.cpp`:

```cpp
cvar_t max_medkit  = { "max_medkit", "200", FCVAR_SERVER };
cvar_t max_cameras = { "max_cameras", "2", FCVAR_SERVER };
cvar_t timed_damage = { "timed_damage", "1", FCVAR_SERVER };
```

### 5.6 Konsolenbefehl "inventory" hinzufügen

**Datei:** `dlls/client.cpp` – `inventory`-Befehl implementieren, der Items aktiviert/deaktiviert.

---

## 6. Server: Custom-Kill-Nachrichten

### 6.1 Multiplayer-Todesnachrichten erweitern

**Datei:** `dlls/multiplay_gamerules.cpp` – In der `DeathNotice()` Funktion:

```cpp
// Nach bestehender Waffen-Erkennung hinzufügen:
const char *killer_weapon_name = /* bestehender Code */;
const char *kill_technique = "";

// Prüfe auf Custom-Kill-Felder am Inflictor:
if (pInflictor)
{
    CBaseEntity *pInflictorEnt = CBaseEntity::Instance(pInflictor);
    if (pInflictorEnt)
    {
        if (pInflictorEnt->killname)
            killer_weapon_name = STRING(pInflictorEnt->killname);
        if (pInflictorEnt->killmethod)
            kill_technique = STRING(pInflictorEnt->killmethod);
    }
}

// Technik-String an den Client mitsenden
```

### 6.2 Client-seitige Todesnachrichten

**Datei:** `cl_dll/death.cpp` – Todesnachrichten-Parsing erweitern:

```cpp
// Trenne Technik-String am '%'-Zeichen in technique_A und technique_B
// Formatierte Ausgabe: "[Spieler] [technique_A] killed [Opfer] [technique_B]"
```

---

## 7. Server: Waffen-Änderungen

### 7.1 Holster-Zeiten reduzieren

In jeder Waffen-Datei die `Holster()`-Methode anpassen:

```cpp
// ALT (in jeder Waffen-Datei):
m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;

// NEU:
m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
```

**Betroffene Dateien:**
- `dlls/crossbow.cpp`
- `dlls/crowbar.cpp`
- `dlls/debugger.cpp`
- `dlls/egon.cpp`
- `dlls/gauss.cpp`
- `dlls/glock.cpp`
- `dlls/hornetgun.cpp` (von 2.0 auf 0.5)
- `dlls/mp5.cpp`
- `dlls/python.cpp`
- `dlls/rpg.cpp`
- `dlls/satchel.cpp`
- `dlls/shotgun.cpp`
- `dlls/squeakgrenade.cpp`
- `dlls/tripmine.cpp`

---

## 8. Server: Entity-Fixes und Verbesserungen

### 8.1 Rotierende Brushes

**Datei:** `dlls/bmodels.cpp`:
- `CFuncWall`: Rotation über „message"-Key-Parsing mit `UTIL_StringToVector()` hinzufügen
- `CFuncRotating`: Automatische Rotationsmittelpunkt-Erkennung wenn kein Origin gesetzt

### 8.2 Trigger-Fixes

**Datei:** `dlls/triggers.cpp`:
- `CTriggerHurt`: Aktivator als Angreifer zuweisen statt World-Entity
- `CBaseTrigger::ToggleUse`: Aktivator für Frag-Zuordnung speichern
- `CTargetFMODAudio`: Save/Restore für Wiedergabe-Status hinzufügen
- `PlayCDTrack()`: Audio-Dateipfade über `pev->message` unterstützen
- `CMotionManager`/`CMotionThread`: Null-Pointer-Workaround nach Save/Restore

### 8.3 Monster Maker Refactoring

**Datei:** `dlls/monstermaker.cpp`:
- Temporären `pWhere`-Entity-Pointer durch `pev->vuser1/2/3` ersetzen
- Dynamische Werte über `pev->noise`, `pev->noise1`, `pev->noise2`, `pev->noise3` unterstützen

### 8.4 Locus-System

**Datei:** `dlls/locus.cpp` – Neue Spawnflags für `CCalcSubVelocity`:

```cpp
#define SF_CALCVELOCITY_DISCARDX  (1 << 5)
#define SF_CALCVELOCITY_DISCARDY  (1 << 6)
#define SF_CALCVELOCITY_DISCARDZ  (1 << 7)
```

### 8.5 Sonstige Server-Fixes

| Datei | Änderung |
|-------|----------|
| `dlls/effects.cpp` | `env_light` Abklingparameter: Entity-Health statt berechneter Zeit |
| `dlls/effects.cpp` | `env_sky`: Skalierungsparameter (Byte) für Parallax hinzufügen |
| `dlls/effects.cpp` | `env_mirror`: FL_WORLDBRUSH, Positionierung, Reihenfolge-Fixes |
| `dlls/effects.cpp` | `CGibShooter`: Delay-Logik und Body-Initialisierung korrigiert |
| `dlls/effects.cpp` | Partikel: Aktivierung nur ohne Name, ansonsten Spawnflags beachten |
| `dlls/plats.cpp` | `CFuncTrain`: Sound nur bei Zustandsänderung neustarten |
| `dlls/scripted.cpp` | PossessEntity-Fixes für Script-Entity-Verhalten |
| `dlls/monsters.cpp` | `SCRIPT_EVENT_SENTENCE_RND1` bricht nicht vorzeitig ab |
| `dlls/leech.cpp` | Debug-Alert entfernt |
| `dlls/func_tank.cpp` | Konsolen-Alerts deaktiviert |
| `dlls/h_cycler.cpp` | Kleinere Ergänzung |

---

## 9. Server: Entfernte Dateien

Folgende Dateien aus dem Server-Projekt entfernen:

| Datei | Grund |
|-------|-------|
| `dlls/islave_deamon.cpp` | Duplicate/Alternate Alien-Slave Implementierung entfernt |
| `dlls/wpn_shared/hl_wpn_glock.cpp` | Glock-Implementierung verschoben |
| `dlls/Makefile.old` | Veraltete Linux-Build-Konfiguration |

> **Wichtig:** Beim Entfernen von `islave_deamon.cpp` sicherstellen, dass keine anderen Dateien auf darin definierte Klassen oder Funktionen verweisen.

---

## 10. Client: Wettersystem (Regen-Rendering)

### 10.1 Neue Dateien hinzufügen

Kopiere folgende Dateien aus dem SOHL 1.5 Quellcode in `cl_dll/`:

- **`rain.h`** – Header mit Definitionen:
  - Geschwindigkeitskonstanten: `DRIPSPEED` (900), `SNOWSPEED` (200)
  - Sprite-Konstanten und Limits: `MAXDRIPS` (2000), `MAXFX` (3000)
  - Strukturen: `rain_properties`, `cl_drip_t`, `cl_rainfx_t`

- **`rain.cpp`** – Implementierung:
  - `ProcessRain()` – Hauptupdate, Tropfen/Schneeflocken erzeugen und bewegen
  - `ProcessFXObjects()` – Wasserring-Effekte aktualisieren
  - `WaterLandingEffect()` – Wasser-Rippel bei Aufprall
  - `ResetRain()` – Aufräumen bei Levelwechsel
  - `InitRain()` – Initialisierung
  - `ParseRainFile()` – Lädt map-spezifische `.pcs`-Konfiguration

### 10.2 Regen-Rendering in tri.cpp

**Datei:** `cl_dll/tri.cpp` – Folgende Funktionen und Includes hinzufügen:

```cpp
#include "rain.h"
#include "com_model.h"
#include "studio_util.h"

void SetPoint(float x, float y, float z, float (*matrix)[4])
{
    // 3D-Punkt mit Matrix transformieren und als Vertex setzen
}

void DrawRain(void)
{
    // Regen-Sprite (sprites/hi_rain.spr) oder Schnee (sprites/snowflake.spr) laden
    // Render-Modus: kRenderTransAdd
    // Regen: Dreiecke senkrecht zur Kamera
    // Schnee: Quads zur Kamera ausgerichtet mit Matrix-Transformation
}

void DrawFXObjects(void)
{
    // Wasserring-Effekte (sprites/waterring.spr) rendern
    // Quad-basiertes Rendering mit Lifetime-basiertem Fading
}
```

In `HUD_DrawTransparentTriangles()` aufrufen:

```cpp
ProcessFXObjects();
ProcessRain();
DrawRain();
DrawFXObjects();
```

### 10.3 Regen in HUD integrieren

**Datei:** `cl_dll/hud.cpp`:

```cpp
#include "rain.h"

// In CHud::Init():
HOOK_MESSAGE(RainData);
CVAR_CREATE("cl_raininfo", "0", 0); // Debug-CVar

InitRain();

// In CHud::~CHud() und CHud::VidInit():
ResetRain();
```

**Datei:** `cl_dll/hud_msg.cpp`:

```cpp
int CHud::MsgFunc_RainData(const char *pszName, int iSize, void *pbuf)
{
    BEGIN_READ(pbuf, iSize);
    // Regen-Parameter auslesen und in rain_properties speichern
}
```

---

## 11. Client: Kamera-System

### 11.1 Third-Person-Kamera ersetzen

**Datei:** `cl_dll/in_camera.cpp` – Alte Third-Person-Kamera-Logik (~570 Zeilen) entfernen und durch vereinfachte Vorwärtsdeklarationen ersetzen:

```cpp
// New clipping style camera - original idea by Xwider
// Die meiste Kamera-Logik wurde nach view.cpp verschoben
```

### 11.2 Neue Third-Person-Kamera in view.cpp

**Datei:** `cl_dll/view.cpp`:

```cpp
// Neue Funktion:
void V_CalcThirdPersonRefdef(struct ref_params_s *pparams)
{
    // Third-Person-Berechnung mit Clipping
    // Sky-Parallax-Rendering mit konfigurierbarem Skalierungsfaktor
}

// Neue Konsolenbefehle registrieren:
// thirdperson, firstperson, drawplayer, hideplayer

// V_CalcNormalRefdef erweitern:
// - trigger_viewset-Unterstützung (viewFlags & 8 = Spieler zeichnen, & 4 = X invertieren)
// - m_iSkyScale für Parallax
// - SKY_ON_DRAWING Zustandsmaschine

// V_GetChasePos Signatur ändern:
// ALT: void V_GetChasePos(int target, float *cl_angles, float *origin, float *angles)
// NEU: void V_GetChasePos(cl_entity_t *ent, float *cl_angles, float *origin, float *angles)
```

### 11.3 HUD-Definitionen anpassen

**Datei:** `cl_dll/hud.h`:

```cpp
// Neue Member in CHud:
cvar_t *RainInfo;
int m_iSkyScale;    // Parallax-Skalierung (0 = keine Parallax)
int m_iCameraMode;  // Kamera-Modus

#define SKY_ON_DRAWING 2

// Neue Deklarationen:
int MsgFunc_RainData(const char *pszName, int iSize, void *pbuf);
int MsgFunc_Inventory(const char *pszName, int iSize, void *pbuf);

extern int g_iInventory[MAX_ITEMS];
```

### 11.4 Spectator-Anpassung

**Datei:** `cl_dll/hud_spectator.cpp` – `V_GetChasePos()` Aufrufe aktualisieren:

```cpp
// ALT:
V_GetChasePos(target, ...);

// NEU:
V_GetChasePos(gEngfuncs.GetEntityByIndex(target), ...);
```

---

## 12. Client: Custom-Menu und Inventar-UI

### 12.1 Custom-Menu hinzufügen

Kopiere **`cl_dll/CustomMenu.cpp`** aus SOHL 1.5 – enthält die `CCustomMenu`-Klasse für interaktive Klassenauswahl.

### 12.2 VGUI-Integration

**Datei:** `cl_dll/vgui_TeamFortressViewport.cpp`:

```cpp
// Globales Inventar-Array:
int g_iInventory[MAX_ITEMS];
char *sLocalisedInventory[] = { "#Item1", "#Item2", ..., "#Item10" };
char *sInventorySelection[] = { "inventory 1", "inventory 2", ..., "inventory 10" };

// CreateCustomButton() für !SHOWINVENTORY erweitern
// MENU_CUSTOM Case in ShowVGUIMenu() hinzufügen
// CreateCustomMenu() und ShowCustomMenu() implementieren
```

**Datei:** `cl_dll/vgui_TeamFortressViewport.h`:

```cpp
// Neue Member:
CCustomMenu *m_pCustomMenu;

// Neue Methoden:
void CreateCustomMenu(void);
CCustomMenu *ShowCustomMenu(void);
```

### 12.3 Menu-Definition

**Datei:** `cl_dll/tf_defs.h`:

```cpp
#define MENU_CUSTOM 10
```

### 12.4 Eingabe-Befehle

**Datei:** `cl_dll/input.cpp`:

```cpp
// Neue kbutton-Eingaben:
kbutton_t in_customhud;
kbutton_t in_briefing;

// Neue Befehle registrieren: +hud/-hud, +briefing/-briefing
```

---

## 13. Client: HUD-Änderungen

### 13.1 Kamera-HUD-Anzeige

**Datei:** `cl_dll/hud_redraw.cpp`:

```cpp
// Kamera-Aktivitäts-Sprite (blinkt mit 1 FPS):
// Sprites: camera_active, camera_rect_tl, camera_rect_tr, camera_rect_bl, camera_rect_br
// Aktiv wenn: viewFlags & 1 (Custom-View) UND viewFlags & 4 (Kamera)
```

### 13.2 Inventar-Nachrichten-Handler

**Datei:** `cl_dll/hud_msg.cpp`:

```cpp
int CHud::MsgFunc_Inventory(const char *pszName, int iSize, void *pbuf)
{
    BEGIN_READ(pbuf, iSize);
    int index = READ_BYTE();  // 0 = Reset (Tod)
    if (index == 0) {
        memset(g_iInventory, 0, sizeof(g_iInventory));
    } else {
        g_iInventory[index] = READ_SHORT();
    }
    return 1;
}
```

### 13.3 Sky-Scale und Mirror-Reset

**Datei:** `cl_dll/hud_msg.cpp`:
- `MsgFunc_SetSky()`: `m_iSkyScale` aus Nachricht lesen
- `MsgFunc_InitHUD()`: `numMirrors` auf 0 zurücksetzen

---

## 14. Client: Rendering-Verbesserungen

### 14.1 Spiegel-Rendering

**Datei:** `cl_dll/StudioModelRenderer.cpp`:
- `b_PlayerMarkerParsed` und `m_nCachedFrameCount` Member hinzufügen
- Null-Model-Rendering (models/null.mdl) für Spieler-Darstellung implementieren
- 3-Achsen Spiegel-Unterstützung (X, Y, Z) mit Matrix-Transformationen
- Sky-Modus-Prüfung: Rendering in `SKY_ON_DRAWING` überspringen wenn `renderfx != kRenderFxEntInPVS`

**Datei:** `cl_dll/StudioModelRenderer.h`:

```cpp
// Neue Member:
bool b_PlayerMarkerParsed;
int m_nCachedFrameCount;
```

### 14.2 Spieler-Blend-Korrektur

**Datei:** `cl_dll/StudioModelRenderer.cpp`:

```cpp
// ALT:
*pBlend = (*pPitch * 3);

// NEU:
*pBlend = (*pPitch * -6);
```

---

## 15. Client: Sonstige Änderungen

### 15.1 FMOD-Audio

**Datei:** `cl_dll/mp3.cpp`:
- `FSOUND_GetVersion()` Laden auskommentieren
- Funktions-Erkennung: Neuere `Stream_Open` vor älterer `Stream_OpenFile` prüfen
- Pfadbehandlung: Hardcodierten `sound/fmod/`-Prefix entfernen

### 15.2 Voice-Status

**Datei:** `game_shared/voice_status.cpp`:
- `extern int cam_thirdperson;` entfernen
- Voice-Icon-Anzeige für lokalen Spieler aktivieren (unabhängig vom Kamera-Modus):

```cpp
// ALT:
if(pClient == localPlayer && !cam_thirdperson)
    continue;

// NEU (auskommentiert):
//if(pClient == localPlayer && !gHUD.m_iCameraMode)
//    continue;
```

---

## 16. Zusammenfassung der neuen CVars und Befehle

### Server-CVars

| CVar | Standard | Beschreibung |
|------|----------|-------------|
| `max_medkit` | `200` | Maximale Ladung des tragbaren Medkits |
| `max_cameras` | `2` | Maximale Anzahl tragbarer Kameras |
| `timed_damage` | `1` | Zeitbasierte Schäden (Nervengas/Strahlung) ein/aus |

### Client-CVars

| CVar | Standard | Beschreibung |
|------|----------|-------------|
| `cl_raininfo` | `0` | Debug-Ausgabe für das Regensystem |

### Konsolenbefehle

| Befehl | Beschreibung |
|--------|-------------|
| `thirdperson` | Third-Person-Kamera aktivieren |
| `firstperson` | First-Person-Kamera aktivieren |
| `drawplayer` | Spielermodell in Third-Person anzeigen |
| `hideplayer` | Spielermodell in Third-Person verstecken |
| `+hud` / `-hud` | Custom-Menü anzeigen/verbergen |
| `+briefing` / `-briefing` | Map-Briefing anzeigen/verbergen |
| `inventory <1-10>` | Inventar-Item aktivieren |

### Netzwerknachrichten

| Nachricht | Richtung | Beschreibung |
|-----------|----------|-------------|
| `RainData` | Server → Client | Regen-Parameter (Tropfenrate, Wind, Modus, Höhe) |
| `Inventory` | Server → Client | Inventar-Item-Aktualisierung (Index + Anzahl) |
| `CamData` | Server → Client | Kamera-Steuerungsdaten (G-Cont) |

---

## Checkliste für die Migration

- [ ] Backup des SOHL 1.4 Projekts erstellen
- [ ] Versionsstring aktualisieren (`gamerules.h`)
- [ ] `MAX_ITEMS` auf 10 erhöhen (`cbase.h`)
- [ ] `killname`/`killmethod` Felder hinzufügen (`cbase.h`)
- [ ] `RADIATION_DURATION` auf 10 ändern (`cbase.h`)
- [ ] `TakeHealth()` Rückgabewert anpassen (`cbase.cpp`)
- [ ] Regen-Entities implementieren (`effects.cpp`, `effects.h`)
- [ ] Regen-Variablen im Spieler hinzufügen (`player.h`, `player.cpp`)
- [ ] Inventar-Items implementieren (`items.cpp`, `items.h`)
- [ ] Inventar-Precache in `world.cpp` einbinden
- [ ] Neue CVars registrieren (`game.cpp`)
- [ ] Custom-Kill-Nachrichten implementieren (`multiplay_gamerules.cpp`, `death.cpp`)
- [ ] Holster-Zeiten auf 0.5 reduzieren (14 Waffen-Dateien)
- [ ] Entity-Fixes anwenden (8.1–8.5)
- [ ] Entfernte Dateien löschen (`islave_deamon.cpp`, etc.)
- [ ] `rain.cpp`, `rain.h`, `CustomMenu.cpp` zum Client-Projekt hinzufügen
- [ ] Regen-Rendering in `tri.cpp` implementieren
- [ ] Kamera-System in `in_camera.cpp` und `view.cpp` aktualisieren
- [ ] HUD-Änderungen anwenden (`hud.h`, `hud.cpp`, `hud_msg.cpp`, `hud_redraw.cpp`)
- [ ] Spiegel-Rendering-Updates in `StudioModelRenderer.cpp` übernehmen
- [ ] VGUI-Integration für Inventar und Custom-Menu
- [ ] FMOD- und Voice-Status-Änderungen anwenden
- [ ] Projekt-Dateien (.dsp) aktualisieren
- [ ] Kompilieren und testen
