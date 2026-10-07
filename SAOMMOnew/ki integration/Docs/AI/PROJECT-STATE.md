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
