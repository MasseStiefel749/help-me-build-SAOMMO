# OVERNIGHT 2026-09-24 — Morgenbericht

> Diese Datei bleibt **uncommitted** (bewusst). Sie ist der morgendliche Übergabebericht.
> Commit-Regel: eine logische Änderung pro Commit (Band 8).

## Auftrag

**Phase 1 von 3 — erledigt:**

1. Ehrlicher Vertical-Slice-Audit nach **Band 3 §20** (Desktop-Punkte 1–4 implementiert,
   Punkte 3–4 noch nicht runtime-proven; Punkte 5–8 brauchen den Headset).
2. **Block 3 = VR-Fundament** vorbereiten: `VRCharacter` an das GameMode binden (Band 2 §7),
   Hand-Anchoren + Schwert — nur was sich sinnvoll ohne Headset vorbereiten lässt.
3. Kontinuierlich das Obsidian-Vault `D:\SecondBrain` aktualisieren (long-term memory).

**Phase 2 von 3 — Band-Doku-Durchgang (nur Dokumentation, kein Code):**

1. Death-Screen-Präsentationslayer in Band 3 §11 nennen (Folge aus ADR-016-Konsequenzen).
2. Spec-Lücken bei Creator-Farben und Body-Types dokumentieren.

**Phase 3 von 3 — CC0-Texturen (Pending-Item 4/5):**

1. Passende CC0-Texturen für `Content/Materials` beschaffen (Skill `free-asset-pipeline`,
   Band 5 §18 Lizenz-Check, §17 Speicherbudget, §15 Ordnerstruktur).
2. Importieren + Lizenzen in `ASSET-LICENSES.csv` eintragen.
3. Nicht-Mögliches (z. B. `EnhancedInputSequence` aus Fab) nur als offenen Punkt dokumentieren.

## Regeln dieser Nacht

- Antwort auf Deutsch; Bänder sind Quelle der Wahrheit (Band 6 §10 vor jeder Änderung lesen;
  Ehrlichkeit/Verifikation = §19/§20 — die frühere §25/§26-Angabe war falsch).
- **Live Coding erlaubt** (neue Regel): C++-Änderungen über
  `Build.bat SAOMMOnewEditor Win64 Development -project=...\SAOMMOnew.uprojet` im laufenden
  Editor patchen. **Editor NIEMALS schließen/neustarten.** Hot Reload nur als Fallback bei
  ABI-Konflikten. Voller Rebuild weiterhin VERBOTEN — falls `.Build.cs`, `Target.cs`,
  `.uproject` oder Engine-Plugins geändert würden: als offener Punkt **dokumentieren**, nicht ausführen.
- Keine Installationen.
- Eine logische Änderung pro Commit, Push nach jedem fertigen Punkt.
- Keine Rückfragen — autonom entscheiden und jede Entscheidung dokumentieren (ADR, wo das Band sie verlangt).

## Repowelt (Start)

- Branch: `main` = `2712723` (= ADR-016), synchron mit origin.
- Nur untracked: `opencode.json` (Harness-/User-Config, kein Projektcode → liegen lassen, dokumentieren).
- Achtung: AGENTS.md nennt Branch `going-to-make-an-project` — existiert lokal nicht;
  die aktuelle Session-Arbeit liegt auf `main`. → als Diskrepanz dokumentieren, nicht umcheckouten.

## Fortschritt (laufend)

- [x] Vault-Kontext gelesen (Agent memory, Do Nots, Session Handoff, Ultra Prompt), Skills geladen.
- [x] Bänder 1, 2, 3 (§20), 4 (§17), 6 (§10/§19/§20), 8 gelesen; ADR-005…016 gesichtet.
- [x] Engine-APIs verifiziert (keine erfundenen Namen):
      `UMotionControllerComponent::MotionSource`/`SetTrackingMotionSource`,
      MotionSources `"Left"`/`"Right"` (`IMotionController::LeftHandSourceId`),
      `AGameModeBase::GetDefaultPawnClassForController` = BlueprintNativeEvent
      (in `generated.h` bestätigt), `GEngine->XRSystem` vorhanden.
      `HeadMountedDisplayFunctionLibrary.h` **nicht** auffindbar → nicht verwenden.
- [x] Audit-Dokument `SAOMMOnew/ki integration/VERTICAL_SLICE_AUDIT_5.8.md` geschrieben →
      `44fca9e`; danach Konsistenz-Pass gegen alle Bänder/ADRs/WIRING_GUIDE/PLUGIN_AUDIT →
      Korrekturen in `4e0343c` + `0692ba0` (Score 9/4/1, §19/§20 statt §26, P1/P6/P14 präzisiert,
      VR-Block 5–8, Abgrenzungssektion „Relation to the other 5.8 documents"). **Gepusht.**
- [x] Vault final aktualisiert: `10 Projects/SAOMMO.md`, `99 AI/Agent memory.md`,
      `99 AI/Session Handoff.md` (§25/§26 → §10/§19/§20 korrigiert), `50 Daily/2026-09-24.md`,
      `Home.md`; neu: `10 Projects/SAOMMO — Widerspruchstagebuch` (Widerspruchsregister),
      `10 Projects/SAOMMO ADRs and Bands` (ADR-005…017 + Bandpfade),
      `99 AI/Live Coding Status` (Build-Stand je Commit + Nachtregeln).
- [x] **Phase 2 — Band-Doku-Durchgang, beide Punkte gepusht:**
      `bd4689e` „Mention the death-screen presentation layer in Band 3 section 11"
      (Text zwischen Flow-Diagramm und Zukunftsliste, verweist auf ADR-016, betont
      presentational only — keine Penalties/Checkpoints/Resurrection);
      `3fee99f` „Record the creator color and body type spec gap in Band 5 section 3"
      (Spec gap (open): Body-Typen + Skin/Hair/Eye-Farben unentschieden, ADR-016 =
      Interimsverhalten, kein Spec). Beide Änderungen in **zwei Kopien** je Band
      (kanonisch `SAOMMOnew/ki integration/`, identischer Text im Root-Spiegel —
      Hash-Identität danach verifiziert).
      **Entscheidungen:** (a) kanonische Kopie = `ki integration` (ULTRA-PROMPT Step 1),
      Root-Mirror mitgezogen, damit die nachts geprüfte Hash-Identität der Bänder 2/3/5–9
      erhalten bleibt (keine neue Band-4-artige Drift); (b) Band 3 §20 **nicht** geändert —
      Nutzer hat §11 explizit benannt, §20 bleibt Capability-Checkliste (13 Die / 14 Restart);
      (c) keine Versionsbump auf 1.1 — Git liefert die Traceability (Band 6 §13/§14),
      Header-Format retrofitten wäre Scope-Creep; (d) **kein** neues ADR — ADR-016 enthält
      die Entscheidung bereits, hier wird nur seine Consequence umgesetzt, die Spec-Lücke
      wird notiert, nicht entschieden; (e) Spec-Lücke nach **Band 5 §3** (Repräsentations-Home,
      ADR-016c verweist selbst auf „Band 5 §3–§5"), nicht nach `Docs/World/open-questions.md`
      (Authority Band 7 §12/Band 4 §14, „World & Lore", ausdrücklich „do not fill in by accident").
- [x] **Phase 3 — CC0-Texturen, beide Punkte gepusht:**
      `0922f87` „Import three CC0 texture sets into Content/Materials" (12 `.uasset`):
      **Ground037** (Waldboden Moos/Gras, Starting Reach), **Rock063** (verwitterter
      moosiger Cliff, Brunnfelder Steine/Ruinen), **Planks009** (alte Bretter, Gerüste/
      Zäune/Alte Straße) — je Color/NormalGL/Roughness/AO, 1K JPG, ambientCG, CC0 1.0;
      `a5ab9f7` „Add ASSET-LICENSES.csv for the imported CC0 texture sets" (12 Zeilen
      mit Quelle, Source-URL, Lizenz + Lizenz-URL, Quelldatei, sRGB/Compression-Flag
      als Verifikationsnachweis, Zweck).
      **Import-Weg (Entscheidung):** MCP-Bridge war tot (`mcp-remote: fetch failed`,
      zweimal — im laufenden Editor nicht neustartbar, Neustart verboten) → stattdessen
      headloser Import via `UnrealEditor-Cmd -run=pythonscript` (PythonScriptPlugin ist
      in der `.uproject` aktiv; Skill erlaubt explizit Python-Batch-Import). **Kein
      Build, Editor unberührt, separater Prozess, nur neuer Ordner + neue Assets,
      nichts Bestehendes angefasst** (Band 6 §21).
      **Verifiziert (Band 6 §20):** Commandlet-Log „Python script executed successfully"
      + Interchange-„import completed" für alle 12 Quelldateien; Flags per Rücklese-Skript
      bestätigt — Color sRGB=on, alle Datenkarten sRGB=off, Normale = `TC_NORMALMAP`,
      Rest = `TC_DEFAULT`. (Das „problems=9" im Bericht war ein Groß-/Kleinschreibungs-
      Fehler meines Vergleichs (`TC_Default` vs. `TC_DEFAULT`), kein Asset-Problem.)
      **Weitere Entscheidungen:** nur 3 Sets statt Katalog-Flut (Band 6 §12);
      Normal**GL** statt NormalDX (UE = OpenGL-Konvention); Displacement/NormalDX/
      Previews/`.blend`/`.usdc` nicht importiert (kein Tessellieren, redundant,
      per CSV-URL nachladbar); Quell-Downloads bleiben im Temp-Staging (nicht im Repo);
      CSV am Repo-Root (am auffälligsten neben AGENTS.md/README); keine `M_`/`MI_`-
      Mastermaterial-Erstellung (Band 5 §9 = separater Art-Schritt, nicht Teil dieser Phase).
- [ ] **Aufgeschoben (eigene Commits nächste Session):**
      P1 `GlobalDefaultGameMode` → `/Script/SAOMMOnew.MainGameMode` (Wirkung erst nach
      Prozess-Neustart, Editor-Neustart heute Nacht verboten → nicht verifizierbar);
      Block 3a/3b/3c (GameMode-Pawn-Wahl, MotionController-Hands, Schwert-Attach);
      ADR-017. Grund + Mechanismus stehen in `99 AI/Session Handoff` Pending 2+3 und
      `99 AI/Live Coding Status`.

## Offene Punkte / Kandidaten

- `GlobalDefaultGameMode` in `Config/DefaultEngine.ini` zeigt auf nicht existierendes
  `BP_MainGameMode` (Dateiname seit `7dcadc6` zurückbenannt, Redirects stehen weiterhin auf
  die neuen Namen) — `L_StartingReach.umap` hat aber nativen `MainGameMode`-Override
  (per String-Scan verifiziert), deshalb bootet das Spiel. → P1, eigener Commit.
- Alle `[CoreRedirects] PackageRedirects` zeigen von existierenden Altnamen auf fehlende
  Neunamen (platiniert geprüft: `BP_SAOMMOGameMode` existiert, `BP_MainGameMode` nicht;
  `IMC_SAOMMO` existiert, `IMC_Player` nicht) — Laufzeffekt ungeprüft, siehe
  Vault-Notiz `10 Projects/SAOMMO — Widerspruchstagebuch` (Item 13).
- VR-Respawn-Lücke: `MainPlayerController::DoRespawn` spawnt `APlayerCharacter`
  (`CharacterClass`/`DefaultCharacterClass` typisiert `TSubclassOf<APlayerCharacter>`) —
  VR-Klasse typseitig nicht ausdrückbar.
- Build-Status: ADR-016 (`2712723`) und alles davor sind gebaut (voller `Build.bat`-Proof);
  `44fca9e`/`4e0343c`/`0692ba0` (Phase 1) und `bd4689e`/`3fee99f` (Phase 2) sind docs-only,
  `0922f87`/`a5ab9f7` (Phase 3) sind Asset-/CSV-Only — alles nach `2712723` braucht keinen
  Build. Erster Code-Change der nächsten Session braucht den Live-Coding-Check →
  `99 AI/Live Coding Status`.
- **Phase 3 offen geblieben (wie beauftragt nur dokumentiert):**
  1. `EnhancedInputSequence`-Plugin: in `SAOMMOnew.uproject` auf `Enabled: false` mit
     `MarketplaceURL` (Fab-Produkt `c33feca9…`) gesetzt — Download aus Fab erfordert den
     Epic-Account/Launcher-Interaktion → **nicht ohne Simon möglich**; danach Plugin in der
     Editor-Preferences aktivieren und die Input-Sequenz-Tools prüfen.
  2. **MCP-Bridge tot** (`http://127.0.0.1:8000` → `mcp-remote: fetch failed`) — hängt am
     laufenden Editor; Neustart verboten → nächste Session braucht zuerst eine funktionierende
     Bridge, bevor Editor-Automatisierung (PIE, Asset-Scans) genutzt werden kann.
  3. **Editor-FrischecHECK am Morgen:** Die 12 Texturen liegen auf Disk + sind im Commandlet
     verifiziert; der laufende Editor zeigt sie nach Fokus/F5 im Content Browser unter
     `Content/Materials` (automatischer Nachweis mangels MCP nicht möglich).
  4. Erstschritt Material-Arbeit (offener Art-Schritt, nicht Teil von Phase 3): Mastermaterial
     `M_…` + Instanzen `MI_…` (Band 5 §9) bauen, das die vier Karten je Set verkabelt.

---

## Selbstauferlegte nächste Aufgaben

Nachtschicht-Abschluss 2026-09-24. Abgeleitet aus den Bändern `projekt-ki-promts-1..9`
(vorallem Band 3 §20, Band 4 §17/§20, Band 5 §3/§9), `VERTICAL_SLICE_AUDIT_5.8.md` inkl.
Gaps G1–G8 und den ADRs — **jede Aufgabe mit Quelle, nichts erfunden**. Kanonische Tabelle
mit Wikilinks: `D:\SecondBrain\10 Projects\SAOMMO.md` → „Nächste Aufgaben (selbstauferlegt)`;
dieselbe Reihenfolge ist die Pending-Liste in `D:\SecondBrain\99 AI\Session Handoff.md`.
Größen: S ≤ ½ Tag · M = 0,5–2 Tage · L > 2 Tage. **Keine Commits in dieser Runde** (Nacht
vorbei) — alles nur dokumentiert; Tree unverändert (`main = a5ab9f7`, sync).

| # | Aufgabe | System | Größe | Abhängigkeiten | Test | Quelle |
|---|---|---|---|---|---|---|
| 1 | P1-Boot-Fix: `DefaultEngine.ini:4` → `/Script/SAOMMOnew.MainGameMode` | World/Levels | S | keine | PIE | Band 3 §20 P1 · Audit G1/P1 |
| 2 | Morgen-PIE-Playtest Desktop-Kette P1–P4 + P9–P13 (+F5 für Texturen) | Combat/Creator/HUD | S | Editor offen | **PIE** | Band 3 §20 · Audit G8/P3/P4 · ADR-016 |
| 3 | VR-Pawn-Wahl (Block 3a, `GEngine->XRSystem` → `AVRCharacter`) | VR/Input | S–M | P1 hilfreich | Headset | Audit G2+G6/P5 · Band 2 §7 |
| 4 | Motion-Controller-Hände (Block 3b, `MotionSource` Left/Right, Mirror → `FInputFrame`) | VR/Input | M | #3 | Headset | Audit G3/P6 · Band 2 §7/§3 |
| 5 | VR-Schwert (Block 3c, Attach + `CombatComponent` auf VR-Pawn; CombatComponent selbst nicht anfassen) | Combat + VR/Input | M | #3+#4 | Headset | Audit G4/P7/P8 · Band 2 §8/§9 |
| 6 | ADR-017 (VR-Umfang, Live-Coding-Status, Respawn-Modell) | VR/Input (ADR) | S | #3–#5 | — | Band 6 §15 · Audit „planned ADR-017" |
| 7 | VR-Respawn (G5: `TSubclassOf<APlayerCharacter>`-Hartkodierung weiten) | Progression/Checkpoint | M | #6 | Headset | Audit G5/P14 · Band 3 §20 P14 |
| 8 | Headset-Nachweis Punkte 5–8 (+14) → schließt G8 | VR/Input + Combat | M | #3–#5, #7 | **Headset** | Band 3 §20 P5–P8 · Band 1 §8 |
| 9 | Mastermaterial `M_`/`MI_` für die3 CC0-Sets (4 Karten je Set) | World/Levels | M | `0922f87` | Editor | Band 5 §9 · `ASSET-LICENSES.csv` |
| 10 | Test-Arena-Checklist (Traverse, Zonen, Landmarks, Artefakt-Teaser, Lore, VR+Desktop; §6 Licht, §7 Props) | World/Levels | L | #9, VR-Hälfte #8 | PIE + Headset | Band 4 §17/§20 · `test-arena-spec.md` §6–§8 |
| 11 | Creator-Spec-Entscheidung (Owner): Body-Typen + Farben kanonisch festlegen | Creator | S | Simon | — | Band 5 §3 „Spec gap" · ADR-016c · Widerspruchstagebuch 16 |
| 12 | `EnhancedInputSequence` aus Fab laden + aktivieren | VR/Input | S | Epic-Account | Editor | `.uproject` (MarketplaceURL) · Band 2 §3 |
| 13 | AGENTS.md Branch-Hinweis korrigieren (G7) | Prozess/Doku | S | keine | — | Audit G7 |
| 14 | Nach dem Slice: Hair-/Eye-Assets + Skin-Tint-Ziel (Creator), dann Checkpoints/Respawn (Band 3 §11) + Progression (Band 3 §12) | Creator + Progression | L | #11 | Headset/PIE | Band 5 §3–§5 · ADR-016c · Band 3 §11/§12 |

Nicht im Spiel-Backlog: G8 = Meta (durch #2+#8 geschlossen), Windows-Reboot = Betrieb
(Handoff), Save hat keinen offenen Slice-Task (Band 2 §15 `USaveGame` über `CharacterSave`,
ADR-016c). **Top-3 für den Morgen: #1 P1-Boot-Fix → #2 PIE-Playtest → #3 VR-Pawn-Wahl.**
