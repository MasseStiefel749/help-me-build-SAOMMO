# SAOMMO — Branch-Bänder V2 / Micro-Step Build Plan

**Stand:** 2026-09-30 — geprüft gegen Live-Repo `MasseStiefel749/help-me-build-SAOMMO` (96 Commits, Branch `main`)
**Zweck:** Jede alte „Band“-Aufgabe in so kleine Branches zerlegen, dass Open Code pro Run nur 1 Sache ändern kann.
**Live-Wahrheit:** `SAOMMOnew/` UE 5.8 (`.uproject` EngineAssociation 5.8), C++, Enhanced Input, OpenXR. Docs erwähnen historisch 5.4.4 — Code gewinnt.

> Goldene Regel: **1 Branch = 1 testbares Ziel = max. 3 Dateien. Danach BUILD → TEST → DIFF → COMMIT → STOP.**

---

## 0. Was sich für mich geändert hat (Repo-Diff)

1. **8 → 96 Commits.** Test-Arena, NavMesh, CC0-Props, Master-Material, Automation-Tests, VR-Desktop-Fallback, SaveGame-Roundtrip, XP/HUD, ItemPickup, EnemySpawner — alles schon angefangen.
2. **AGENTS.md ist jetzt bindend.** Liest Band 6 zuerst (`SAOMMOnew/ki integration/projekt-ki-promts-6-of-6`), dann Band 2, dann Fachband, dann Role-Prompt `Docs/AI/agents/<role>.md`, dann `LOCAL-SETUP.md`.
3. **Branch-Regel = `main`.** `going-to-make-an-project` ist voll in `main` gemerged — nie auschecken.
4. **UE 5.8 live, 5.4.4 nur historisch.** Band 2 §0.1 sagt das explizit. README sagt noch 5.4.4 → nicht als Wahrheit nehmen.
5. **`.clinerules` + `opencode.json` existieren:** Ollama `qwen3-coder:30b`, Compact Prompt ON, 16K Kontext, `question:* = deny`. Open Code darf dich also nicht zurückfragen — Tasks müssen komplett sein.
6. **Headless-Tooling vorhanden:** `perf_audit.py`, Headless-Mesh-/Texture-Importer, `RunAutomation.ps1` Stufen 0-5 + Log-Whitelist, `SAOMMOnew.Tests` (5 Tests), `OVERNIGHT-2026-09-24.md`, ADR-010/011/013e/014.
7. **Verbote klar:** nie `Binaries/ Intermediate/ Saved/ DerivedDataCache/ .vs/ Backup/ *.uasset` per Text-Tools anfassen. Nur `SAOMMOnew/Source/ Config/ Plugins/ ki integration/` lesen/editieren.

Deine alten V1-Dateien in `Downloads/` sind noch gut, aber gehen von 8 Commits / UE 5.8.1 aus. Diese V2 ersetzt die Annahmen durch geprüfte Fakten + Copy-Paste-Prompts.

---

## 1. Branch-Namensschema (so musst du es nutzen)

```text
feature/DOC-00-doc-truth
feature/b01-01-engine-check
feature/b01-02-editor-build
feature/b02-01-inputframe-struct
...
```

Niemals `feature/sword-combat-final`. Immer `feature/b07-04-sword-velocity`.

Vor jedem Run in Open Code:

```text
git status           # muss clean sein
git branch --show-current  # muss main oder feature/... sein
```

---

## 2. DOC-00 — Dokumentations-Wahrheit (ZUERST, kein Code!)

**Ziel:** Eine Wahrheit für Engine, Pfade, Bänder.

- [ ] `SAOMMOnew/SAOMMOnew.uproject` öffnen → `EngineAssociation` notieren (=5.8)
- [ ] `AGENTS.md`, `.clinerules`, `opencode.json`, Band 2 §0.1, Band 6 lesen
- [ ] README-Hinweis „5.4.4“ als historisch markieren (nicht löschen, nur ADR-Verweis)
- [ ] Festlegen: Aktive Bänder = `SAOMMOnew/ki integration/projekt-ki-promts-X-of-6` + `Docs/AI/`
- [ ] Root-Duplikate `projekt-ki-promts-*` als `legacy/` markieren (nicht wild löschen)
- [ ] `SAOMMOnew/ki integration/Docs/AI/PROJECT-STATE.md` anlegen mit:

```md
# PROJECT-STATE
Current engine: 5.8 (uproject)
Current branch: main
Last verified build: <Datum + Stufe RunAutomation>
Last verified gameplay: <z.B. TestArena Movement+Pickup>
Current feature: <Branch>
Blocker: <kein / was>
Known bugs: <Liste>
Next micro-task: <z.B. B02-01>
```

- [ ] Commit: `docs(state): add PROJECT-STATE and doc-truth`

**Abnahme:** Open Code kann sagen welche Datei für Vision/Architektur/Gameplay/World/Art/AI/Lore gilt.
**Copy-Paste-Prompt:**

```text
SAOMMO MICRO TASK DOC-00. Lies AGENTS.md, Band 6, Band 2 §0.1. Ändere keinen C++-Code.
Erstelle nur Docs/AI/PROJECT-STATE.md. Erlaubt: Docs/AI/. Verboten: Source/, Config/.
Prüfe uproject EngineAssociation. Stoppe danach. Kein weiterer Task.
```

---

## 3. B01 — Foundation (nur stabil machen, nichts neu erfinden)

### B01-01 Engine-Check
- [ ] `.uproject` lesen
- [ ] Installierte Engine prüfen (UE 5.8)
- [ ] Bei Mismatch: STOP + REPORT, keine Auto-Migration
- [ ] In PROJECT-STATE eintragen

### B01-02 Editor-Build
- [ ] `RunAutomation.ps1` Stufe 0-1 laufen lassen (statt blind Build im Editor)
- [ ] Nur eigene Scope-Fehler fixen
- [ ] Log-Whitelist beachten (AIModuleToolset ist schon deaktiviert per #22 — nicht reaktivieren)

### B01-03 Git-Baseline
- [ ] `git status`, `.gitignore` prüft `__pycache__/ *.pyc Binaries/ Intermediate/ Saved/`
- [ ] Keine generierten Dateien stagen

Prompt-Vorlage für B01:
```text
SAOMMO MICRO TASK B01-02. Lies AGENTS.md + LOCAL-SETUP.md. Führe RunAutomation Stufe 1 aus,
fixe nur Buildfehler in deinem Scope, keine Architekturänderung. Zeige git diff --stat. Stop.
```

---

## 4. B02 — Universal Input Contract (FSAOInputFrame)

Band 2 Kap.3/4 ist Pflichtlektüre. Aktuell gibt es schon `SAOMMOnew.Input.VRDesktopFallback` (#23) + `VRCharacter desktop input fallback` (#16) — **wiederverwenden, nicht neu bauen.**

### B02-01 Struct prüfen (nicht sofort erstellen!)
- [ ] Suche `FSAOInputFrame` in `SAOMMOnew/Source/`
- [ ] Wenn vorhanden: Felder dokumentieren, STOP (kein Duplikat)
- [ ] Wenn fehlend: kleinste Struct mit Move, Turn, HeadRotation, Left/RightHand Pos/Rot, Attack, Menu
- [ ] `generated.h` als letzter Include

### B02-02 Desktop Mapping
- [ ] Nur Enhanced Input Actions prüfen (Move/Turn/Jump/Attack/CameraToggle)
- [ ] Keine Legacy Input API

### B02-03 Desktop → InputFrame
- [ ] Mapping → Frame, noch kein Gameplay-Umbau
- [ ] VR-Pfad nicht brechen (Audit: Desktop geht noch? VR geht noch?)

### B02-04 Debug
- [ ] Temporärer Print, danach hinter `bDebugInput` Flag oder löschen
- [ ] Kein Spam-Log im Tick

---

## 5. B03 — Desktop Movement (Schritt für Schritt)

- **B03-01 Bestand:** Character-Klasse finden, Parent + MovementComponent notieren, nichts ersetzen
- **B03-02 Forward:** nur Vor/Zurück, Build, Test im TestArena Movement-Bereich
- **B03-03 Strafe:** nur seitlich
- **B03-04 Kamera-relativ:** Vektor mit Control-Rotation transformieren
- **B03-05 Jump:** Bodenprüfung, kein Double-Jump erfinden
- **B03-06 Sprint:** nur Speed während Input
- **B03-07 Regression:** Laufen/Strafe/Sprung/Sprint/Stop/Richtungswechsel

Jeder Sub-Step = eigener Commit, z.B. `feat(movement): add forward move B03-02`.

---

## 6. B04 — Kamera FP/TP

- **B04-01 Bestand prüfen** (keine 2. Kamera wenn eine da ist)
- **B04-02 FP:** Position am Kopf, eigenes Mesh ausblenden prüfen
- **B04-03 TP:** SpringArm + Collision-Test
- **B04-04 Toggle:** 1 Input Action, kein Character-Neuspawn, kein Level-Reload
- **B04-05 Übergang:** FP→TP→FP während Laufen/Springen/Angriff testen

---

## 7. B05 — VR Foundation

Beachte: OpenXR-1.1-Init ist aktuell deaktiviert (`bIsOpenXR1_1Enabled=False` per #18c) + `CommonGameViewportClient` gesetzt (#18a) + Creator-Widget focusable (#18b). **Nicht heimlich reaktivieren ohne ADR.**

- **B05-01 Plugins:** OpenXR + Enhanced Input prüfen, nichts extra aktivieren
- **B05-02 VROrigin:** Origin/Camera/Left/Right prüfen
- **B05-03 Head:** Rotation → Kamera, Desktop bleibt heil
- **B05-04 Controller:** Pos/Rot links/rechts
- **B05-05 Koexistenz:** VR starten, Desktop starten, gleicher Gameplay-Code

---

## 8. B06 — Interaction

Es gibt schon `ItemPickup TryPickup-Roundtrip` (#38, ADR-010d). Darauf aufbauen.

- **B06-01 Interface:** existiert? Wenn ja nutzen. Minimal, keine Weapon-Logik
- **B06-02 Highlight:** nur Debug-Highlight, kein UI
- **B06-03 PickUp:** Attach prüfen, Collision definieren
- **B06-04 Drop:** Physik zurück
- **B06-05 Throw:** Velocity vom Controller übernehmen
- **B06-06 Activate:** 1 Testobjekt
- **B06-07 Door:** erste echte Tür (Open, Close optional)

---

## 9. B07 — Sword (der kritischste Pfad)

Aktuell: `M_CC0Metal + MI_PracticeBlade` (#20b) + CC0-Blade-Rack (#24b) vorhanden. Nutze diese Meshes, importiere keine neuen.

- **B07-01 Actor:** `ASAOSword` suchen, sonst minimal: Mesh + Collision + Grip-Point
- **B07-02 Attach:** rechte Hand, Pos/Rot korrigieren, Desktop-Test
- **B07-03 Movement:** Current/Previous Pos + DeltaTime speichern
- **B07-04 Velocity:** `(Current-Previous)/DeltaTime`, Div-by-0 Guard (`if DeltaTime < 1e-6 return`), Clamp bei Explosion
- **B07-05 Rotation:** nur sammeln, kein Damage
- **B07-06 Hit:** 1x Sphere/Overlap, kein Self-Hit, nur 1 Hit pro Fenster
- **B07-07 Blade-Trace:** erst nach B07-06 grün: Blade-Points, Prev/Curr Trace, Fast-Swing-Test

---

## 10. B08 — Combat

VR-Pawn hat schon Health+Death-Path (#15). Wiederverwenden.

- **B08-01 Health:** Max/Current/Damage-Funktion (kein zweites Health-System)
- **B08-02 Damage:** Sword → Enemy, Health runter
- **B08-03 Death:** State, kein Weiter-Angreifen
- **B08-04 PlayerDamage:** Enemy → Player, kein Respawn-System erfinden
- **B08-05 DeathUI:** Overlay + Keypress-Restart + Timer nur Fallback, keine XP-Penalty (Band 3 Regel)

---

## 11. B09 — Enemy + B10 — AI (zusammen testen, getrennt committen)

NavMesh ist gebaut + gespeichert (#17, ADR-013e), CC0-Props haben Blocking-Collision (#24c/d), Spawner hat RootComponent (#24...). Nutze TestArena.

- **B09-01 Actor:** `ASAOEnemy` suchen, Mesh/Collision/Health
- **B09-02 Controller:** AIController besitzt Enemy, keine AI-Logik im Player
- **B09-03 Nav:** auf Arena-Walk-Mesh testen (Bounds-Volume ist schon skaliert)
- **B09-04 Detect:** erst Distanz-Check, dann Perception
- **B09-05 Approach / B09-06 Attack / B09-07 Loop:** Idle→Detect→Approach→Attack→Recover
- **B10-01 Blackboard:** nur TargetActor, State, TargetLocation
- **B10-02 BT:** Root+Selector nur bei Bedarf: Idle/Chase/Attack
- **B10-03 Fail:** Target tot/weg, Nav fail → nicht hängenbleiben
- **B10-04 Debug:** BT-Debugger + Perception + Nav-Debug

Roster-Vertrag beachten (ADR-011b, #36).

---

## 12. B11 — Test Level (Labor, keine Welt)

Arena existiert schon (dressed + zoned per #... + §6/§7/§5). Nur prüfen/erweitern, nicht neu bauen.

- [ ] Ground/Spawn/Licht
- [ ] Movement: Gerade + Rampe + Hindernis
- [ ] Interaction: Pickup + Door + Throw
- [ ] Combat: 1 Enemy + Reset + Sword + PlayerHealth
- [ ] VR: Start + Controller + Sword
- [ ] Schilder: MOVEMENT/CAMERA/VR/INTERACTION/SWORD/ENEMY TEST

Datei-Namen/Asset-Pfade nach `test-arena-spec` (#37) angleichen.

---

## 13. B12 — Vertical Slice (Meilenstein)

Loop: Start→Move→Explore→Find→Fight→Defeat→Reward→Continue. Kein Inventory/MMO nötig.

- **B12-01:** Start, Weg, 1 Enemy
- **B12-02:** Combat grün
- **B12-03:** Simple Reward (Kill-Reward/XP/HUD schon da per #33 — nutzen)
- **B12-04:** Rückkehr/Weiter
- **B12-05 VR-Playtest / B12-06 Desktop-Playtest (FP+TP+Combat)**

Erst wenn das geht → Feature-Lock `Combat architecture: LOCKED`.

---

## 14. B13→B15 — Inventory / Progression / Save (erst nach Slice!)

- **B13:** Item-Typ → Container → Add → Remove → Stack nur bei Bedarf → Weapon → Consumable → UI
- **B14:** XP-Struct → Vergabe → LevelUp → Stats → Equipment wirkt, kein Skill-System ohne Anforderung
- **B15:** SaveGame-Klasse (Roundtrip schon da #33) → Pos → Inventory → Progression → Load → Corrupt-Handling → Versionierung

---

## 15. B16→B18 — World / Art / Lore

- **B16:** Region-Contract (Name/Type/Difficulty/Gameplay) → Mini-Settlement → Wilderness → Dungeon-Eingang → Dungeon → verbinden. Kein World Partition „weil MMO“. Region-Doc `01-starting-reach.md` gegen Band 4 S17/S19 prüfen (#31).
- **B17:** Asset-Inventar (Name/Type/Source/License/Path/Status/UsedBy) → Naming `SM_ SKM_ T_ M_ MI_ AM_ ABP_ IA_ IMC_ WBP_ NS_` (Band 5 S14 schon erweitert #29a) → Sword → Player → Enemy → VFX (Trail/Hit, verdeckt kein Gameplay) → Shared Master Materials (M_CC0Metal schon da) → Perf (perf_audit.py aus #30 nutzen). Quarantäne aus ADR-014 beachten (#29b).
- **B18:** Canon (Ancient/Blade-Culture/Fall/Rebuild/Player) → Location-Template → NPC-Template → Lore-Lock: bei Konflikt STOP, Quelle suchen, Änderung vorschlagen, erst dann implementieren. Player-Strings bilingual halten (#32).

---

## 16. B19→B22 — BudgetVR / Netzwerk / Multiplayer / MMO (spät!)

- **B19:** Protokoll → Timestamp → Head-Rot → Hand-Rot → Hand-Pos (wenn HW kann) → Loss-Handling → Seq-Nummern → Latenz → Invalid-Reject → Bridge→InputFrame → Test. Architektur: Phone→AndroidApp→Network→SAOBudgetVRBridge→FSAOInputFrame→Gameplay.
- **B20:** Singleplayer-Systeme für Replikation markieren → Authority-Doku → Player-Pos-Replikation → Combat-Event → Enemy-Authority → Ownership-Test. Kein MMO.
- **B21:** Listen-Server → 2 Clients → sehen einander → Movement → Interact → Combat → Disconnect → Reconnect/Clean-Fail
- **B22:** Identity → Account → Char-Persistenz → World-Persistenz → Inventory-Persistenz → Guild → Party → Server-Arch → Backend → Security. Nur nach bewiesenem Multiplayer.

---

## 17. B23 QA + B24 AI-Infra (läuft immer mit)

**B23 pro Branch:** Build ok, Feature ok, Regression ok, keine Warning-Flut, nullptr-Check, kein Tick-Missbrauch, kein UObject-Spam, VR+Desktop getestet, Doku stimmt.

**B24:**
- **B24-01 PROJECT-STATE.md** (siehe DOC-00)
- **B24-02 TASK.md Vorlage:**

```text
Task ID: B07-04
Branch: feature/b07-04-sword-velocity
Role: Technical AI
Goal: Sword Velocity berechnen
Read: AGENTS.md, Band 6, Band 2 Kap.8/9, PROJECT-STATE.md
Allowed: Source/.../SAOSword.*
Forbidden: Binaries/, Intermediate/, Saved/, Content/*.uasset
Prereq: B07-03 done
Changes: 1. PrevPos speichern 2. Velocity=(Curr-Prev)/DT + Guard 3. Clamp
Accept: [ ] kompiliert [ ] Velocity plausibel [ ] kein Self-Hit-Bruch
Build: RunAutomation.ps1 Stufe 2
Test: Schwert bewegen, Log prüfen
Regression: Attach, Move, Rotate, Collide, Player-Move
Docs: PROJECT-STATE Next-Task updaten
Commit: feat(combat): add sword velocity calc B07-04
STOP. Keine nächste Aufgabe.
```

- **B24-03 CHANGELOG-AI.md:** nur Architektur-Entscheide (Date/Decision/Reason/Affected/Migration)
- **B24-04 ADRs:** z.B. ADR-017 InputFrame, ADR-018 Hit-Detection, ADR-019 Authority, ADR-020 Save-Format
- **B24-05 Locks:** `Combat: LOCKED nach Slice — Änderung nur mit ADR`

---

## 18. Open-Code-Verbote (aus Band 6 + AGENTS.md)

Kein Rewrite, kein Refactor ohne Branch, keine Engine-Migration ohne ADR, kein Plugin ohne 7-Fragen-Check (Band 6 §22, UE-Version = 5.8 prüfen!), keine Legacy-Input, keine HW-Abhängigkeit im Gameplay, kein MMO während Core instabil, keine Lore-Erfindung als Canon, kein Asset ohne Lizenz in `ASSET-LICENSES.csv`, keine `.uasset` per Text-Tool, keine Artefakte committen, kein Force-Push, keine History-Rewrites, kein Auskommentieren von Fehlern, keine Warnungen ignorieren, kein „while we're here“-Refactor.

**Stop-Bedingungen:** Widerspruch, Datei fehlt, Code unverstanden, Buildfehler außer Scope, Architektur nötig, Lore-Entscheid nötig, Plugin/Asset fehlt, fremdes System müsste mitgeändert werden, mehrere Langfrist-Optionen → `STOP REPORT DO NOT GUESS`.

---

## 19. Empfohlene Reihenfolge

```text
DOC-00 → B01 → B02 → B03 → B04 → B05 → B06 → B07 → B08 → B09 → B10 → B11 → B12
→ [LOCK Combat] → B13 → B14 → B15 → B16 → B17 → B18 → B19 → B20 → B21 → B22
+ B23/B24 immer
```

## 20. Done-Definition

```text
[ ] kompiliert (RunAutomation Stufe passend)
[ ] Feature manuell getestet
[ ] Regression getestet
[ ] diff --stat + diff geprüft, Debug-Code weg
[ ] nur erlaubte Dateien, keine generierten
[ ] PROJECT-STATE + Docs aktuell
[ ] keine neue Warnung
[ ] kleiner Commit mit Message feat/fix/docs(...)
```
