# SAOMMO — ADR-009: UE 5.8 Build Fix (UBT Clean)

**Date:** 2026-09-03
**Status:** Active
**Builds on:** ADR-008
**Bands:** B1 §8 (UE 5.8.1), B2 §0.1 version policy, B6 §22 plugin discipline + Task workflow, B9 Ch.5 plugin list

## Context

`SAOMMOnewEditor` UBT failed at makefile stage:
`Unable to find plugin 'GameplayTags' (referenced via SAOMMOnew.uproject)`.
After plugin fix, 1 C++ error remained in `Source/SAOMMOnew/Core/SAOMMOPlayerController.cpp:23` (C2248).

## Decisions

### ADR-009a — Remove invalid plugin entries (UBT-blocking)

File: `SAOMMOnew.uproject`
Removed 7 entries (28 → 21):
- `GameplayTags` — no `.uplugin` in UE 5.8. Gameplay Tags is a runtime **module**, already in `SAOMMOnew.Build.cs:19` (`GameplayTags`). B9 Ch.5 "Gameplay Tags" intent preserved via module dep, not plugin entry.
- `IKRetargeter`, `MotionMatching` — no `.uplugin` in UE 5.8 (`IKRig.uplugin`, `PoseSearch.uplugin` cover retarget + motion matching; `MotionTrajectory` is the 5.8 motion path). B9 animation goal kept via `IKRig/ControlRig/FullBodyIK/MotionWarping/PoseSearch`.
- `Iris`, `OnlineSubsystemEOS`, `OnlineServicesEOS`, `EOSVoiceChat` — deferred per B1 §6 + B2 (MMO/multiplayer later, single-player slice first). No `[OnlineSubsystem]` config + no Build.cs deps existed, so packaged/net builds would fall back to NULL. Re-enable with config + Build.cs deps when multiplayer phase starts.

Kept: OpenXR x5, EnhancedInput x2, StateTree/GameplayStateTree, AIModuleToolset, ControlRig/FullBodyIK/IKRig/MotionWarping/PoseSearch, Niagara/PCG/Water/Landmass, ModelingToolsEditorMode (Editor-only), CommonUI.

### ADR-009b — PlayerController respawn fix (C2248)

File: `Source/SAOMMOnew/Core/SAOMMOPlayerController.cpp:17-26`
`AController::GetActorTransform()` is private in 5.8 (`HIDE_ACTOR_TRANSFORM_FUNCTIONS()` in `Controller.h:408`).
Replaced controller-transform fallback with `PlayerCameraManager` camera transform; `OnPossess()` still overwrites with pawn transform on first spawn. No API change.

### ADR-009c — Stale template config

- `Config/DefaultGame.ini:3` `ProjectName=Third Person Game Template` → `SAOMMOnew`
- `Config/DefaultEditor.ini:2` `SimpleMapName=/Game/ThirdPersonCPP/...` → `/Game/Levels/StartingReach/L_StartingReach` (matches `DefaultEngine.ini:2-3`, verified `L_StartingReach.umap` exists)

`Source/SAOMMOnew/SAOMMOnew.Build.cs`, `Source/SAOMMOnew.Target.cs`, `Source/SAOMMOnewEditor.Target.cs` reviewed — no changes needed (V7 + `Unreal5_8`, deps correct, EQS via AIModule comment accurate for 5.8).

## Verify

- `UnrealBuildTool.exe SAOMMOnewEditor Win64 Development -Project=.../SAOMMOnew.uproject` → **Succeeded** (7.05s incremental, 4 actions)
- `UnrealBuildTool.exe SAOMMOnew Win64 Development -Project=...` → **Succeeded**, output `Binaries/Win64/SAOMMOnew.exe`
- UHT clean, 0 errors, 1 benign warning (XGE license standalone mode)

## Revert

- Restore `SAOMMOnew.uproject` from git (`git diff SAOMMOnew.uproject`) to get back EOS/Iris if needed
- B9 Ch.5 deviations above are UE 5.8 renames/removals, not scope cuts
