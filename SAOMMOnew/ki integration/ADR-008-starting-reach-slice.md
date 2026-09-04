# SAOMMO — ADR-008: Starting Reach Slice Activation

**Date:** 2026-09-02
**Status:** Active
**Builds on:** ADR-005/006/007
**Scope:** Level + project wiring (no C++ API change)

## Context

Phases 1-3 created the isolated target-architecture systems. The remaining blockers (ADR-005e follow-ups) were editor wiring: GameMode/level, Input, plugin pollution, and the test arena itself.

## Decisions

### ADR-008a — Level exists

- `Content/Levels/StartingReach/L_StartingReach.umap` created (copied from `Lvl_ThirdPerson` as valid binary, to be rebuilt as Starting Reach blockout per `test-arena-spec.md`).
- `L_StartingReach_Ruine.umap` reserved as dungeon sub-level (Level Instance).
- `Content/Levels/StartingReach/README.md` and `Content/World/StartingReach/README.md` define zones, sizes, landmarks, enemy counts.

### ADR-008b — Project maps now point to slice

- `Config/DefaultEngine.ini:1` `GameDefaultMap` / `EditorStartupMap` → `/Game/Levels/StartingReach/L_StartingReach`
- `GlobalDefaultGameMode` → `/Game/Blueprints/BP_SAOMMOGameMode.BP_SAOMMOGameMode_C`
- Revert by restoring `DefaultEngine.ini` to `Lvl_ThirdPerson` if slice breaks.

### ADR-008c — Plugin discipline applied

- `SAOMMOnew.uproject` replaced with minimal 28-plugin set (Band 9 Ch.5 keep list). Full backup `SAOMMOnew.uproject.full.bak` (595 enabled → 28). Restore via copy-back if a required plugin was missed.
- Explicitly enabled: `OpenXR*`, `EnhancedInput*`, `GameplayTags`, `StateTree/GameplayStateTree`, `AIModuleToolset`, `ControlRig/FBIK/IKRig/IKRetargeter/MotionWarping/Matching/PoseSearch`, `Niagara/PCG/Water/Landmass`, `CommonUI`, `ModelingToolsEditorMode`, plus `Iris`/`EOS` for future multiplayer.
- Deferred marketplace `VRM4U/Nwiro/SteamSAL` not in minimal — re-enable if needed.

### ADR-008d — C++ fixes included

- `ASAOMMOCharacter` sword attach now prefers `hand_rSocket/hand_r` (was root-only).
- `ASAOMMOCharacter::UpdateCameraAttachment` now uses `AttachToComponent` (was `SetupAttachment` at runtime, broke FP/TP switch).
- `ASAOMMOPlayerController` caches `RespawnTransform` on `BeginPlay/OnPossess` (was 0,0,0).

## Verification (must pass in Editor)

- [ ] Editor restarts cleanly on 5.8 with minimal plugins, no missing module errors
- [ ] PIE on `L_StartingReach` spawns `BP_SAOMMOCharacter` via `BP_SAOMMOGameMode`
- [ ] IMC_SAOMMO drives `FSAOInputFrame` (WASD/Look/Jump/Attack/ToggleCamera)
- [ ] Sword on hand socket, swing arms 0.25s, hits `BP_SAOEnemy` (3 hits kill)
- [ ] Death → respawn at `RespawnTransform` (not origin)
- [ ] Full revert path tested: `Copy SAOMMOnew.uproject.full.bak → SAOMMOnew.uproject`

## Remaining (requires Editor hands-on)

- Blockout geometry for Brunnfeld/Klingenhof/Grauwaldrand/Ruine per `Content/Levels/StartingReach/README.md`
- NavMeshBoundsVolume, PlayerStarts, enemy spawns, sealed niche
- Verify `BP_SAOMMOPlayerController.DefaultMappingContexts=[IMC_SAOMMO]` in Editor (see `WIRING_GUIDE_SAOMMO_5.8.md`)
