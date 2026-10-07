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
- `LOCAL-SETUP.md` Zeile 4 nennt Branch `going-to-make-an-project`; bindend ist `main` (AGENTS.md) — Fix als eigene Reparatur.

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
