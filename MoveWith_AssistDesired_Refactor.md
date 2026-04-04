# MoveWith Assist/Desired Refactor — Performance, Determinismus, Safety

Repo-Kontext: `dlls/movewith.cpp`, `dlls/movewith.h`, `dlls/cbase.*` (CBaseEntity), `dlls/world.*` (CWorld).
Ziel: Assist/Desired-Mechanik so umbauen, dass
- **Performance**: kein Full-Scan über eine wachsende Liste pro Frame nötig ist
- **Determinismus**: definierte Abarbeitungsreihenfolge (FIFO), keine “LIFO/zufällige” Reihenfolge
- **Safety**: keine Dangling-Pointer bei Entity-Removal, keine Reentrancy-Endlosschleifen

---

## 0) Problemzusammenfassung (Ist-Zustand)

Aktuell:
- Entities werden über `CWorld::World->m_pAssistLink` in eine **einfach verkettete Liste** eingehängt.
- `CheckAssistList()` iteriert diese Liste **vollständig** und setzt ggf. Post-Assist Vel/AVel (Flags).
- Danach ruft es `CheckDesiredList()` auf, welches **wieder** die gleiche Liste scannt und bei `LF_DODESIRED` `DesiredAction()` / `Think()` ausführt.
- `UTIL_DesiredAction()` / `UTIL_DesiredThink()` hängen Entities vorne ein (LIFO) → Reihenfolge nicht deterministisch.

Risiken:
- Liste kann “ewig wachsen” → O(n) pro Frame, n steigt im Levelverlauf
- LIFO-Einfügen → Starvation möglich
- Removal/Free eines Entities kann die Liste korrumpieren, falls nicht sauber unlinked
- Reentrancy: während Desired-Ausführung neu schedulen kann zu “runaway” führen

---

## 1) Ziel-Design (Soll-Zustand)

Ersetze die “scan everything” Logik durch **zwei explizite FIFO Queues**:

1) **PostAssist Queue**  
   Enthält Entities, die *in diesem Frame* Post-Assist Velocity / AVelocity angewendet bekommen müssen.

2) **Desired Queue** (deferred calls)  
   Enthält Entities, die `DesiredAction()` und/oder `Think()` deferred ausführen müssen.

Wichtige Eigenschaften:
- **FIFO**: Reihenfolge entspricht Scheduling-Reihenfolge → deterministisch
- **Queued-once** pro Entity pro Queue: keine Duplikate
- **Two-batch / Two-phase**: Alles was *während* der Abarbeitung scheduled wird, läuft **nächster Frame** (Anti-Reentrancy / deterministisch)
- **Budget/Cap**: pro Frame max. N Einträge abarbeiten (Schutz vor perf-killer scripts)

---

## 2) Datenstrukturen / neue Members

### 2.1 In `CBaseEntity` hinzufügen
Füge neue Pointer für Queue-Verkettung hinzu (separat von MoveWith sibling/child und separat von alten Assist-Pointern):

- `CBaseEntity* m_pPostAssistNext;`
- `CBaseEntity* m_pDesiredNext;`

Füge Queue-Flags hinzu (oder verwende freie Bits in `m_iLFlags`):
- `LF_IN_POSTASSIST_QUEUE`
- `LF_IN_DESIRED_QUEUE`

Hinweis:
- Die existierenden Flags `LF_POSTASSISTVEL`, `LF_POSTASSISTAVEL`, `LF_DODESIRED`, `LF_DESIRED_ACTION`, `LF_DESIRED_THINK` bleiben als “Was soll passieren?”-Markierung bestehen.
- Die neuen `LF_IN_*_QUEUE` sind rein “bin ich schon enqueued?” (Duplikat-Schutz).

### 2.2 In `CWorld` (oder global in movewith.cpp) Queue-Heads/Tails
Definiere pro Queue zwei Batches: **current** und **next**.

Beispiel:
- `CBaseEntity* m_pPostAssistHeadCur;`
- `CBaseEntity* m_pPostAssistTailCur;`
- `CBaseEntity* m_pPostAssistHeadNext;`
- `CBaseEntity* m_pPostAssistTailNext;`

- `CBaseEntity* m_pDesiredHeadCur;`
- `CBaseEntity* m_pDesiredTailCur;`
- `CBaseEntity* m_pDesiredHeadNext;`
- `CBaseEntity* m_pDesiredTailNext;`

Wenn du keine CWorld-Member erweitern willst, lege sie als `static` in `movewith.cpp` an (aber CWorld ist meist sauberer wegen Save/Restore-Integration).

---

## 3) API-Änderungen (welche Funktionen sollen wie arbeiten)

### 3.1 `UTIL_DesiredAction(CBaseEntity* pEnt)`
Muss:
1) Flags setzen:
   - `LF_DODESIRED | LF_DESIRED_ACTION`
2) Entity in **DesiredNext-Queue** enqueuen, falls nicht schon `LF_IN_DESIRED_QUEUE`.

Wichtig: **Nicht** mehr in `World->m_pAssistLink` einhängen.

### 3.2 `UTIL_DesiredThink(CBaseEntity* pEnt)`
Analog:
1) Flags setzen:
   - `LF_DODESIRED | LF_DESIRED_THINK`
2) enqueue in DesiredNext-Queue (Duplikat-Schutz).

### 3.3 Neue Utility: `UTIL_PostAssistVelocity(pEnt, vel)` / `UTIL_PostAssistAVelocity(pEnt, avel)`
Optional (empfohlen, für klare Call-Sites):
- Setze:
  - `pEnt->m_vecPostAssistVel = vel;` + `LF_POSTASSISTVEL`
  - bzw. `m_vecPostAssistAVel` + `LF_POSTASSISTAVEL`
- enqueue in PostAssistNext-Queue (Duplikat-Schutz `LF_IN_POSTASSIST_QUEUE`)

Wenn es schon Call-Sites gibt, die nur Flags setzen: diese auf neue Utils umbauen oder dort ebenfalls enqueue sicherstellen.

---

## 4) Frame-Tick Ablauf (Check-Funktionen neu bauen)

Ersetze `CheckAssistList()` und `CheckDesiredList()` durch:

### 4.1 `MoveWith_ProcessFrameQueues()`
Pseudo-Flow pro Frame:

1) **Swap Batches**:
   - `Cur = Next`, `Next = empty`
   - Für PostAssist und Desired getrennt

2) **Process PostAssist Cur (FIFO)**:
   - Pop head bis null oder Budget erreicht
   - Bei jedem Entity:
     - clear `LF_IN_POSTASSIST_QUEUE`
     - wenn `LF_POSTASSISTVEL`: apply `pev->velocity = m_vecPostAssistVel`, clear flag
     - wenn `LF_POSTASSISTAVEL`: apply `pev->avelocity = m_vecPostAssistAVel`, clear flag
   - Wenn währenddessen neue PostAssist-Requests kommen → landen in Next-Queue und laufen im nächsten Frame

3) **Process Desired Cur (FIFO)**:
   - Pop head bis null oder Budget erreicht
   - Bei jedem Entity:
     - clear `LF_IN_DESIRED_QUEUE`
     - wenn `LF_DODESIRED`:
       - clear `LF_DODESIRED`
       - wenn `LF_DESIRED_ACTION`: clear + call `DesiredAction()`
       - wenn `LF_DESIRED_THINK`: clear + call `Think()`

4) Optional: Wenn Budget erschöpft, Rest des Cur-Batches wieder vorne an Cur lassen oder in Next schieben (definiere klar!).
   - Empfehlung: Rest in Cur lassen und im nächsten Frame *vor* Next abarbeiten (bewahrt FIFO über Frames).

### 4.2 Budgets
Definiere Konstanten:
- `MAX_POSTASSIST_PER_FRAME` (z.B. 512)
- `MAX_DESIRED_PER_FRAME` (z.B. 512)

Wenn Budget überschritten:
- logge rate-limited Warnung (nicht jeden Frame spammen).

---

## 5) Safety: Entity-Removal / Unlinking

### 5.1 Neue Methode in `CBaseEntity::UpdateOnRemove()` (oder äquivalent)
Wenn Entity removed wird, muss es aus Queues “logisch” entfernt werden.

Einfachste safe Strategie:
- Wir entfernen **nicht** aus der Mitte (weil single-linked).
- Stattdessen markieren wir es als “invalid for queue processing”, z.B. über:
  - `pev->flags |= FL_KILLME` ist im HL-Umfeld oft ein Signal
  - oder `m_iLFlags |= LF_MARKED_FOR_REMOVE`
- Beim Queue processing: wenn Entity “removed/killed” ist → skip, Flags löschen (`LF_IN_*_QUEUE`) und nicht dereferenzieren, was gefährlich ist.

Alternative (besser, aber mehr Aufwand):
- Verwende doubly-linked queue nodes oder intrusive list mit prev pointer → dann O(1) unlink on remove.

**Anweisung:** Implementiere mindestens “skip-on-remove” plus robustes Clear der queue flags.

### 5.2 Null-Pointer / World fehlt
Wenn `CWorld::World` null ist: keine Verarbeitung, aber enqueue sollte nicht crashen.
- Entweder: enqueue nur wenn World existiert, oder: queues global halten bis World ready.
- Empfehlung: in enqueue functions `if (!CWorld::World) return;`

---

## 6) Determinismus / Reentrancy Regeln (müssen exakt so gelten)

1) Alles, was während `DesiredAction()` oder `Think()` erneut `UTIL_DesiredThink/Action` aufruft, wird in **DesiredNext** enqueued → läuft **nächster Frame**.
2) Gleiches für PostAssist: Requests aus Desired/Think heraus laufen im nächsten Frame.
3) Keine LIFO-Einfügepunkte: immer an Tail enqueuen.
4) Pro Queue pro Entity maximal ein Node (Duplikat-Schutz via `LF_IN_*_QUEUE`).

---

## 7) Testplan (muss umgesetzt werden)

### 7.1 Unit/Debug Tests (in-game debug)
Füge temporäre Debug-Kommandos oder asserts hinzu:

1) **FIFO-Reihenfolge-Test**
   - Spawn 10 test entities E1..E10
   - schedule DesiredThink in der Reihenfolge E1..E10
   - logge die Ausführungsreihenfolge → muss exakt E1..E10 sein

2) **Reentrancy-Test**
   - Entity A’s DesiredAction schedult sich selbst erneut (und B)
   - Erwartung: A läuft nicht unendlich in einem Frame, sondern max einmal pro Frame

3) **Removal-Test**
   - schedule DesiredThink für Entity X
   - entferne X bevor Queue verarbeitet wird
   - Verarbeitung darf nicht crashen, und queue flags müssen sauber cleared werden

4) **Performance-Test**
   - schedule 5000 desired calls
   - sicherstellen, dass Budget greift und das Spiel nicht “hart” stottert
   - logge wie viele pro Frame abgearbeitet wurden

### 7.2 Regression
- Stelle sicher, dass MoveWith-Propagation (Origin/Angles/Velocity) unverändert funktioniert.
- Stelle sicher, dass alte Flags (`LF_POSTASSISTVEL`, `LF_POSTASSISTAVEL`, `LF_DODESIRED`, etc.) weiterhin korrekt funktionieren.

---

## 8) Migrationshinweise (wie vorhandenen Code anpassen)

1) Entferne/Deprecate Nutzung von `CWorld::World->m_pAssistLink` für Desired/PostAssist.
2) Suche Call-Sites, die `LF_POSTASSISTVEL`/`LF_POSTASSISTAVEL` setzen, und stelle sicher, dass dort zusätzlich enqueue in PostAssistQueue passiert (oder ersetze durch neue UTIL-Funktionen).
3) `CheckAssistList()` und `CheckDesiredList()`:
   - entweder ersetzen durch neuen `MoveWith_ProcessFrameQueues()`
   - oder `CheckAssistList()` wird nur ein Wrapper, der `MoveWith_ProcessFrameQueues()` ruft.

---

## 9) Definition of Done (DoD)

- Keine Full-Scans mehr über eine “ever-growing” Assist-Liste.
- Deterministische FIFO Ausführung der deferred Tasks.
- Scheduling aus innerhalb `DesiredAction/Think` läuft garantiert erst im nächsten Frame.
- Kein Crash bei Entity-Removal vor Queue-Verarbeitung.
- Budgets vorhanden und getestet.
- Keine spürbaren Behavior-Regressions in bestehenden Maps (smoke test).