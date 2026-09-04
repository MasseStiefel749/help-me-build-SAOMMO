# SAOMMO — ADR-012: Prefix-Free Naming (No SAO Outside Descriptions)

**Date:** 2026-09-04
**Status:** Active
**Scope:** All C++ identifiers, source filenames, content asset filenames,
editor-visible categories, save slot. NOT renamed: module/project identity
(`SAOMMOnew`), prose comments/tooltips/docs, redirect old-names.

## Context

Owner request: better naming, no `SAO`/`SAOMMO` in data except
descriptions. Full scope incl. Content, no replacement prefix (plain
descriptive names). Module/project identity `SAOMMOnew` kept as title-only
per owner decision.

## Decisions

### ADR-012a — Name map (old → new)

| Old | New |
|---|---|
| `ASAOMMOCharacter` | `APlayerCharacter` |
| `ASAOMMOPlayerController` | `AMainPlayerController` |
| `ASAOMMOGameMode` | `AMainGameMode` |
| `ASAOEnemy` | `AEnemy` |
| `ASAOSword` | `ASword` |
| `USAOMMOCombatComponent` | `UCombatComponent` |
| `USAOMMOInventoryComponent` | `UInventoryComponent` |
| `USAOMMOProgressionComponent` | `UProgressionComponent` |
| `USAOMMOInteractionComponent` | `UInteractionComponent` |
| `USAOMMOInputFrameComponent` | `UInputFrameComponent` |
| `USAOMMOHudWidget` | `UPlayerHudWidget` |
| `USAOMMOSaveGame` | `UPlayerSaveGame` |
| `USAOMMOWorldSubsystem` | `UGameWorldSubsystem` |
| `ASAOItemPickup` | `AItemPickup` |
| `ASAOCheckpoint` | `ACheckpoint` |
| `ASAOEnemySpawner` | `AEnemySpawner` |
| `ASAOVRCharacter` | `AVRCharacter` |
| `FSAOInputFrame` | `FInputFrame` |
| `ESAOInputDevice` | `EInputDevice` |
| `ESAOCameraMode` | `ECameraMode` |
| `ESAOEnemyState` | `EEnemyState` |
| `ESAOItemType` | `EItemType` |
| `FSAOItem` | `FInventoryItem` |
| `ISAOCombatAttacker` | `IAttacker` (see 012c) |
| `ISAOCombatDamageable` | `IDamageable` (see 012c) |
| `SAOMMOCoreTypes.h` | `SharedTypes.h` (see 012c) |
| `SAOMMO_Save` slot | `PlayerSave` (old saves orphaned) |
| Legacy `SAOMMOnewCharacter/GameMode/PlayerController` | `LegacyCharacter/GameMode/PlayerController` |
| `LogSAOMMOnew` | `LogGame` |
| `ClassGroup (SAOMMO)` / `Category "SAO Input"` | `(Game)` / `"Input"` |
| `FOnSAOEnemyDied` | `FOnEnemyDefeated` (see 012c) |

Assets: `BP_SAOMMOCharacter`→`BP_PlayerCharacter`,
`BP_SAOMMOGameMode`→`BP_MainGameMode`,
`BP_SAOMMOPlayerController`→`BP_MainPlayerController`,
`BP_SAOEnemy`→`BP_Enemy`, `BP_SAOSword`→`BP_Sword`,
`IMC_SAOMMO`→`IMC_Player` (root + subfolder copies alike).
`DefaultEngine.ini`: `GlobalDefaultGameMode` points at `BP_MainGameMode`.

### ADR-012b — Redirects keep old assets loadable

`Config/DefaultEngine.ini` gained `ActiveClassRedirects` (all renamed
classes incl. legacy `SAOMMOnew*` template targets, which were also
re-pointed), `ActivePackageRedirects` (11 moved assets),
`ActiveStructRedirects` (`FSAOItem`, `FSAOInputFrame`) and
`ActiveEnumRedirects` (4 enums). Old maps/packages referencing previous
names resolve on Editor load; verify in the Editor session from ADR-010.

### ADR-012c — Collisions found during the rename (all fixed)

1. `UCombatAttacker`/`UCombatDamageable` clashed with the legacy
   `Variant_Combat` interfaces (same UHT engine name) → Core contracts are
   now `IAttacker`/`IDamageable` (shorter, no clash).
2. New `CoreTypes.h` was shadowed by the engine's
   `Runtime/Core/Public/CoreTypes.h` (wrong header included, `FInputFrame`
   undefined) → ours is `SharedTypes.h` (verified no engine counterpart).
3. New `FOnEnemyDied` clashed with the legacy `CombatEnemy` delegate →
   ours is `FOnEnemyDefeated`; legacy untouched.

### ADR-012d — Deliberately NOT renamed

- Module/project identity: `SAOMMOnew.uproject`, `Source/SAOMMOnew/`,
  `SAOMMOnew.Build.cs`, `*.Target.cs`, `SAOMMONEW_API`, `ProjectName`,
  `.sln` (regenerate project files in Editor after this change).
- Prose comments, tooltips, band docs, ADRs (descriptions by design).
- Redirect old-names in `DefaultEngine.ini` (load-bearing).
- Strings *inside* Blueprints (binary; needs the Editor session).

## Verify

- Token sweep: 0 identifier-like `SAO` tokens in `Source/`, 0 `SAO`
  content filenames (script `rename_sao.py` + include check).
- `UnrealBuildTool SAOMMOnewEditor Win64 Development` → Succeeded
- `UnrealBuildTool SAOMMOnew Win64 Development` → Succeeded

## Revert

- Single commit; `git revert` restores all names. Note: reverting
  re-breaks any Blueprints saved in the Editor under new names.
