# Optimierungsempfehlung / Optimization Recommendations

Dieses Dokument fasst die Analyse der Zombie-Klassenhierarchie und der allgemeinen Codequalität zusammen,
basierend auf der Prüfung von `agent.md`, `sohl.md` und den Quelldateien.

---

## 1. Behobene Fehler (Fixed Bugs)

### 1.1 Null-Pointer in `TakeDamage()` — KRITISCH
**Dateien:** `zombie.cpp`, `zombie_soldier.cpp`, `zombie_barney.cpp`
**Problem:** `pevInflictor` wurde ohne Null-Prüfung dereferenziert. Wenn ein Zombie Schaden
von einer unbekannten Quelle erhält (z.B. `trigger_hurt` ohne Inflictor), führt dies zu einem Crash.
**Lösung:** Null-Check für `pevInflictor` hinzugefügt — Knockback wird nur angewendet, wenn der
Inflictor gültig ist. Schadensskalierung (30%) wird weiterhin angewendet.

### 1.2 Duplizierte Bedingung in `IgnoreConditions()` — HOCH
**Dateien:** `zombie.cpp`, `zombie_soldier.cpp`, `zombie_barney.cpp`
**Problem:** Die Bedingung `(m_Activity == ACT_MELEE_ATTACK1) || (m_Activity == ACT_MELEE_ATTACK1)`
prüfte zweimal die gleiche Aktivität. `ACT_MELEE_ATTACK2` wurde nie geprüft, was dazu führte,
dass Zombies während des zweiten Nahkampfangriffs zusammenzuckten.
**Lösung:** Zweite Bedingung zu `ACT_MELEE_ATTACK2` korrigiert.

### 1.3 Fehlende Bereichsprüfung `m_iPose` in `CDeadZombieSoldier` — HOCH
**Datei:** `zombie_soldier.cpp`
**Problem:** `m_iPose` wurde aus einer KeyValue-Zeichenfolge geparst (`atoi`) ohne Bereichsprüfung.
Ein Wert außerhalb von 0-1 führt zu einem Array-Überlauf bei `m_szPoses[m_iPose]`.
**Lösung:** Bereichsprüfung hinzugefügt: Werte außerhalb des gültigen Bereichs werden auf 0 zurückgesetzt.

### 1.4 `Classify()` fehlte `m_iClass`-Prüfung — MITTEL
**Dateien:** `zombie_soldier.cpp`, `zombie_barney.cpp`
**Problem:** `CZombieSoldier::Classify()` und `CZombieBarney::Classify()` gaben hartkodiert
`CLASS_ALIEN_MONSTER` zurück, ohne `m_iClass` zu prüfen. Dies verhinderte die
SoHL-Allegianz-Überschreibung per Mapper-Keyvalue.
**Lösung:** Alle Zombie-Varianten verwenden nun `return m_iClass ? m_iClass : CLASS_ALIEN_MONSTER;`
über die gemeinsame Basisklasse.

### 1.5 Fehlende Custom-Model-Unterstützung — MITTEL
**Dateien:** `zombie_soldier.cpp`, `zombie_barney.cpp`
**Problem:** `CZombieSoldier` und `CZombieBarney` ignorierten `pev->model` aus dem Entity-Setup.
Mapper konnten keine benutzerdefinierten Modelle über Keyvalues zuweisen.
**Lösung:** Alle Zombie-Varianten prüfen nun `FStringNull(pev->model)` und unterstützen Custom Models.

---

## 2. Code-Einheitlichkeit (Code Consistency)

### 2.1 Klassen-Vererbungshierarchie
**Vorher:** Alle drei Zombie-Klassen (`CZombie`, `CZombieSoldier`, `CZombieBarney`) erbten
direkt von `CBaseMonster` und duplizierten ~300 Zeilen identischen Code.

**Nachher:** Neue Basisklasse `CZombieBase` (in `zombie_base.h`) enthält:
- Gemeinsame Sound-Arrays (global, nicht pro Klasse dupliziert)
- `SetYawSpeed()`, `Classify()`, `TakeDamage()`, `IgnoreConditions()`
- `PainSound()`, `AlertSound()`, `IdleSound()`, `AttackSound()`
- `CheckRangeAttack1()`, `CheckRangeAttack2()` (beide false)
- Helper: `ZombieSpawnHelper()`, `ZombiePrecacheHelper()`, `ZombieHandleAnimEvent()`
- Rein virtuelle Methoden: `GetOneSlashDamage()`, `GetBothSlashDamage()`, `GetDefaultModel()`

Jede Zombie-Variante definiert nur noch:
- `Spawn()` — setzt spezifische Gesundheit, ruft `ZombieSpawnHelper()` auf
- `Precache()` — ruft `ZombiePrecacheHelper()` auf
- `HandleAnimEvent()` — delegiert an `ZombieHandleAnimEvent()`
- Die drei `Get*`-Methoden für variantenspezifische Werte

### 2.2 Sound-Auswahl standardisiert
**Vorher:** `CZombie` verwendete `RANDOM_SOUND_ARRAY()`-Makro, während `CZombieSoldier` und
`CZombieBarney` manuelle Array-Indizierung verwendeten: `pSounds[RANDOM_LONG(0, ARRAYSIZE(pSounds) - 1)]`.
**Nachher:** Alle verwenden einheitlich das `RANDOM_SOUND_ARRAY()`-Makro.

### 2.3 Sound-Precaching standardisiert
**Vorher:** `CZombie` verwendete `PRECACHE_SOUND_ARRAY()`-Makro, während die anderen Varianten
6 manuelle For-Schleifen mit `(char*)`-Casts verwendeten.
**Nachher:** Alle verwenden einheitlich das `PRECACHE_SOUND_ARRAY()`-Makro.

### 2.4 Pitch-Berechnung vereinheitlicht
**Vorher:** `CZombie` verwendete `95 + RANDOM_LONG(0, 9)` (Bereich 95–104) für alle Sounds.
`CZombieSoldier`/`CZombieBarney` verwendeten `100 + RANDOM_LONG(-5, 5)` (Bereich 95–105)
für `IdleSound()` und `AttackSound()`.
**Nachher:** Alle Varianten verwenden einheitlich `95 + RANDOM_LONG(0, 9)`.

---

## 3. Performance-Empfehlungen

### 3.1 Reduzierte Code-Duplizierung
Durch die Basisklasse wurden ~600 Zeilen duplizierter Code entfernt, was:
- die Kompilierzeit leicht reduziert
- die Binary-Größe reduziert (gemeinsame Sound-Arrays statt 3× kopiert)
- die Cache-Effizienz verbessert (weniger Code-Footprint)

### 3.2 Inline Sound-Arrays
Die Sound-Arrays sind als `inline`-Variablen im Header deklariert. Der Compiler/Linker
konsolidiert diese zu einer einzigen Instanz pro Translation Unit (C++17 `inline` Semantik).

### 3.3 Mögliche zukünftige Optimierungen
- **Vector::Normalize() in TakeDamage():** Die Normalisierung enthält eine Quadratwurzeloperation.
  Falls Performance kritisch ist, könnte `VectorNormalizeFast()` verwendet werden.
- **Sound-Precaching:** Sound-Arrays werden bei jedem Zombie-Spawn precached. Da die Engine
  doppelte Precache-Aufrufe ignoriert, ist dies kein Problem, aber ein zentrales Precache
  (z.B. in `GameDLLInit`) wäre effizienter bei vielen Zombies.

---

## 4. Debugging-Empfehlungen

### 4.1 Konsistente Alert-Meldungen
Die `CDeadZombieSoldier`-Klasse gab `"Dead hgrunt with bad pose"` aus — dies wurde zu
`"Dead zombie soldier with bad pose"` korrigiert, um Verwechslungen mit dem HGrunt zu vermeiden.

### 4.2 Empfohlene zukünftige Debug-Hilfen
- **ALERT bei ungültigem Inflictor:** Ein `at_aiconsole`-Log wenn `pevInflictor` null ist,
  würde helfen, defekte `trigger_hurt`-Entitäten zu identifizieren.
- **Spawn-Logging:** Ein optionales `at_aiconsole`-Log in `ZombieSpawnHelper()` mit dem
  verwendeten Modellnamen würde Custom-Model-Probleme leichter diagnostizierbar machen.
- **m_iClass-Logging:** Wenn `m_iClass` gesetzt ist, könnte ein Debug-Log die Allegianz-Überschreibung
  bestätigen.

---

## 5. Empfehlungen für sinnvolle Funktionen

### 5.1 Neue Zombie-Varianten einfach hinzufügen
Mit der `CZombieBase`-Klasse kann eine neue Zombie-Variante in ~20 Zeilen erstellt werden:

```cpp
class CZombieNew : public CZombieBase
{
public:
    void Spawn() override;
    void Precache() override;
    void HandleAnimEvent(MonsterEvent_t* pEvent) override;
protected:
    float GetOneSlashDamage() override { return gSkillData.newDmgOneSlash; }
    float GetBothSlashDamage() override { return gSkillData.newDmgBothSlash; }
    const char* GetDefaultModel() override { return "models/zombie_new.mdl"; }
};
```

### 5.2 Mögliche Erweiterungen
- **Custom Sounds per Variante:** Die Basisklasse könnte virtuelle Methoden für Sound-Arrays
  bereitstellen, falls zukünftige Zombie-Varianten eigene Sounds verwenden sollen.
- **Konfigurierbare Bullet-Resistenz:** Der 0.3-Multiplikator könnte eine virtuelle Methode
  `GetBulletDamageMultiplier()` werden, um variantenspezifische Resistenz zu ermöglichen.
- **Konfigurierbare Knockback-Stärke:** Die Angriffsgeschwindigkeit (±100) und Punch-Angles
  könnten per virtuelle Methoden anpassbar gemacht werden.

---

## 6. Weitere Code-Qualitätsbeobachtungen

### 6.1 gonome.cpp — rendercolor-Bug
In `COFGonomeGuts::Spawn()` wird `pev->rendercolor.x` dreimal hintereinander zugewiesen
(statt `.x`, `.y`, `.z`). Dieser Bug existiert sowohl im deutschen als auch im normalen Codepfad.
Dies ist ein vorbestehender Bug und nicht Teil dieser Änderungen.

### 6.2 Anim-Event-Defines
Die Anim-Event-Defines (`ZOMBIE_AE_ATTACK_RIGHT/LEFT/BOTH`) hatten in allen drei Dateien
und in `gonome.cpp` identische Werte (0x01, 0x02, 0x03). Die Defines sind nun in `zombie_base.h`
zentralisiert und werden nicht mehr pro Datei dupliziert.
