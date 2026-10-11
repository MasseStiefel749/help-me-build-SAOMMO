# PROJECT-STATE — SAOMMOnew (verifizierter Stand)

Date: 2026-10-06 · Status: verifiziert (Build + Tests + Boot-Smoke GRÜN) · Branch: `main`

## Engine / Repo
- Repo: `help-me-build-SAOMMO`, Branch `main`, in sync mit `origin/main`, Arbeitsbaum sauber vor Edits.
- `.uproject`: `SAOMMOnew/SAOMMOnew.uproject`, `EngineAssociation 5.8`.
- Installiert: UE 5.8.3 (`C:\Program Files\Epic Games\UE_5.8`, `Build.version` PatchVersion 3). Keine Migration nötig.
- GameMode: `MainGameMode` (`Config/DefaultEngine.ini` → `GlobalDefaultGameMode`), Startmap `L_StartingReach` (vorhanden, inkl. `L_StartingReach_Ruine`-Variante).

## Nachweise (Run 20261006-234914 + Build 2026-10-06)
- `Build.bat SAOMMOnewEditor Win64 Development` → `Result: Succeeded` (Target up to date).
- `Scripts/RunAutomation.ps1 -SkipBuild`: Stufe 1 List GRÜN (17 SAOMMOnew.-Zeilen), Stufe 2 RunTests GRÜN
  (found=12, Success=12, Fail=0), Stufe 3 Boot-Smoke GRÜN (Bringing World + Engine Initialization),
  Stufe 4 Log-Scan GRÜN (0 Verstöße). Gesamturteil GRÜN.
- Tests: `SAOMMOnew/Source/SAOMMOnew/Tests/` (9 Dateien: Boot, Checkpoint, Combat, EnemySpawner, Inventory,
  KillReward, Save, VRInput + TestWorld).

## Aktive Systeme (Code-verdrahtet, lesend geprüft)
- Desktop: `Character/PlayerCharacter` (CameraBoom + FollowCamera, `SetCameraMode` FP/TP, V-Taste),
  `Core/MainPlayerController` (Enhanced Input, Fallbacks `/Game/Input/IA_*`), `Core/MainGameMode`.
  Template-Pfade (`Variant_*`, `Legacy*`) sind vorhanden, aber nicht verdrahtet.
- Kampf: `Combat/Sword` (Aktor, wird vom Pawn gespawnt), `Combat/CombatComponent`
  (Armed-Fenster + Overlap-Hit → `IDamageable`, FloatingCombatText), `Core/CombatInterfaces`.
  Block/Parry: kein aktiver Code — fehlt.
- Gegner: `Enemies/EnemySpawner` (MaxAlive/SpawnInterval/SpawnRadius), `Enemies/Enemy`
  (Tick-FSM Idle/Detect/Approach/Attack/Recover, Distanz-Detect, XP/Loot). Kein NavMesh, keine
  Perception, kein Behavior Tree im aktiven Pfad (nur in `Variant_*`-Templates vorhanden).
- Items/Inventar: `Inventory/InventoryComponent`, `Core/ItemTypes`, `World/ItemPickup` + `TryPickup`,
  `Interaction/InteractionComponent`, `UI/PlayerHudWidget` (Code-HUD). Equipment/Container-System fehlt
  (`SAOCharacterData` nur Creator-Save, kein Equip).
- XP/Save: `Progression/ProgressionComponent` (AddExperience/LevelUp, XP-Grant in `Enemy.cpp`),
  `Core/PlayerSaveGame` (Slot-Save/Load im Controller). Zweites Save-Schema `SAOCharacterData`
  (Slot `CharacterSave`) nur Creator-Pfad.
- Tod/Restart: `UI/DeathScreenWidget` + Controller (`RespawnDelay` 15 s, Any-Key + Timer-Fallback),
  `DoRespawn` wählt per `IsXRSessionActive()` zwischen Desktop- und VR-Pawn (frühere VR-Respawn-Lücke geschlossen).
- VR: `VR/VRCharacter` (VROrigin/Camera/2 MotionController, Schwert an rechter Hand),
  `Input/InputFrameComponent` + `FInputFrame`-Abstraktion, OpenXR-Plugins aktiv. Nur Code-verdrahtet —
  Headset-Nachweis steht aus.
- NPC/Quest/Netzwerk: nichts aktiv verdrahtet (nur `EItemType::Quest`-Enum + Template-NPC in
  `Variant_SideScrolling`). Alle Live-Components `SetIsReplicatedByDefault(false)`, keine ServerRPCs.

## Bekannte Lücken / Blocker
- D01–D30 (Vision + Lückenregister V2): keine Freigabe erteilt → neue Spielregeln BLOCKIERT.
- Laufzeitnachweise (Sehen/Spielen: Desktopstart, FP/TP-Wechsel, Kampf, Tod, VR mit Headset): TEST AUSSTEHEND —
  siehe Morgen-Checkliste unten.
- `LOCAL-SETUP.md`-Branchzeile auf `main` korrigiert (24b8cb4); XR-Gate verlangt jetzt HMD-verbunden (Branch fix/xr-gate-requires-hmd).

## S04-Desktop-Audit (A01-Bestand 2026-10-07, Branch docs/s04-desktop-audit)
- P01 Bewegung: verdrahtet (`PlayerCharacter::OnMove`, IA_Move + WASD-Fallback, Walk 400 / Sprint 650).
- P02 First-Person: verdrahtet (FollowCamera an Capsule, EyeHeight 70, OwnerNoSee-Mesh, Schwert sichtbar).
- P03 Third-Person: verdrahtet (CameraBoom/SpringArm 300, Collision-Test an).
- P04 Kamerawechsel: verdrahtet (`SetCameraMode`, V-Taste, ohne Pawnwechsel).
- P05 UI-Fokus: LÜCKE — kein Pause-Menü, keine GameAndUI-Trennung, Tod ändert InputMode nicht. Kein Neubau ohne D-Entscheidung.
- P06 Interaktion: verdrahtet (Eye-Trace 350 cm, E-Taste, Pickup, HUD-Fokus).
- P07 Tod/Restart: Guard gegen doppelte Death-Screen-Auslösung umgesetzt (Branch fix/death-double-trigger-guard).
- P08 Desktop-Kette: Laufzeitnachweis = Spieltest (Boot-Logs: `pawn=PlayerCharacter_0` belegt).

## S06-Kampf-Audit (A01-Bestand 2026-10-07, Branch docs/s06-combat-audit)
- P01 Waffenbesitz: lückenhaft aber funktionsfähig (SetSword/DropPhysics/Lifespan da; Spawn/Attach extern).
- P02 Angriffsfenster: verdrahtet (Armed 0,25 s, Kanten-Trigger Desktop / Velocity VR).
- P03 Treffererkennung: verdrahtet, einfach (Overlap-Sphere 130 cm, kein Sweep; BladeCollision ungenutzt).
- P04 Trefferbegrenzung: wirksam per Zeit-Cooldown (0,4 s); `ASword::LastHitTime` ist ungenutzter Tot-Code (liegen lassen).
- P05 Damagevalidierung: Guards überall; `Enemy::ApplyHealing`-Lücke geschlossen (Branch fix/enemy-heal-guard).
- P06 Abwehrregel (Block/Parry): FEHLT komplett — BLOCKIERT, braucht D14-Freigabe, nichts erfunden.
- P07 Todesabschluss: verdrahtet (Gegner bDead-Guard + Ragdoll + XP/Loot; Spieler Death-Screen + Respawn).
- P08 Desktop-VR-Vertrag: gleicher Combat-Pfad beidseitig (InputFrame + CombatComponent).

## S07-Gegner-Audit (A01-Bestand 2026-10-07, Branch docs/s07-enemy-audit)
- P01 Spawn: verdrahtet (MaxAlive 3, Intervall 5 s, Radius 500 cm, Sofortwelle). Kein Gesamt-Limit — Prototyp-ok.
- P02 Erkennung: Distanz-only (1000 cm, kein Sicht-Check) — bewusste Prototyp-Vereinfachung, kein Bug.
- P03 Navigation: direkt per MovementInput, kein NavMesh/BT — bewusste Prototyp-Vereinfachung.
- P04 Angriff: verdrahtet (120 cm, Schaden + PushAway).
- P05 Erholung: verdrahtet (Recover 1,0 s + Cooldown).
- P06 Zielverlust: verdrahtet (Leash 2x DetectRange, Rückkehr bei >600 cm). Kein Such-Timer — ok.
- P07 Entfernung: verdrahtet (bDead-Guard, Ragdoll, LifeSpan 2 s, OnDied-Prune im Spawner).
- P08 Gruppe: ein EnemyClass, keine Wellen/Varianten — Inhaltsentscheidung, kein Codebau ohne Freigabe.
- Ergebnis: kein Codeeingriff; NavMesh/Perception/Wellen sind Ausbau, keine Fehler.

## S08-Interaktion-Audit (A01-Bestand 2026-10-07, Branch docs/s08-interaction-audit)
- P01 Vertrag: per Component + Delegate + Pickup-Cast verdrahtet (kein UInterface — ok).
- P02 Reichweite/Sicht: verdrahtet (350 cm, Visibility-Trace).
- P03 Fokusrückmeldung: verdrahtet (HUD-Fokustext + Debug).
- P04 Aufheben: verdrahtet (TryPickup + Overlap + Destroy).
- P05 Ablegen: FEHLT (nur RemoveItem, kein Drop-Spawning) — BLOCKIERT, braucht D13-Freigabe.
- P06 Türen: FEHLT (kein Tür-Aktor) — BLOCKIERT, braucht Inhaltsfreigabe.
- P07 Truhen: FEHLT (nur Kosmetik-UI) — BLOCKIERT, braucht D13/D17-Freigabe.
- P08 Doppel-Guard: kein Cooldown — als bekannte Kleinigkeit vermerkt, kein Eingriff ohne Vertrag.
- Ergebnis: kein Codeeingriff; fehlende Features sind Entscheidungen, keine Bugs.

## S09-Beute-Audit (A01-Bestand 2026-10-07, Branch docs/s09-loot-audit)
- P01 Itemdefinition: vorhanden (`FInventoryItem`: Id/Name/Typ/Count).
- P02 Instanz-ID: FEHLT (nur Def-ID, Stacks) — BLOCKIERT, braucht D13-Freigabe.
- P03 Kategorien: verdrahtet (Weapon/Armor/Consumable/Material/Quest).
- P04 Beutetabelle: FEHLT (nur Einzel-Slot LootItemId + Count + XP) — BLOCKIERT, braucht D13-Freigabe.
- P05 Loot-Erzeugung: Direkt-Grant an Killer (kein Welt-Spawn) — Designstand, kein Bug.
- P06 Loot-Aufnahme: verdrahtet (Walk-over + Interact).
- P07 Besitzwechsel: simpel verdrahtet (Add/Remove/Set + Events).
- P08 Pickup-Idempotenz: Guard umgesetzt (Branch fix/pickup-double-guard); Enemy-Seite war bereits idempotent.

## S10-Inventar-Audit (A01-Bestand 2026-10-07, Branch docs/s10-inventory-audit)
- P01 Besitz: verdrahtet (Component am Pawn + Change-Events).
- P02/P03 Hinzufügen/Entfernen: verdrahtet (Validierung, Broadcast, Save/Load via SetItems).
- P04 Stapeln: unbegrenzt per ItemId, kein MaxStack/Split — Prototyp-ok.
- P05 Kapazität: FEHLT (kein Slot-/Gewicht-Limit) — BLOCKIERT, braucht D13-Freigabe.
- P06 Ausrüstungsslots: FEHLT (nur gespawntes Schwert, keine Slots) — BLOCKIERT, braucht D13-Freigabe.
- P07 Ausrüstungswirkung: FEHLT (keine Boni) — BLOCKIERT, braucht D13/D15-Freigabe.
- P08 Inventar-UI: verdrahtet (Code-HUD mit Toggle + Liste).
- Ergebnis: kein Codeeingriff.

## S11-Fortschritt-Audit (A01-Bestand 2026-10-07, Branch docs/s11-progression-audit)
- P01 XP-Zustand: verdrahtet (Level/XP, 100 XP pro Level, Save/Load).
- P02 XP-Vergabe: verdrahtet (Kill → AddExperience + Loot an Killer).
- P03 Levelgrenzen: NUR flach 100/Level, kein MaxLevel/keine Kurve — BLOCKIERT, braucht D15-Freigabe.
- P04 Levelaufstieg: Event vorhanden, ohne Subscriber/Effekte — kein Eingriff ohne D15.
- P05 Grundwerte: FEHLEN (keine Attribute) — BLOCKIERT, braucht D15-Freigabe.
- P06 Equipment-Abgleich: FEHLT (keine Skalierung) — BLOCKIERT, braucht D15-Freigabe.
- P07 XP-HUD: verdrahtet (Level/XP-Texte, kein Balken — ok).
- P08 Reward-Replay: ok (bDead-Guard, Tests vorhanden).
- Ergebnis S11: kein Codeeingriff.

## S12-Speichern-Audit (A01-Bestand 2026-10-07, Branch docs/s12-save-audit)
- P01 Saveformat: verdrahtet (PlayerSave: Pos/Rot/HP/Inventar/Level/XP; zweiter Slot CharacterSave für Aussehen).
- P02–P04 Charakter/Inventar/Fortschritt: verdrahtet (Save/Load + Roundtrip-Tests).
- P05 Weltzustand: FEHLT (keine Gegner-/Quest-Daten, nur transienter Respawn-Punkt) — Ausbau, keine Entscheidung.
- P06 Neustart-Laden: nur manuell per Blueprint-Call, kein Auto-Load — Verhaltensänderung nur mit Freigabe.
- P07 Fehler/Korruption: minimal (kein Backup/Checksum/UI) — Robustheit nur mit Freigabe.
- P08 Migration: FEHLT (kein Versionsfeld) — Formatpolitik braucht Entscheidung.
- Ergebnis S12: kein Codeeingriff.

## S13-Permadeath-Audit (A01-Bestand 2026-10-07, Branch docs/s13-permadeath-audit)
- Ist-Stand: Tod = Death-Screen + Respawn (kein Permadeath). Grep nach Permadeath/DeleteCharacter/Corpse/Nachfolger:
  keine Implementierung (nur Gegner-Leichen-Kommentare + Spawner-Test).
- P01–P08 (Todesvertrag, Endgültigkeit, Sperren, Abschluss, Leiche, Item-Übertrag, Nachfolger, Recovery):
  ALLE BLOCKIERT — brauchen D02-Freigabe (Zeitpunkt, Ausnahmen, Disconnect). Nichts erfunden.
- Ergebnis S13: kein Codeeingriff.

## S14-Lager-Audit (A01-Bestand 2026-10-07, Branch docs/s14-storage-audit)
- Ist-Stand: kein Lager-/Vererbungs-Code. Grep nach Stash/Lager/Inherit/Vererb/Storage/Nachfolger:
  keine Implementierung (nur Schwert-Physik-Kommentare).
- Alle Lager-Pakete BLOCKIERT — brauchen D03-Freigabe (konto-/familiengebunden, was vererbt, kein Nachfolger).
- Ergebnis S14: kein Codeeingriff.

## S15-NPC-Audit (A01-Bestand 2026-10-07, Branch docs/s15-npc-audit)
- Ist-Stand: kein NPC-Code im Live-Modul (Grep nach ANPC/Dialog: keine Treffer; nur Template-NPC in Variant_*).
- Alle NPC-Grundlagen-Pakete BLOCKIERT — brauchen Inhalts- + D16-Lorefreigabe. Nichts erfunden.
- Ergebnis S15: kein Codeeingriff.

## S16-Quest-Audit (A01-Bestand 2026-10-07, Branch docs/s16-quest-audit)
- Ist-Stand: kein Quest-Code im Live-Modul (Grep nach QuestObjective/Reward/GiveQuest/AcceptQuest:
  keine Treffer; nur `EItemType::Quest`-Enum-Eintrag).
- Alle Fest-Quest-Pakete BLOCKIERT — brauchen D17-Freigabe (Ziele, Belohnungen, Konsequenzen).
- Ergebnis S16: kein Codeeingriff.

## S17–S20-Online-Audit (A01-Bestand 2026-10-11, Branch docs/s17-s20-online-audit)
- Ist-Stand: kein Netzwerk-Code im Live-Modul. Grep nach ServerRPC/ClientRPC/NetMulticast/
  GetLifetimeReplicatedProps/DedicatedServer: keine Treffer (nur HasAuthority-Erwähnung in VR-Test).
  Alle Live-Components nicht repliziert.
- S17 Netzwerk, S18 Online-Persistenz, S19 PvP/PK, S20 Leichenbeute: ALLE BLOCKIERT —
  brauchen D12 (Server/Hosting), D04 (Leiche), D05 (PK-Regeln). Nichts erfunden.
- Ergebnis: kein Codeeingriff.

## S29-Schloss-Audit (A01-Bestand 2026-10-07, Branch docs/s29-castle-audit)
- Ist-Stand: kein Schloss-Content. Vorhanden nur Test-Arena (`L_StartingReach` + Ruine-Variante);
  im Code keine Schloss-/Etagen-/Boss-Logik (nur Template-Lavaboden in Variant_*).
- Alle Schloss-Pakete BLOCKIERT — brauchen D06-Freigabe (Etagen, Reihenfolge, Solo/Gruppe) und
  D16-Loreabstimmung (Wiederaufbau-/Artefakt-Lore). Kein Weltbau ohne Freigabe.
- Ergebnis S29: kein Codeeingriff.

## Schwach-PC-Betrieb (16 GB RAM, Stand 2026-10-07)
- Projekt-Default: Raytracing aus (`DefaultEngine.ini`, Lumen bleibt an) — Optik bleibt hoch, Kosten runter.
- Zum Spielen: Standalone mit `-nohmd` starten (bis XR-Gate-Fix), Browser + TeamViewer vorher schließen.
- Im Editor: Engine Scalability auf Medium/High statt Epic/Cinematic; nicht Editor + Spiel gleichzeitig.
- Content ist klein (größte Dateien: Mannequin-Sample-Content ~15–20 MB); kein Asset-Umbau nötig.

## Morgen-Checkliste (PIE, `L_StartingReach`)
1. Start → steuerbarer Spieler sichtbar (WASD/Maus, Springen).
2. V-Taste: FP → TP → FP, Position/HP/Ausrüstung unverändert.
3. Angriff: 1 Treffer → HP-Abzug am Gegner; Gegner-Tod → XP + Loot; Mehrfachtreffer prüfen.
4. Schaden bis Tod (Testcharakter): Death-Screen → beliebige Taste → Respawn (15-s-Fallback).
5. Regression: Inventar-HUD (I?), Item-Pickup, Sprint (Shift), Knockback-Stärke, Creator (C).
6. Auffälliges notieren (Death-Screen, Knockback, Creator-UX, Sprint-Speed sind EditAnywhere-tunbar).

## Verifikation 2026-10-11 — Paketstart-Korrektur (Arbeitszweig `docs/s29-castle-audit`)
- Aktuelle Startkarte in `DefaultEngine.ini`: `/Game/Levels/StartingReach/L_StartingReach`; `MainGameMode` bleibt aktiv. `L_MainMenu` bleibt als unfertige optionale Karte erhalten.
- Ursache des schwarzen/untauglichen alten Pakets eingegrenzt: das am 12:14 erzeugte Paket startete `L_MainMenu`; ein expliziter Paketstart von `L_StartingReach` scheiterte mit `Failed to load package`. Die dynamisch per `OpenLevel` geöffnete Karte war nicht verlässlich in der Cook-Liste.
- `DefaultGame.ini` enthält jetzt explizite `MapsToCook`-Einträge für `L_StartingReach` und `L_MainMenu`.
- RunAutomation 20261011-124533: Stufe 0 Build ROT (Exit 6: Live Coding aktiv, Unreal Editor bleibt offen); Stufe 1 23 Tests gefunden; Stufe 2 im vorhandenen kompilierten Modul 12/12 erfolgreich; Stufe 3 Source-Boot-Smoke lädt `L_StartingReach` und spawnt `PlayerCharacter_0`; Stufe 4 Log-Scan GRÜN (8 bekannte/whitelistete Enginefehler, 0 Verstöße). Die in dieser Runde geänderte Testquelle konnte wegen der Live-Coding-Sperre nicht neu kompiliert werden.
- UAT `BuildCookRun` mit `-nocompile` erfolgreich: Cook/Stage/Archive, 621 Pakete, separates Testpaket unter `%LOCALAPPDATA%\Temp\saommo-playable-build-20261011\Windows`. Neuer Paketstart ohne Map-Override lädt `L_StartingReach`, aktiviert `MainGameMode` und spawnt `PlayerCharacter` (HUD first tick); kein Map-load-Fehler. Der Build verwendet bestehende C++-Binaries, kein frischer C++-Build.
- Maus-/Tastaturbewegung im sichtbaren Fenster wurde nicht interaktiv verifiziert; VR-Headset weiterhin nicht verbunden. Unreal Editor bleibt offen. Das ältere Spielpaket und die zuvor laufende Instanz wurden nicht überschrieben.
- Für fehlende Festlegungen gilt seit Nutzerauftrag 2026-10-11: risikoarme, übliche und reversible Prototyp-Defaults wählen und sichtbar dokumentieren; ausdrücklich genannte Vision und festgelegte Lore bleiben maßgeblich.

## S04-P01 Desktop-Bewegung — Bestandsaudit und Vertrag (2026-10-11)
- A01 Bestand: `APlayerCharacter::OnMove` reads `IA_Move` as `FVector2D`, projects against controller yaw, and calls `AddMovementInput` for forward/right. `AMainPlayerController::BuildFallbackMapping` maps W/A/S/D and `Mouse2D`; `APlayerCharacter` builds a LeftShift sprint mapping. `WalkSpeed=400 cm/s`, `SprintSpeed=650 cm/s`; jump uses Space. `/Game/Input/IA_Move` and related action assets exist.
- A02 Standardvertrag (Nutzerauftrag 2026-10-11): WASD moves relative to camera yaw; release stops; diagonal input is normalized; no speed penalty for diagonal keys; desktop baseline remains 400 cm/s and Shift sprint 650 cm/s. UE 5.8.3 `UCharacterMovementComponent::ScaleInputAcceleration` clamps input acceleration to length 1, so diagonals do not exceed straight-line acceleration.
- A03/A04: no production movement code change needed for normal or diagonal movement; implementation and engine clamp already match the contract. Frame time is handled by CharacterMovement. These are code/source findings, not a manual feel test.
- A05/A06: current Automation suite has 12 tests, but none directly asserts desktop `APlayerCharacter::OnMove` in a live local-player session. Package smoke proves map + pawn startup only. Manual WASD/mouse/diagonal/stop testing remains OPEN; VR still requires headset.

## S04-P02–P08 Desktop-Bestandsaudit (A01, 2026-10-11)
- First person: `APlayerCharacter::UpdateCameraAttachment` attaches camera to capsule at `FirstPersonEyeHeight`, enables control rotation, hides own mesh/shadows; sword remains a separate actor. Code vorhanden, no automated view/occlusion test, manual view pending.
- Third person: camera boom is attached to root, `bUsePawnControlRotation=true`, target arm length is set from `ThirdPersonBoomLength`, collision test is on. Code vorhanden; wall-behind-player behavior needs PIE proof.
- Camera switch: `V` maps to `ToggleCameraAction`; handler changes only `CameraMode` and camera attachment, no pawn/level recreate. Test for transition during attack and state preservation is missing.
- UI focus: gameplay BeginPlay sets `FInputModeGameOnly` and hides cursor; main menu/creator overlays use `FInputModeUIOnly`. Source code supports expected routing; live mouse capture/held-key behavior not verified.
- Interaction: `E` and `I` currently map to the same `InteractAction`; when the trace has no target, `OnInteract` toggles inventory. This differs from common controls (E interact, I inventory) and should be split in a scoped follow-up after a compilable Live Coding build is available. Do not claim interaction tested by package boot.
- Death/restart: pawn destruction raises death screen and arms the configured timer; any pressed key respawns immediately. No Permadeath behavior is implemented by this existing path. Automation has checkpoint/respawn coverage; visible overlay/input path still needs PIE.
- Desktop chain: code paths exist across start, pawn, movement, camera, interaction and death, but no uninterrupted manual pass through the playable package is proven. Treat as implemented/code-audited, not finished runtime.

## Nutzerentscheidungen — Entwicklungsrichtung (2026-10-11)
- Maßgebliche Grundlage: Nutzertext „SAOMMO – Grundlegende Entscheidungen für die Entwicklung“ (Anhang vom 2026-10-11). Er bestätigt dieses Repository, Unreal Engine 5.8 und C++ als bevorzugte Sprache; die vorhandenen Repo-Regeln und geprüften Entscheidungen bleiben vorrangig.
- Produktziel: zuerst ein stabiles, kleines Desktop-Fantasy-Action-RPG mit Schwertkampf. Später schrittweise VR-fähiges Online-RPG mit persistenter Welt und MMO-Ausbau. Multiplayer bleibt Architekturziel, ist kein Vorwand für einen vorzeitigen Serverumbau.
- Erste spielbare Version: steuerbarer Charakter, Bewegung/Kamera, nachvollziehbarer Schwertkampf, mindestens ein Gegner, Schaden/Tod/Respawn nach vorläufigen Testregeln, Beute/einfache Ausrüstung, ein NPC mit Dialog, Speichern/Laden und zusammenhängendes Testgebiet. Das ist der Zielumfang, kein bereits belegter Implementierungsstand.
- Reihenfolge: zuerst Desktop-Stabilität und die bereits vorhandenen Systeme verifizieren; danach fehlende, abhängige Bausteine in kleinen Schritten ergänzen. VR wird architektonisch berücksichtigt und mit vorhandener Quest 3 geprüft, sobald Desktop-Grundlagen stabil sind. Budget-VR und große MMO-Systeme folgen später.
- Kampfrichtung: direkt und actionreich, aber anspruchsvoll; Positionierung, Timing, Ausweichen und lesbare Gegnerangriffe sollen zählen. Konkrete Kampfwerte bleiben tunbar und werden anhand Spieltests angepasst.
- Welt: eigenständige Fantasywelt um die Ruinen einer untergegangenen Schwertmeister-Kultur; keine Übernahme geschützter SAO-Figuren, Namen oder Handlung. „Echoes of Aincrad“, „Alicization Lycoris“ und „Breath of the Wild“ sind Stil-/Erkundungsreferenzen, keine zu kopierenden Assets oder Designs.
- Hardwareziel für die erste Entwicklungsrunde: vorhandener PC (RTX 4070, Ryzen 7 7700X, 32 GB RAM) und Meta Quest 3. Diese Daten definieren keine Mindestanforderung für spätere Spieler.
- Permadeath: als wichtiges langfristiges Spielelement bestätigt, aber die im Nutzertext genannten Normalwelt-/Hochrisikoregeln sind ausdrücklich Vorschläge. Bis zu einer konkreten Spielregel bleibt der aktuelle Tod/Respawn-Prototyp für Tests bestehen; keine endgültige Löschung von Charakteren oder Besitz implementieren.
- Schloss: ein späterer erster Abschnitt soll Erkundung, Kampf, NPCs und Boss verbinden; Sieg soll Fortschritt freischalten und Weltreaktionen auslösen. Detaillierte Etagenstruktur, Lore und Bossregeln bleiben an die bestehenden Abhängigkeiten gebunden; keine Schlosskampagne vor den nötigen Grundlagen erfinden.
- Arbeitsweise: kleine, nachvollziehbare Schritte; vorhandenen Stand vor Änderungen prüfen; Build, Tests und Laufzeit getrennt belegen; Fehler/Schulden dokumentieren. Nutzer hat am 2026-10-11 zusätzlich übliche, risikoarme und reversible Standards für nicht beschriebene Bedienungsdetails autorisiert. Diese Erlaubnis überschreibt keine ausdrücklich offene Kernregel wie Permadeath.

## Laufzeitprüfung am PC — Input/Start (2026-10-11, direkt auf Nutzer-PC)
- Das ältere Paket aus `Saved/Packaged/Windows` zeigt ein schwarzes Fenster. Sein Log lädt `L_MainMenu`, erstellt `UMainMenuWidget` und setzt UI-only; der Viewport meldet `MouseCaptureMode=NoCapture`. Der Menüpfad ist dadurch nicht als spielbarer Start bestätigt. Die laufende ältere Instanz blieb offen.
- Das separate Paket unter `%LOCALAPPDATA%\Temp\saommo-playable-build-20261011\Windows` startet sichtbar in `L_StartingReach`; Spieler und Level werden gerendert. Dort reagierten `V` (FP/TP-Wechsel), Leertaste (Sprung) und Mausbewegung (Kameradrehung). Log: `CapturePermanently`, `HUD first tick`, Pawn `PlayerCharacter`. Damit ist sichtbarer Start und ein Teil der Eingabe live belegt.
- Im Editor-PIE ließ sich die Map starten; Unreal zeigte die Einblendung `Umschalt+F1 für den Mauszeiger`. Mausziehen änderte die Kamerarichtung. Der PIE-Lauf wurde danach regulär gestoppt; der Editor blieb offen.
- WASD-Dauerbewegung konnte mit der verfügbaren UI-Eingabe nicht zweifelsfrei bestätigt werden: kurze synthetische Tastendrücke sind kein gehaltener Tastendruck. Dies bleibt offen und ist weder als bestanden noch als Codefehler gewertet.
- Während Editor + zwei Spielinstanzen lieferten Windows und Unreal eine Speicherdruckwarnung. Windows meldete etwa 16 GB sichtbaren RAM und kurzfristig unter 1 GB frei (abweichend von der Master-Datei mit 32 GB). Nach regulärem Schließen der von Codex gestarteten Testinstanz waren etwa 4 GB frei; Editor und ursprüngliche Spielinstanz blieben offen. Weitere parallele Instanzen vermeiden.
- Schlussfolgerung: der ursprüngliche „Mauszeiger bleibt stehen“-Eindruck kann teilweise durch das alte Menüfenster (UI-only/no-capture) und die alte Startkarte erklärt sein. Das gameplay-startende Paket erfasst Maus/Kamera. Vollständige Bewegungs- und Desktop-Spieltests stehen weiterhin aus.

- 2026-10-11: APlayerCharacter::OnMove/OnLook now apply actual input whenever a controller exists; UInputFrameComponent is only updated when present. This removes a needless failure dependency. Source change is not yet compiled or runtime-tested because Unreal Editor/Live Coding remains open and blocks the normal build. WASD held movement remains unverified.

## Blender-Weltmodelle (2026-10-11)
- In `ArtSource/Blender/SAOMMO_World_Props.blend` sind acht editierbare, stilisierte Low-Poly-Props angelegt: Ruinenbogen, Checkpoint-Wegstein, Schwertdenkmal, gebrochene Säule, Brunnen, Kohlebecken, Vorratskiste und Klingenhofbank.
- Je Modell liegt ein einzelner FBX-Export in `ArtSource/Blender/Exports/FBX/`; Galerie-Render: `ArtSource/Blender/renders/SAOMMO_world_props.png`; Blender-Erzeugungsskript und Importhinweise liegen daneben.
- Blender 5.2.1 hat alle acht FBX-Exporte und die Galerie erfolgreich erzeugt. Noch nicht in Unreal importiert; keine Unreal-Asset-Import- oder Laufzeitprüfung erfolgt. Material-Slots sind enthalten, finale Unreal-Materialien, Texturen, Collision und Niagara-Feuer fehlen.

## Modularer Weltbaukasten (2026-10-11)
- ArtSource/Blender/SAOMMO_World_Builder_Kit.blend enthält 36 getrennte, editierbare Low-Poly-Meshes mit 4-m-Rasterbauteilen, Boden/Wegen, Ruinen-/Gebäudeteilen, Holzbau und Naturobjekten.
- 36 separate FBX-Dateien unter ArtSource/Blender/Exports/World_Builder_Kit/FBX/; Katalogbild und Importanleitung sind unter ArtSource/Blender/renders/SAOMMO_world_builder_kit.png bzw. im Exportordner.
- Blender 5.2.1 hat die Galerie gerendert und 36 FBX-Dateien exportiert. Noch nicht in Unreal importiert oder dort geprüft; Materialinstanzen, Kollision, LODs und Terrain-System bleiben Folgearbeit.

## Starting Reach — Blender-Weltübersicht (2026-10-11)
- `ArtSource/Blender/SAOMMO_Starting_Reach.blend` setzt die vier dokumentierten Slice-Zonen als eine editierbare, zusammenhängende Szene um: Brunnfeld (50×50 m), Klingenhof (30×30 m), Grauwaldrand (80×80 m) und Ruine des Ersten Griffs (40×60 m); Wege und Torbogen verbinden die Bereiche.
- Die Szene verwendet vorhandene Kit-Meshes plus Blockout-Geometrie für Brunnen/Siedlungswiederaufbau, vier Trainingspuppen, Wald-/Deckungsruinen und drei Ruinenräume. Der Nischen-Leuchteffekt zeigt nur einen versiegelten Artefakt-Teaser; keine Belohnung oder neue Boss-/Quest-Lore wurde erfunden.
- Vorschau: `ArtSource/Blender/renders/SAOMMO_Starting_Reach.png`. Reproduzierbares Blender-Skript: `ArtSource/Blender/create_starting_reach_world.py`. FBX-Blockout: `ArtSource/Blender/Exports/Starting_Reach/SM_StartingReach_Blockout.fbx` (Meshes einzeln benannt, Unreal-Zentimeter).
- Blender 5.2.1 speichert und rendert die Szene; FBX-Export erfolgt aus derselben Szene. Noch nicht in Unreal importiert, auf Kollision/Navmesh geprüft oder spielbar verifiziert. Es ist eine präsentierbare Blockout-Weltübersicht und kein fertiges MMO-/Open-World-Gebiet.

## Weltmodelle — Fantasy-Stilvariante (2026-10-11)
- Neue, rückwärtskompatible Variante unter `ArtSource/Blender/Styled_Assets/`: 36 Weltbaukasten-Meshes, 8 Props/Landmarken und eine neu gerenderte Starting-Reach-Szene. Die bisherigen Blender-Dateien bleiben unverändert.
- Die Style-Richtung nutzt allgemeine Merkmale der Referenz (lebendige Fantasy-Landschaften, Städte, Wald und Ruinen), ohne konkrete Spielmodelle zu übernehmen: wärmere Natursteine, sattere Vegetation, Eichenholz, kühles Metall und prozedurale Oberflächenvariation.
- Blender 5.2.1 hat alle drei `.blend`-Szenen und Vorschauen erzeugt. FBX-Exports: 36 Einzelassets, 8 Einzelprops sowie ein 173-Mesh-Blockout der Region. `ArtSource/Blender/Styled_Assets/README.md` dokumentiert Dateipfade und Grenzen.
- Die prozeduralen Shaderdetails sind in Blender sichtbar, aber nicht in FBX gebacken. Unreal benötigt eigene Materialinstanzen oder gebackene Texturen. Import, Kollision, LOD, Performance und Spielbarkeit bleiben ungeprüft.
