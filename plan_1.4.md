# SOHL 1.4 Migrationsplan – Bugfixes & Partikelsystem

> Dieses Dokument fasst alle relevanten Änderungen aus Spirit of Half-Life 1.4 zusammen,
> die auf ein bestehendes Projekt übertragen werden müssen, das bereits mit SOHL 1.2 arbeitet.
> Quelle: `changelog.md` (Commit 3a7d32a und nachfolgende Fixes).

---

## Inhaltsübersicht

1. [Partikelsystem-Fixes](#1-partikelsystem-fixes)
2. [Kritische Bugfixes (Server-DLL)](#2-kritische-bugfixes-server-dll)
3. [Kritische Bugfixes (Client-DLL)](#3-kritische-bugfixes-client-dll)
4. [Buffer-Overflow-Behebungen](#4-buffer-overflow-behebungen)
5. [NULL-Pointer-Behebungen](#5-null-pointer-behebungen)
6. [Migrations-Checkliste](#6-migrations-checkliste)

---

## 1. Partikelsystem-Fixes

Diese Fixes betreffen das Aurora-Partikelsystem im Client-DLL und sind die wichtigsten
Änderungen für Projekte, die Partikeleffekte nutzen.

### 1.1 Division durch Null in `particlesys.cpp`

| Eigenschaft | Detail |
|---|---|
| **Datei** | `SpiritSource/cl_dll/particlesys.cpp` |
| **Zeilen** | 53, 78, 752 |
| **Schwere** | Kritisch – Absturz des Spiels |

**Problem:** Drei Stellen verwenden `1/wert` ohne Null-Prüfung:
- Zeile 53: `1/pPart->age_death` – Partikel-Lebenszeit kann 0 sein (immortal particle)
- Zeile 78: `1/m_SprayRate.GetInstance()` – Spray-Rate kann 0 zurückgeben
- Zeile 752: `1/part->pType->m_SprayRate.GetInstance()` – gleicher Fall im Update-Loop

**Fix-Anleitung:**

```cpp
// Zeile 53 – InitParticle(): age_death-Prüfung
// ALT:
float fLifeRecip = 1/pPart->age_death;
// NEU:
float fLifeRecip = (pPart->age_death > 0.0001f) ? 1/pPart->age_death : 1.0f;

// Zeile 78 – InitParticle(): SprayRate-Prüfung
// ALT:
pPart->age_spray = 1/m_SprayRate.GetInstance();
// NEU:
float fSprayRate = m_SprayRate.GetInstance();
pPart->age_spray = (fSprayRate > 0.0001f) ? 1/fSprayRate : 0.0f;

// Zeile 752 – UpdateSystem(): SprayRate-Prüfung im Update-Loop
// ALT:
part->age_spray = part->age + 1/part->pType->m_SprayRate.GetInstance();
// NEU:
float fSprayRate = part->pType->m_SprayRate.GetInstance();
part->age_spray = part->age + ((fSprayRate > 0.0001f) ? 1/fSprayRate : 0.0f);
```

> **Hinweis:** Laut Aurora-Referenz bedeutet eine Lebenszeit von 0 oder undefiniert,
> dass das Partikel unsterblich (immortal) ist. Der Fallback-Wert `1.0f` für `fLifeRecip`
> verhindert den Absturz und liefert ein sinnvolles Standardverhalten.

### 1.2 Division durch Null beim Wasser-Rendering in `tri.cpp`

| Eigenschaft | Detail |
|---|---|
| **Datei** | `SpiritSource/cl_dll/tri.cpp` |
| **Zeile** | 106 |
| **Schwere** | Kritisch – Absturz bei Wasseroberflächen |

**Problem:** `1/(m_fScale*fHeight)` – wenn `m_fScale` oder `fHeight` 0 ist, kommt es zur Division durch Null.

**Fix-Anleitung:**

```cpp
// ALT:
float fFactor = 1/(m_fScale*fHeight);
// NEU:
float fDenom = m_fScale*fHeight;
if (fDenom > -0.0001f && fDenom < 0.0001f) return;
float fFactor = 1/fDenom;
```

---

## 2. Kritische Bugfixes (Server-DLL)

### 2.1 Memory Corruption: `delete` statt `delete[]` für Arrays

| Eigenschaft | Detail |
|---|---|
| **Datei** | `SpiritSource/dlls/nodes.cpp` |
| **Zeilen** | 3325–3328, 3461–3462 |
| **Schwere** | Kritisch – Heap-Korruption, undefiniertes Verhalten |

**Problem:** Arrays alloziert mit `new[]` werden mit `delete` statt `delete[]` freigegeben.
Betrifft: `Routes`, `BestNextNodes`, `pRoute`, `pMyPath`, `pMyPath2`.

**Fix-Anleitung:**

```cpp
// ALT:
delete Routes;
delete BestNextNodes;
delete pRoute;
// NEU:
delete[] Routes;
delete[] BestNextNodes;
delete[] pRoute;
```

### 2.2 Operator-Vorrang-Fehler bei Tür-Synchronisation

| Eigenschaft | Detail |
|---|---|
| **Datei** | `SpiritSource/dlls/doors.cpp` |
| **Zeile** | 1012–1013 |
| **Schwere** | Kritisch – Fehlerhafte Tür-Logik |

**Problem:** Fehlende Klammern bei `&&`/`||`-Verknüpfung. `func_door_rotating`-Entities konnten
fälschlicherweise mit sich selbst synchronisieren.

**Fix-Anleitung:**

```cpp
// ALT:
VARS(pTarget->pev) != pev && FClassnameIs(pTarget->pev, "func_door") || FClassnameIs(pTarget->pev, "func_door_rotating")
// NEU:
VARS(pTarget->pev) != pev && (FClassnameIs(pTarget->pev, "func_door") || FClassnameIs(pTarget->pev, "func_door_rotating"))
```

### 2.3 Division durch Null bei Spieler-Gesundheitsanzeige

| Eigenschaft | Detail |
|---|---|
| **Datei** | `SpiritSource/dlls/player.cpp` |
| **Zeile** | 1889 |
| **Schwere** | Kritisch – Absturz wenn max_health 0 ist |

**Fix-Anleitung:**

```cpp
// ALT:
pEntity->pev->health / pEntity->pev->max_health
// NEU:
(pEntity->pev->max_health > 0) ? (pEntity->pev->health / pEntity->pev->max_health) : 0
```

---

## 3. Kritische Bugfixes (Client-DLL)

Die Partikelsystem-Fixes (Abschnitt 1) sind die einzigen Client-DLL-Fixes neben dem
Wasser-Rendering-Fix (tri.cpp). Beide sind in Abschnitt 1 beschrieben.

---

## 4. Buffer-Overflow-Behebungen

### 4.1 NPC-Sprachausgabe (barney.cpp)

| Eigenschaft | Detail |
|---|---|
| **Datei** | `SpiritSource/dlls/barney.cpp` |
| **Zeilen** | 280–283, 594–597, 612–615, 628–631, 799–802 |

**Fix:** Buffer von 32 auf 64 Bytes vergrößern; `strcpy`/`strcat` durch `snprintf` ersetzen.

```cpp
// ALT:
char szBuf[32];
strcpy(szBuf, STRING(m_iszSpeakAs));
strcat(szBuf, "_ATTACK");
// NEU:
char szBuf[64];
snprintf(szBuf, sizeof(szBuf), "%s_ATTACK", STRING(m_iszSpeakAs));
```

### 4.2 Map-Name zu lang (triggers.cpp)

| Eigenschaft | Detail |
|---|---|
| **Datei** | `SpiritSource/dlls/triggers.cpp` |
| **Zeilen** | 3500–3503, 3514–3516 |

**Fix:** `return` nach der Fehlermeldung „Map name too long", bevor `strcpy` aufgerufen wird.

### 4.3 Audio-Befehl (triggers.cpp)

| Eigenschaft | Detail |
|---|---|
| **Datei** | `SpiritSource/dlls/triggers.cpp` |
| **Zeile** | 2819 |

**Fix:** `sprintf` durch `snprintf(command, sizeof(command), ...)` ersetzen.

### 4.4 Satzverarbeitung (sound.cpp)

| Eigenschaft | Detail |
|---|---|
| **Datei** | `SpiritSource/dlls/sound.cpp` |
| **Zeilen** | 1490, 1503 |

**Fix:**
1. Off-by-one: `>` durch `>=` ersetzen bei `gcallsentences >= CVOXFILESENTENCEMAX`
2. `strcpy` durch `strncpy` mit Null-Terminierung ersetzen.

### 4.5 Client-Nachrichten (client.cpp)

| Eigenschaft | Detail |
|---|---|
| **Datei** | `SpiritSource/dlls/client.cpp` |
| **Zeilen** | 110, 254, 259, 559 |

**Fix:** Alle `sprintf` durch `snprintf(buf, sizeof(buf), ...)` ersetzen.

---

## 5. NULL-Pointer-Behebungen

### 5.1 Scripted Sequences (AI_BaseNPC_Schedule.cpp)

| Eigenschaft | Detail |
|---|---|
| **Datei** | `SpiritSource/dlls/AI_BaseNPC_Schedule.cpp` |
| **Zeilen** | 526, 538, 1289, 1293 |

**Fix:** `m_pCine &&` Prüfung vor jeder Dereferenzierung von `m_pCine` hinzufügen.

### 5.2 Grunt-Angriffsprüfung (hgrunt.cpp)

| Eigenschaft | Detail |
|---|---|
| **Datei** | `SpiritSource/dlls/hgrunt.cpp` |
| **Zeile** | 453 |

**Fix:** `m_hEnemy &&` Prüfung vor `m_hEnemy->IsPlayer()` hinzufügen.

### 5.3 Barney-Schaden (barney.cpp)

| Eigenschaft | Detail |
|---|---|
| **Datei** | `SpiritSource/dlls/barney.cpp` |
| **Zeile** | 624 |

**Fix:** `m_hEnemy &&` Prüfung vor `m_hEnemy->IsPlayer()` hinzufügen.

---

## 6. Migrations-Checkliste

### Vorbereitung

- [ ] Backup des bestehenden SOHL 1.2 Projekts erstellen
- [ ] Versionskontrolle (Git-Branch) für die Migration anlegen
- [ ] SOHL 1.4 Quellcode als Referenz bereithalten

### Partikelsystem (Priorität: Hoch)

- [ ] `cl_dll/particlesys.cpp` Zeile 53: Division-durch-Null-Fix für `age_death`
- [ ] `cl_dll/particlesys.cpp` Zeile 78: Division-durch-Null-Fix für `m_SprayRate` in `InitParticle`
- [ ] `cl_dll/particlesys.cpp` Zeile 752: Division-durch-Null-Fix für `m_SprayRate` im Update-Loop
- [ ] `cl_dll/tri.cpp` Zeile 106: Division-durch-Null-Fix für Wasser-Rendering
- [ ] Partikeleffekte testen: `particletest.bsp` laden und alle Aurora-Effekte prüfen (Map liegt im SOHL 1.4 Paket unter `Spirit/maps/`)

### Kritische Speicherfehler (Priorität: Hoch)

- [ ] `dlls/nodes.cpp` Zeilen 3325–3328, 3461–3462: `delete` → `delete[]` für alle Arrays
- [ ] `dlls/doors.cpp` Zeile 1012–1013: Operator-Vorrang-Fix mit Klammerung
- [ ] `dlls/player.cpp` Zeile 1889: Division-durch-Null-Schutz für `max_health`

### Buffer Overflows (Priorität: Mittel)

- [ ] `dlls/barney.cpp` (5 Stellen): Buffer vergrößern + `snprintf` verwenden
- [ ] `dlls/triggers.cpp` Zeile 3500–3516: `return` nach „Map name too long"
- [ ] `dlls/triggers.cpp` Zeile 2819: `sprintf` → `snprintf`
- [ ] `dlls/sound.cpp` Zeilen 1490, 1503: Off-by-one + `strncpy`
- [ ] `dlls/client.cpp` (4 Stellen): `sprintf` → `snprintf`

### NULL-Pointer-Absicherung (Priorität: Mittel)

- [ ] `dlls/AI_BaseNPC_Schedule.cpp` (4 Stellen): `m_pCine` NULL-Prüfung
- [ ] `dlls/hgrunt.cpp` Zeile 453: `m_hEnemy` NULL-Prüfung
- [ ] `dlls/barney.cpp` Zeile 624: `m_hEnemy` NULL-Prüfung

### Abschluss

- [ ] Vollständigen Compile-Test (Client-DLL + Server-DLL) durchführen
- [ ] Testmap mit Partikeleffekten laden und visuell prüfen
- [ ] Türen-Logik in einer Map mit `func_door_rotating` testen
- [ ] NPC-Interaktion (Barney, Grunt) mit Sprachausgabe testen
- [ ] Changelevel mit langem Map-Namen testen
- [ ] Spieler-Gesundheitsanzeige bei NPCs mit `max_health = 0` prüfen
- [ ] Code-Review durch zweiten Entwickler
- [ ] Änderungen in Versionskontrolle committen
