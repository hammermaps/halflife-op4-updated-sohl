# Agent Instructions: Hybrid AI Core for Half-Life: Opposing Force Updated

## Ziel

Diese Datei beschreibt eine **Hybrid-Lösung zur Modernisierung der NPC-KI** für:

- Repository: `twhl-community/halflife-op4-updated`
- Engine: GoldSrc / Half-Life 1 SDK
- Zielbereich: `dlls/`
- Fokus: Gegner- und Friendly-AI
- Architektur: neues taktisches Entscheidungsmodell auf bestehender HLSDK-Infrastruktur

Die Hybrid-Lösung ersetzt **nicht** das komplette Monster-/NPC-System.

Stattdessen wird die bestehende GoldSrc-Infrastruktur als Ausführungs- und Integrationsschicht beibehalten:

```text
Navigation
Nodegraph
Schedules
Tasks
Animationen
Movement
scripted_sequence
Save/Restore
Relationships
EHANDLE
Entity lifecycle
```

Ersetzt bzw. überlagert wird primär:

```text
taktische Entscheidungslogik
Enemy Memory
Enemy Lost Behavior
Cover-Bewertung
Squad-Koordination
Flanking
Suppression
Morale
Personality
Friendly Formation
Move-Aside / Block Avoidance
```

Das Ziel ist:

> Neues KI-Gehirn auf bestehendem GoldSrc-Körper.

---

# 1. Grundarchitektur

Die Zielarchitektur lautet:

```text
                GoldSrc / HLSDK
                     |
        +------------+-------------+
        |                          |
        v                          v
 Existing perception         Existing navigation
 conditions/sounds           nodes/routes/movement
        |                          |
        +------------+-------------+
                     |
                     v
              Enhanced AI Core
                     |
        +------------+-------------+
        |            |             |
        v            v             v
     Memory       Utility       Squad Data
                   AI
        |            |             |
        +------------+-------------+
                     |
                     v
             Tactical Decision
                     |
      +--------------+--------------+
      |              |              |
      v              v              v
   Attack          Cover          Search
      |              |              |
      +---------+----+------+-------+
                |           |
                v           v
              Flank     Suppression
                |
                v
         Existing Schedules
                |
                v
           Existing Tasks
                |
                v
     Existing GoldSrc Movement
```

---

# 2. Was bleibt bestehen

Die folgenden Systeme sollen grundsätzlich erhalten bleiben:

```text
CBaseMonster
CSquadMonster
MonsterThink()
RunAI()
GetScheduleOfType()
Schedule_t
Task_t
StartTask()
RunTask()
BuildRoute()
CheckLocalMove()
MovementComplete()
MoveToLocation()
Nodegraph
soundent
Relationship system
scripted_sequence
save/restore
animations
activity system
monster state system
EHANDLE
```

Der Agent darf bestehende Funktionen erweitern, aber nicht ohne Not ersetzen.

---

# 3. Was ersetzt oder überlagert wird

Der Hybrid-Core darf große Teile der aktuellen taktischen Auswahl überlagern:

```text
GetSchedule()-Entscheidungsbäume
Enemy-lost Entscheidungen
Cover-Auswahl
Squad-Rollen
Angriffspriorisierung
Flankenwahl
Rückzugsentscheidungen
Friendly Follow Positioning
Blocking Reactions
```

Wichtig:

`GetSchedule()` bleibt technisch erhalten.

Es wird aber zunehmend zum Adapter zwischen:

```text
Enhanced AI Decision
```

und:

```text
bestehenden HLSDK-Schedules
```

Beispiel:

```cpp
Schedule_t* CHGrunt::GetSchedule()
{
    if (ShouldUseEnhancedAI())
    {
        const AIDecision decision = m_AI.SelectDecision();

        if (auto schedule = ResolveEnhancedDecision(decision))
            return schedule;
    }

    return GetOriginalSchedule();
}
```

Die originale Logik muss als Fallback erhalten bleiben.

---

# 4. Kernprinzip: Utility AI

Die taktische Entscheidung soll bevorzugt als **Utility AI** umgesetzt werden.

Nicht primär:

```text
if enemy visible -> attack
else if hurt -> cover
else ...
```

Sondern:

```text
jede mögliche Aktion bekommt einen Score
höchster gültiger Score gewinnt
```

Beispiel:

```text
ATTACK       54
COVER        71
FLANK        83
SUPPRESS     42
SEARCH        0
RETREAT      18
```

Gewählt wird:

```text
FLANK
```

---

# 5. Tactical Actions

Definiere zunächst eine begrenzte Menge universeller Aktionen:

```cpp
enum AITacticalAction
{
    AI_ACTION_NONE = 0,

    AI_ACTION_IDLE,
    AI_ACTION_ALERT,
    AI_ACTION_ATTACK,
    AI_ACTION_ADVANCE,
    AI_ACTION_COVER,
    AI_ACTION_SEARCH,
    AI_ACTION_INVESTIGATE,
    AI_ACTION_SUPPRESS,
    AI_ACTION_FLANK_LEFT,
    AI_ACTION_FLANK_RIGHT,
    AI_ACTION_RETREAT,
    AI_ACTION_REPOSITION,
    AI_ACTION_FOLLOW,
    AI_ACTION_MOVE_ASIDE
};
```

Nicht jede NPC-Klasse darf jede Aktion nutzen.

Dafür Capability Flags verwenden.

---

# 6. Capability-System

Beispiel:

```cpp
enum AIEnhancedCapabilities
{
    AI_CAP_MEMORY         = 1 << 0,
    AI_CAP_SEARCH         = 1 << 1,
    AI_CAP_COVER          = 1 << 2,
    AI_CAP_SQUAD          = 1 << 3,
    AI_CAP_FLANK          = 1 << 4,
    AI_CAP_SUPPRESSION    = 1 << 5,
    AI_CAP_RETREAT        = 1 << 6,
    AI_CAP_FORMATION      = 1 << 7,
    AI_CAP_MOVE_ASIDE     = 1 << 8,
    AI_CAP_LOCAL_AVOID    = 1 << 9,
    AI_CAP_MORALE         = 1 << 10,
    AI_CAP_PERSONALITY    = 1 << 11
};
```

Der konkrete Bit-Typ muss zum Repository-Stil passen.

---

# 7. AI Core Datenmodell

Empfohlene zentrale Struktur:

```cpp
struct AIState
{
    AIEnemyMemory enemyMemory;
    AIProfile profile;

    AITacticalAction currentAction;
    AITacticalAction previousAction;

    float morale;

    float nextDecisionTime;
    float actionStartTime;

    Vector tacticalTarget;
    Vector moveTarget;

    int squadRole;
};
```

Nicht alles muss zwingend in einer Struktur liegen.

Das konkrete Layout soll sich am bestehenden HLSDK-Stil orientieren.

---

# 8. Enemy Memory

Die neue AI darf nicht omniscient sein.

Benötigte Informationen:

```cpp
struct AIEnemyMemory
{
    EHANDLE enemy;

    Vector lastSeenPosition;
    Vector lastHeardPosition;
    Vector lastSharedPosition;

    float lastSeenTime;
    float lastHeardTime;
    float lastSharedTime;

    float confidence;
};
```

## Wahrnehmungsquellen unterscheiden

Die AI muss unterscheiden:

```text
DIRECT_SIGHT
HEARD
SQUAD_SHARED
INFERRED
```

Direkte Sicht ist die stärkste Quelle.

Squad-Information ist niemals identisch mit direkter Sicht.

---

# 9. Confidence-Modell

Beispiel:

```text
direct sight:
    confidence = 1.0

recent squad report:
    confidence = max(confidence, 0.7)

recent combat sound:
    confidence = max(confidence, 0.5)

old information:
    decay over time
```

Beispielhafte Schwellen:

```text
>= 0.75 confirmed
>= 0.45 probable
>= 0.20 uncertain
<  0.20 weak
<= 0    lost
```

Diese Werte müssen als Konstanten definierbar sein.

---

# 10. Utility Context

Jede Entscheidung arbeitet mit einem kompakten Kontext:

```cpp
struct AIUtilityContext
{
    bool enemyVisible;
    bool enemyRecentlySeen;
    bool enemyKnown;

    float enemyDistance;
    float healthRatio;
    float morale;
    float aggression;
    float caution;
    float teamwork;

    bool hasCover;
    bool flankAvailable;
    bool suppressionPossible;

    bool grenadeDanger;
    bool underHeavyFire;

    int nearbyFriends;
    int nearbyEnemies;
};
```

Nicht jede Abfrage muss jedes Mal neu berechnet werden.

Teure Daten zwischenspeichern.

---

# 11. Utility Scores

Jede mögliche Aktion implementiert:

```cpp
float ScoreAttack(const AIUtilityContext& ctx);
float ScoreCover(const AIUtilityContext& ctx);
float ScoreSearch(const AIUtilityContext& ctx);
float ScoreSuppress(const AIUtilityContext& ctx);
float ScoreFlank(const AIUtilityContext& ctx);
float ScoreRetreat(const AIUtilityContext& ctx);
```

## Beispiel Attack

```cpp
score =
    visibility * 40 +
    aggression * 30 +
    morale * 15 +
    weaponSuitability * 20 -
    danger * 35 -
    lowHealthPenalty * 25;
```

## Beispiel Cover

```cpp
score =
    danger * 40 +
    caution * 25 +
    lowHealth * 30 -
    morale * 10;
```

## Beispiel Flank

```cpp
score =
    teamwork * 25 +
    aggression * 20 +
    flankQuality * 40 +
    squadSupport * 20 -
    danger * 20 -
    routeCost * 15;
```

## Beispiel Search

```cpp
score =
    enemyMemoryConfidence * 40 +
    recentContact * 30 +
    aggression * 10 -
    memoryAgePenalty * 30;
```

---

# 12. Score-Normalisierung

Scores möglichst in konsistentem Bereich halten:

```text
0 - 100
```

Negative interne Werte sind erlaubt.

Am Ende clampen:

```cpp
score = std::clamp(score, 0.0f, 100.0f);
```

Nur verwenden, wenn `std::clamp` im Projektstandard verfügbar ist.

Ansonsten lokale Clamp-Funktion nutzen.

---

# 13. Hysterese

Ohne Hysterese wechselt die Utility AI zu oft die Aktion.

Beispiel:

```text
ATTACK 60
COVER  61
ATTACK 62
COVER  63
```

Ergebnis:

```text
ständiges Umschalten
```

Deshalb:

```cpp
if (newScore < currentScore + SWITCH_THRESHOLD)
    keep current action;
```

Zusätzlich:

```text
minimum action duration
cooldown
commit window
```

Beispiel:

```text
min action duration: 0.5 - 2.0 s
```

Je nach Aktion.

---

# 14. Decision Tick

Die AI darf nicht jedes Frame eine vollständige Utility-Bewertung ausführen.

Beispiel:

```cpp
if (gpGlobals->time >= m_flNextAIDecision)
{
    UpdateEnhancedDecision();
    m_flNextAIDecision =
        gpGlobals->time + GetDecisionInterval();
}
```

Empfehlung:

```text
Combat: 0.15 - 0.35 s
Alert:  0.3  - 0.6 s
Idle:   0.5  - 1.0 s
```

Events dürfen sofortige Re-Evaluation triggern:

```text
new enemy
grenade danger
heavy damage
enemy died
squad leader died
scripted state ended
```

---

# 15. Schedule Adapter

Die Utility AI führt keine Animationen oder Navigation direkt aus.

Sie liefert:

```text
Intent
Target
Optional tactical data
```

Dann erfolgt Mapping auf vorhandene Schedules.

Beispiel:

```cpp
Schedule_t* ResolveEnhancedDecision(const AIDecision& decision)
{
    switch (decision.action)
    {
    case AI_ACTION_ATTACK:
        return GetScheduleOfType(SCHED_RANGE_ATTACK1);

    case AI_ACTION_COVER:
        return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);

    case AI_ACTION_SEARCH:
        return GetScheduleOfType(SCHED_ENHANCED_SEARCH);

    case AI_ACTION_FLANK_LEFT:
    case AI_ACTION_FLANK_RIGHT:
        return GetScheduleOfType(SCHED_ENHANCED_FLANK);

    default:
        return nullptr;
    }
}
```

---

# 16. Neue Schedules nur wo nötig

Vor jeder neuen Schedule prüfen:

```text
Kann vorhandene Schedule verwendet werden?
Kann vorhandene Task-Kombination erweitert werden?
```

Neue Schedules nur für wirklich neue Verhaltensmuster.

Empfohlen:

```text
SCHED_ENHANCED_SEARCH
SCHED_ENHANCED_FLANK
SCHED_ENHANCED_SUPPRESS
SCHED_ENHANCED_REPOSITION
SCHED_ENHANCED_MOVE_ASIDE
```

---

# 17. Search System

Nach Sichtverlust:

```text
enemy seen
    |
    v
enemy occluded
    |
    v
memory updated
    |
    v
SEARCH score rises
    |
    v
move to last known area
    |
    v
scan/listen
    |
    +--> reacquire -> ATTACK
    |
    +--> squad update -> investigate
    |
    +--> timeout -> ALERT
```

Search darf nicht die echte versteckte Playerposition verwenden.

---

# 18. Cover System

Vorhandene Cover-Funktionen bleiben erhalten.

Der Hybrid-Core verbessert primär die Auswahl.

Cover-Kandidaten bewerten nach:

```text
line of fire
distance
danger
route cost
attack opportunity
spacing
squad occupancy
grenades
enemy direction
```

Nicht jede Candidate-Position muss jedes Decision Tick neu gesucht werden.

Cache:

```text
candidate position
score
timestamp
enemy position snapshot
```

Cache invalidieren wenn:

```text
enemy moved significantly
candidate became blocked
grenade danger
route failed
```

---

# 19. Squad Blackboard

Squad-Kommunikation als kleine gemeinsame Wissensstruktur.

Beispiel:

```cpp
struct AISquadBlackboard
{
    EHANDLE knownEnemy;

    Vector lastKnownEnemyPosition;
    float lastEnemyUpdate;

    Vector dangerPosition;
    float dangerTime;

    EHANDLE flankLeftActor;
    EHANDLE flankRightActor;
    EHANDLE suppressActor;
};
```

Es muss geprüft werden, wie `CSquadMonster` aktuell Leader und Member verwaltet.

Blackboard bevorzugt am Squad-Leader oder in bestehender Squad-Struktur speichern.

Kein globaler Manager.

---

# 20. Squad Rollen

Dynamische Rollen:

```text
PRIMARY
SUPPRESS
FLANK_LEFT
FLANK_RIGHT
COVER
RESERVE
```

Rollen sind temporär.

Nicht dauerhaft an NPCs binden.

Beispiel:

```text
NPC verliert Flankenroute
    ->
Role wird freigegeben
```

Vorhandene Squad Slots bevorzugt wiederverwenden.

---

# 21. Flanking

Flankensuche:

```text
enemy position
attacker position
engagement vector
left/right lateral region
reachable node candidates
score candidates
```

Kandidaten bewerten:

```text
lateral angle
route length
cover
line of sight
enemy exposure
squad spacing
occupancy
```

Nicht jede Aktion benötigt exakte 90°.

Gute Bereiche:

```text
ca. 45° bis 120°
```

je nach Map.

---

# 22. Suppression

Suppression benötigt:

```text
recent credible enemy position
suitable ranged weapon
safe firing corridor
limited burst
cooldown
```

Nicht erlaubt:

```text
durch Wände exakt auf Spieler schießen
unbegrenzt auf stale LKP feuern
Friendly Fire ignorieren
```

Suppression Ziel:

```cpp
target =
    lastKnownPosition +
    randomized uncertainty;
```

Unsicherheit steigt mit:

```text
memory age
distance
source quality
```

---

# 23. Morale

Morale:

```text
0.0 - 1.0
```

Event-basiert aktualisieren.

Beispiel:

```text
squadmate dies     -0.15
leader dies        -0.25
heavy damage       -0.10
grenade danger     -0.05
enemy wounded      +0.05
enemy killed       +0.10
reinforcement      +0.10
```

Morale modifiziert Utility Scores.

Nicht direkt:

```text
if morale < 0.2 -> flee
```

Besser:

```text
low morale:
    cover score +
    retreat score +
    attack score -
    flank score -
```

Dadurch bleibt Verhalten organisch.

---

# 24. Personality

Empfohlen:

```cpp
struct AIProfile
{
    float aggression;
    float caution;
    float bravery;
    float teamwork;
    float mobility;
};
```

Beispiel:

```text
HECU:
    aggression 0.70
    caution    0.60
    bravery    0.75
    teamwork   0.90
    mobility   0.70
```

```text
Shock Trooper:
    aggression 0.90
    caution    0.30
    bravery    0.90
    teamwork   0.70
    mobility   0.90
```

```text
Friendly Grunt:
    aggression 0.60
    caution    0.60
    bravery    0.70
    teamwork   0.90
    mobility   0.60
```

```text
Scientist:
    aggression 0.05
    caution    0.95
    bravery    0.20
    teamwork   0.50
    mobility   0.30
```

---

# 25. Friendly AI: Follow System

Das klassische Follow-Verhalten soll nicht vollständig ersetzt werden.

Stattdessen Formation Target hinzufügen.

Beispiel:

```text
Follower 1 -> hinten links
Follower 2 -> hinten rechts
Follower 3 -> weiter hinten links
Follower 4 -> weiter hinten rechts
```

Formation wird relativ zu Player-Orientierung berechnet.

Formation ist nur bevorzugte Position.

Bei Problemen:

```text
slot unreachable
doorway
combat
player too far away
scripted action
```

zur normalen Follow-Logik zurückfallen.

---

# 26. Move Aside

Neue Utility Action:

```text
AI_ACTION_MOVE_ASIDE
```

Score steigt wenn:

```text
player close
player looking/moving toward NPC
NPC blocks forward path
valid escape position exists
NPC not busy with critical action
```

Move-Aside hat für Friendly NPCs kurzfristig hohe Priorität.

Aber nicht über:

```text
death
script
critical healing
special interaction
immediate grenade danger
```

---

# 27. Local Avoidance

Local Avoidance modifiziert nur kurzfristige Bewegung.

Nicht Navigation ersetzen.

Konzept:

```cpp
desired = routeDirection;
avoidance = ComputeNearbyActorAvoidance();

candidate = desired + avoidance * weight;
```

Danach:

```text
CheckLocalMove
collision validation
route preservation
```

Bei Fehlschlag:

```text
original movement
```

---

# 28. Replanning

Replanning nur bei relevanten Änderungen:

```text
enemy changed
enemy moved significantly
cover invalid
flank invalid
danger event
squad role changed
action timeout
movement failure
```

Nicht bei jedem Frame.

---

# 29. Script Priority

Map-Skripte haben Vorrang.

Enhanced AI darf nicht stören bei:

```text
scripted_sequence
forced animation
scripted movement
mission events
medic scripted action
torch scripted action
use sequence
cinematics
```

Wenn NPC in Script-State:

```text
Utility AI suspend
```

Nach Script-Ende:

```text
memory revalidate
decision refresh
```

---

# 30. State Ownership

Es muss klar sein, wer was kontrolliert.

## Enhanced AI Core

Kontrolliert:

```text
was der NPC taktisch tun möchte
```

## Schedule System

Kontrolliert:

```text
welche Sequenz von Tasks das umsetzt
```

## Task System

Kontrolliert:

```text
konkrete Aktion
```

## Navigation

Kontrolliert:

```text
wie Position erreicht wird
```

Keine Ebene darf unnötig Verantwortung der unteren Ebene duplizieren.

---

# 31. NPC Adapter

Nicht sofort alle NPC-Klassen umbauen.

Zuerst:

```text
CHGrunt
```

Dann:

```text
CHFGrunt
CShockTrooper
```

Danach:

```text
friendly grunt classes
```

Erst danach:

```text
aliens
scientists
Barney-like NPCs
```

---

# 32. CHGrunt als Referenzimplementierung

CHGrunt ist die erste vollständige Implementierung.

Zielverhalten:

```text
player entdeckt
    ->
attack / cover utility
    ->
Squad Information
    ->
player verschwindet
    ->
suppression oder search
    ->
optional flank
    ->
reacquire oder give up
```

Die CHGrunt-Implementierung dient danach als Referenz für weitere taktische NPCs.

---

# 33. Klassenschnittstellen

Mögliche Hooks:

```cpp
virtual bool UsesEnhancedAI() const;
virtual int EnhancedAICapabilities() const;
virtual AIProfile GetDefaultAIProfile() const;

virtual float ModifyUtilityScore(
    AITacticalAction action,
    float score,
    const AIUtilityContext& context);

virtual Schedule_t* ResolveEnhancedAction(
    const AIDecision& decision);
```

Nicht zwingend exakt diese Signaturen verwenden.

Repository-Stil prüfen.

---

# 34. Original AI Fallback

Jede konkrete Klasse muss auf Originalverhalten zurückfallen können.

Beispiel:

```cpp
Schedule_t* CHGrunt::GetSchedule()
{
    if (!EnhancedAIEnabled())
        return GetOriginalGruntSchedule();

    auto result = SelectEnhancedSchedule();

    if (result)
        return result;

    return GetOriginalGruntSchedule();
}
```

Die Original-Logik darf nicht gelöscht werden.

---

# 35. CVARs

Master:

```text
ai_hybrid 0/1
```

Optional Debug:

```text
ai_hybrid_debug 0-3
```

Optional Feature Gates:

```text
ai_hybrid_memory
ai_hybrid_utility
ai_hybrid_squad
ai_hybrid_cover
ai_hybrid_flank
ai_hybrid_suppress
ai_hybrid_friendly
ai_hybrid_morale
```

Feature-CVARs primär während Entwicklung.

Später ggf. reduzieren.

---

# 36. Debug Output

Bei:

```text
ai_hybrid_debug 1
```

anzeigen:

```text
NPC
current action
previous action
current score
enemy memory confidence
morale
```

Bei Level 2:

```text
all candidate action scores
cover/flank target
squad role
```

Bei Level 3:

```text
candidate rejection reasons
route failure
trace failure
utility modifiers
```

Kein Spam bei Debug 0.

---

# 37. Beispiel Debug

```text
[AI] monster_human_grunt #31
ATTACK     46
COVER      58
FLANK_L    81
FLANK_R    43
SUPPRESS   34
SEARCH      0

Selected: FLANK_L
Reason:
  teamwork +18
  flank quality +31
  cover +22
  danger -7
```

---

# 38. Save/Restore

Neue persistente Felder:

```text
enemy memory
current action
previous action
morale
personality profile if runtime-modified
decision timers
search timeout
selected tactical target
```

Ephemeral:

```text
temporary candidate arrays
current utility score table
trace results
cache scratch data
```

Ephemeral Daten nicht speichern.

Nach Load:

```text
validate handles
validate tactical target
force decision refresh
```

---

# 39. Performance

GoldSrc-Performance hat Priorität.

Regeln:

```text
keine globale Node-Suche pro Think
keine Entity-Vollsuche pro NPC pro Frame
keine 20+ Traces pro Decision ohne Limit
keine Heap-Allokationen in häufigen AI-Pfaden
```

Kandidaten begrenzen.

Beispiel:

```text
Cover nodes max:       16
Flank nodes max:       12
Nearby actors max:      8
Decision rate:        ~4 Hz
```

Nur Richtwerte.

Messen und tunen.

---

# 40. Determinismus

Utility AI darf leichte Variation enthalten.

Aber:

```text
keine chaotischen Entscheidungen
kein ständiges Random-Rerolling
```

Randomness wenn nötig:

```text
kleiner Score-Jitter
per-action cooldown
seed abhängig von NPC
```

Beispiel:

```text
+/- 3 Utility Punkte
```

Nicht:

```text
+/- 50
```

---

# 41. Anti-Thrashing

Für jede Action:

```text
minimum duration
cooldown
switch threshold
failure cooldown
```

Beispiel:

```text
FLANK:
    min duration 1.5 s
    retry cooldown 3 s after failure
```

```text
COVER:
    min duration 1.0 s
```

```text
SUPPRESS:
    short burst
    cooldown 2-5 s
```

---

# 42. Failure Schedules

Jede neue Schedule braucht Fallback.

Beispiel Flank:

```text
route invalid
    ->
SCHED_TAKE_COVER_FROM_ENEMY
```

Search:

```text
cannot reach LKP
    ->
SCHED_ALERT_STAND
```

Move Aside:

```text
no side position
    ->
normal follow
```

---

# 43. Tactical Memory Expiry

Beispiel:

```text
direct visual position:
    strong for ~2 s
    usable for search ~8-15 s

shared squad position:
    usable shorter

combat sound:
    lower confidence

very old information:
    discard
```

Nicht exakt hardcoden.

Konstanten verwenden.

---

# 44. Threat Evaluation

Optional nach funktionierender Grundarchitektur:

```cpp
struct AIThreatInfo
{
    EHANDLE entity;
    float threatScore;
};
```

Threat Score abhängig von:

```text
distance
visibility
damage received
enemy weapon
current target
relationship
recent aggression
```

Erst später implementieren.

Nicht Teil der ersten Phase.

---

# 45. Multiple Enemies

Erste Version darf weiterhin weitgehend mit:

```text
m_hEnemy
```

arbeiten.

Aber Memory-Struktur so gestalten, dass später mehrere Feinde möglich sind.

Keine komplexe Multi-Enemy Datenbank in Phase 1.

---

# 46. Build Plan

## Phase A

Framework:

```text
AI action enum
profile
memory
utility context
master CVAR
debug
```

Noch keine großen Verhaltenseffekte.

## Phase B

CHGrunt:

```text
memory
attack
cover
search
```

## Phase C

Squad:

```text
shared memory
roles
```

## Phase D

```text
flank
suppression
```

## Phase E

```text
friendly formation
move aside
local avoidance
```

## Phase F

```text
morale
personality tuning
```

## Phase G

```text
other NPC adapters
regression
performance
```

---

# 47. Phase A Acceptance

```text
project builds
master CVAR works
AI core can be created per NPC
debug reports actions
no gameplay change while disabled
save/load unaffected
```

---

# 48. Phase B Acceptance

CHGrunt:

```text
direct sight updates memory
losing sight does not equal instant knowledge loss
search occurs
search expires
attack/cover chosen through utility
fallback works
```

---

# 49. Phase C Acceptance

Squad:

```text
one grunt sees player
others receive approximate knowledge
not direct sight
roles do not duplicate unnecessarily
leader death handled
```

---

# 50. Phase D Acceptance

Flank:

```text
reachable lateral position
limited route cost
one/two squad members max
fails safely
```

Suppression:

```text
recent position only
short bursts
safe fire
stale memory rejected
```

---

# 51. Phase E Acceptance

Friendly:

```text
less doorway blocking
followers spread out
combat overrides formation
scripted behavior remains functional
NPC does not jitter
```

---

# 52. Phase F Acceptance

Personality/Morale:

```text
HECU feel coordinated
Shock Troopers feel aggressive/mobile
friendly soldiers stay useful
scientists remain cautious
morale changes preferences
missions do not break
```

---

# 53. Testmatrix

Mindestens:

```text
single grunt vs player
4-grunt squad
enemy behind cover
enemy lost around corner
grenade near squad
low-health grunt
squad leader killed
friendly follower in corridor
3 friendly followers in open room
medic following player
torch grunt scripted interaction
save during flank
save during search
save during follow
level transition
```

---

# 54. Nicht tun

Der Agent darf nicht:

```text
HLSDK komplett ersetzen
Behavior Tree Framework importieren
GOAP Framework importieren
NavMesh schreiben
externes AI SDK einbauen
LLM Runtime integrieren
Entity System umbauen
scripted_sequence entfernen
original GetSchedule Logik löschen
Nodegraph umgehen
```

---

# 55. Empfohlene Dateien

Vor Implementierung prüfen:

```text
dlls/monsters.cpp
dlls/monsters.h
dlls/schedule.h
dlls/defaultai.cpp
dlls/squadmonster.cpp
dlls/squadmonster.h
dlls/nodes.cpp
dlls/nodes.h
dlls/hgrunt.cpp
dlls/hfgrunt.cpp
dlls/shocktrooper.cpp
dlls/hgrunt_ally.cpp
dlls/hgrunt_medic.cpp
dlls/hgrunt_torch.cpp
```

Mögliche neue Dateien:

```text
dlls/ai_hybrid.h
dlls/ai_hybrid.cpp
```

Optional später:

```text
dlls/ai_utility.h
dlls/ai_utility.cpp
```

Nicht zu früh aufsplitten.

---

# 56. Coding Style

Vor Änderungen prüfen:

```text
Repository coding conventions
clang-format
Compiler targets
C++ standard
platform-specific macros
```

Keine isolierte moderne API einführen, die stilistisch nicht zum Projekt passt.

---

# 57. Agent Arbeitsweise

Vor jeder Phase:

1. aktuelle Implementierung lesen
2. relevante Methoden dokumentieren
3. Datenfluss bestimmen
4. Dateien nennen
5. Änderungen implementieren
6. kompilieren
7. Fehler beheben
8. Verhalten testen
9. Save/Restore testen
10. Bericht erstellen

---

# 58. Bericht je Phase

Der Agent soll am Ende jeder Phase angeben:

```text
Changed files
New classes/functions
New CVARs
Behavior change
Fallback path
Save/restore impact
Performance impact
Tests performed
Known issues
Next phase
```

---

# 59. Sicherheitsregel für Kompatibilität

Bei Unsicherheit:

```text
Originalverhalten bevorzugen.
```

Bei Fehler der Hybrid-AI:

```text
Fallback auf vorhandene GetSchedule()-Logik.
```

Bei ungültiger taktischer Position:

```text
keine Bewegung erzwingen.
```

Bei Script:

```text
Hybrid-AI pausieren.
```

---

# 60. Zielzustand

Wenn die Hybrid-Lösung fertig ist, soll folgendes möglich sein:

```text
HECU Squad:
    sieht Spieler
    verteilt Wissen
    nutzt Deckung
    teilt Rollen
    unterdrückt
    flankiert
    sucht nach Sichtverlust
```

```text
Friendly Squad:
    folgt Spieler
    verteilt sich
    blockiert Türen weniger
    weicht aus
    nutzt Deckung
    kommuniziert Gegnerposition
```

```text
Passive Allies:
    folgen sinnvoll
    vermeiden Blockaden
    reagieren besser auf Gefahr
```

Und gleichzeitig bleibt erhalten:

```text
GoldSrc Navigation
HLSDK Schedules
HLSDK Tasks
Animation System
Scripts
Savegames
Map Compatibility
```

---

# 61. Final Directive

Implementiere die neue AI als **hybride Utility-/Taktikschicht über dem bestehenden HLSDK-AI-Executor**.

Die Enhanced AI entscheidet:

```text
WAS möchte der NPC tun?
```

Das bestehende HLSDK entscheidet weiterhin:

```text
WIE wird diese Aktion ausgeführt?
```

Dieses Prinzip darf nicht verletzt werden.

Der wichtigste Architekturgrundsatz lautet:

> Replace tactical reasoning, not GoldSrc execution infrastructure.

Das System muss kompatibel, nachvollziehbar, performant, debugging-fähig und schrittweise integrierbar bleiben.
