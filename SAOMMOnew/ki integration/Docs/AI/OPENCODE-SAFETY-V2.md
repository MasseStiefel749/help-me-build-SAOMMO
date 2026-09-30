# SAOMMO — OpenCode Safety Protocol V2 (Vibe-Coding-Betriebssystem)

**Stand:** 2026-09-30, Repo `main` 96 Commits, Engine live 5.8
**Gilt mit:** `AGENTS.md` + Band 6 (`SAOMMOnew/ki integration/projekt-ki-promts-6-of-6`) + `.clinerules` + `opencode.json`
**Ziel:** So wenig Fehler wie möglich, wenn du mit Open Code arbeitest.

---

## 1. Master-Loop (immer so)

```text
READ → UNDERSTAND → PLAN (max 5-10 Punkte) → CHANGE ONE THING → BUILD
→ TEST → REGRESSION → DIFF-REVIEW → DOCS → COMMIT → STOP
```

Nie `READ → GENERATE EVERYTHING`.

## 2. Kontext-Hierarchie V2 (gegen Widersprüche)

1. Live-Code / `SAOMMOnew/SAOMMOnew.uproject` (EngineAssociation 5.8 gewinnt)
2. `PROJECT-STATE.md` (die baust du in DOC-00)
3. Aktive ADR (`ADR-010/011/013e/014/...`)
4. Band 6 AI-Manual (bindend)
5. Band 2 Architektur §0.1 (5.8-Hinweis beachten)
6. Fachband (1 Vision, 3 Gameplay, 4 World, 5 Art, 7 Lore, 8 Onboarding)
7. Role-Prompt `Docs/AI/agents/<role>.md`
8. `LOCAL-SETUP.md` (Ollama qwen3-coder:30b, 16K, Compact ON)
9. Alte Doku / README-5.4.4-Hinweis (historisch)
10. AI-Annahme — **darf nie als Fakt gespeichert werden**

Sag bei Unklarheit: `UNKNOWN: ...` statt zu raten.

## 3. Vor jedem Task (Pflicht)

```text
git status  # muss clean sein, sonst Checkpoint-Commit vorschlagen
git branch --show-current
```

Dann lesen: AGENTS.md → Band 6 → passendes Band → PROJECT-STATE.md → relevante Source unter `SAOMMOnew/Source/SAOMMOnew/` (existiert? erst suchen, dann erstellen).

`opencode.json` hat `question:* = deny` — Open Code fragt dich nicht. Dein Prompt muss also Ziel, Erlaubt/Verboten, Abnahme, Build-Befehl enthalten (siehe TASK-Template in Branch-Bands V2 Kap.17).

## 4. Scope-Lock (vor dem Editieren aussprechen)

```text
TASK: ...
ALLOWED: ...
NOT ALLOWED: ...
FILES EXPECTED: ...
ACCEPTANCE: ...
```

Brauchst du doch eine extra Datei? 1. Warum sagen, 2. prüfen ob wirklich nötig, 3. erst dann anfassen. Max. ~3 Dateien pro Run.

Erlaubt: `SAOMMOnew/Source/ Config/ Plugins/ ki integration/`
Verboten: `Binaries/ Intermediate/ Saved/ DerivedDataCache/ .vs/ Backup/ *.uasset` + (per .gitignore) `__pycache__/ *.pyc`.

## 5. Unreal-Spezifisch (häufigste Vibe-Fehler)

- C++: `generated.h` letzter Include, UObject-Lifecycle, keine Raw-Pointer-Flut, kein `NewObject`/Spawn im Tick, `BeginPlay/EndPlay/OnComponent` korrekt, Ownership/Lifetime prüfen.
- Tick-Audit vor jedem neuen Tick: Geht es event-/timer-/callback-basiert? Wenn Tick muss: Warum, Frequenz, Perf-Notiz.
- Null-Audit: Kann Actor/Component/Target/Level/Player/VR-Device weg sein?
- Blueprints nur für Config/Anim/Visual/UI-Logik, Core in C++.
- Nur Enhanced Input. OpenXR nutzen, keine VR-Gameplay-Logik in Shared-Gameplay-Funktion. OpenXR-1.1 bleibt aus (`bIsOpenXR1_1Enabled=False`) bis ADR es erlaubt.
- Multiplayer-Audit auch im Singleplayer: Wer hat Authority? Wer ownt? Kosmetisch oder gameplay-kritisch? Noch kein Netzwerk einbauen, nur dokumentieren.

## 6. Build- / Test- / Diff- / Commit-Gates

**Build:** Nach jeder C++-Änderung `RunAutomation.ps1` passende Stufe (0-5) + Log-Whitelist beachten. Nur eigene Fehler fixen, eine Fehlerklasse gleichzeitig.

**Test:** Feature-Test (nur Neues) + Regression (Nachbarn). Beispiel Sword-Velocity: Attach, Move, Rotate, Velocity, Collide, Player-Move. Arena-Tests + `SAOMMOnew.Tests` nutzen, `perf_audit.py` für Art-Änderungen.

**Diff:**
```text
git diff --stat
git diff
```
Jede Datei erklären können. Debug-/Test-Spam, Format-Rauschen, generierte Files, neue Dependencies → reverten.

**Commit:** Erst bei BUILD+TEST+REGRESSION+DIFF+DOCS grün. Klein + Message:
`feat(input): add desktop input frame mapping`
`feat(combat): add basic sword hit detection`
`fix(ai): handle invalid attack target`
Nie: `update, fix, stuff, AI generated`. Nie force-push / history rewrite.

**Branch:** 1 Ziel. `feature/b07-04-sword-velocity`, nie `feature/sword-combat-final`.

## 7. Locks (verhindern „gute Ideen“, die alles kaputt machen)

- **Refactor-Lock:** Funktionierendes nicht „verschönern“ ohne Task.
- **Architektur-Lock:** Bei Architektur-Entscheid STOP + Bericht (Problem/Bestand/Optionen A-B/Betroffene Systeme/Empfehlung), erst nach Entscheid bauen.
- **Plugin-Lock:** 7 Fragen (Problem? Jetzt nötig? Kann UE es schon? 5.8-kompatibel? Deps? Maintenance? Entfernbar?) → ADR.
- **Lore-Lock:** Tech-AI ändert keine Lore. World-AI nur in Band 7. Konflikt → STOP.
- **Asset-Lock:** Source/Lizenz/kommerziell/Attribution/Änderung in `ASSET-LICENSES.csv`, sonst kein Import. CC0-Bestand (Rack/Barrel/Bench, Metal038, M_CC0Metal) wiederverwenden. ADR-014-Quarantäne beachten.
- **Generated-Lock:** s.o. Verbotsliste.

## 8. Failure-Protokoll

```text
1. Stop 2. Exakten Fehler sichern 3. Reproduzieren 4. Erste failing Schicht finden
5. Nur die fixen 6. Build 7. Test
```
Keine 15 Random-Fixes. Kein Fehler-Wegkommentieren. Keine Warnungs-Flut ignorieren.

## 9. Bestes Vibe-Pattern (so sprichst du mit Open Code)

Schlecht: „Mach SAOMMO.“
Gut:
> „Mach B07-04. Lies zuerst AGENTS.md, Band 6, Band 2 Kap.8/9, PROJECT-STATE.md. Untersuche vorhandenen Sword-Code (ASAOSword, MI_PracticeBlade). Implementiere ausschließlich Velocity=(Curr-Prev)/DT mit Guard. Erlaubt: Source/.../SAOSword.* Verboten: alles andere. Baue mit RunAutomation Stufe 2. Teste. Zeige Diff. Committe erst nach Prüfung. Starte danach nichts Neues.“

Langweilig = stabil. Viele kleine verifizierte Zustände schlagen viel generierten Code:
```text
THIS WORKS. THIS IS TESTED. THIS CHANGE IS UNDERSTOOD. THIS COMMIT IS REPRODUCIBLE.
```

## 10. Was du jetzt tun musst (TODO für dich)

- [ ] Diese V2 + BRANCH-BANDS-V2 ins Repo unter `SAOMMOnew/ki integration/Docs/AI/` legen (nicht Root)
- [ ] DOC-00 zuerst ausführen (PROJECT-STATE anlegen)
- [ ] Root-`projekt-ki-promts-*` als legacy markieren, aktive = `ki integration/`-Kopien
- [ ] Pro Open-Code-Run nur 1 Micro-Branch aus V2 kopieren
- [ ] Nach Vertical Slice (B12) Combat locken
