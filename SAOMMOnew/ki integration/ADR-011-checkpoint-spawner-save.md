# SAOMMO — ADR-011: Checkpoints, Enemy Spawner, Save/Load Wiring

**Date:** 2026-09-04
**Status:** Active
**Builds on:** ADR-010
**Bands:** B3 §9 (Encounter) / §11 (death/respawn) / §12-§14 (Improve/Save),
B2 §15 (USaveGame first, custom framework only if insufficient)

## Context

Kill rewards and respawn existed, but nothing placed encounters in the
world, no checkpoint moved the respawn point, and `USAOMMOSaveGame` had no
writers/readers. All three blocked a playable Starting Reach pass
(Brunnfeld spawn → Klingenhof → Grauwaldrand kills → Ruine teaser).

## Decisions

### ADR-011a — Checkpoint actor

New `World/SAOCheckpoint.h,.cpp`: sphere volume; on
`NotifyActorBeginOverlap` with an `ASAOMMOCharacter` it writes the
checkpoint transform (location + facing) into the owning
`ASAOMMOPlayerController` via `SetRespawnTransform`. Place at safe spots;
no Blueprint glue needed.

### ADR-011b — Enemy spawner actor

New `Enemies/SAOEnemySpawner.h,.cpp`: `EnemyClass` (default `ASAOEnemy`),
`MaxAlive` (default 3, matches test-arena-spec 2-4), `SpawnInterval`,
`SpawnRadius`, `bAutoStart`. Immediate first wave on `BeginPlay`, timer
top-ups, weak-pointer roster pruned on `OnDied` and before each spawn.
`GetAliveCount()` exposed for HUD/objectives.

### ADR-011c — Save/load wiring (USaveGame, no custom framework)

- `SAOMMOPlayerController::SaveProgress/LoadProgress` (+`SaveSlotName`,
  `GetRespawnTransform`): persists pawn transform, health, inventory items,
  level/XP through `USAOMMOSaveGame` slot `SAOMMO_Save`. Load teleports the
  current pawn, restores state, and adopts the loaded transform as the
  respawn point. All BlueprintCallable for menu wiring.
- Minimal state setters (save/load only): `Inventory::SetItems` (validated
  replace + single broadcast), `Progression::SetProgress` (broadcasts only
  on level change), `Character::SetHealth` (clamped).
- Gotcha: locals named `Pawn`/`Character` shadow `AController` members and
  C4458 is an error here — save/load uses `TargetPawn`/`TargetCharacter`.

## Verify

- `UnrealBuildTool SAOMMOnewEditor Win64 Development` → Succeeded
- `UnrealBuildTool SAOMMOnew Win64 Development` → Succeeded

## Revert

- Additive only (`World/SAOCheckpoint.*`, `Enemies/SAOEnemySpawner.*`,
  setters + PC save/load). No content touched. Revert per file via git.

## Still needs Editor (unchanged from ADR-010)

- Place spawners/checkpoints/pickups in `L_StartingReach`, bind the
  Interact key in `IMC_SAOMMO`, fix the pre-existing package-IO crash
  (open + resave map in 5.8).
