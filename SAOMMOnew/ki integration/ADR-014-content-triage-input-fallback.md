# SAOMMO — ADR-014: Content Triage + Code-Only Input Fallback

**Date:** 2026-09-04
**Status:** Active
**Builds on:** ADR-013
**Bands:** B1 Gameplay First, B3 §5 movement / §6 interaction

## Context

Probe matrix (headless, fresh + stale registry, cmd + GUI-PIE): stock
template assets and `IA_*` load fine, but 11 BPs (`BP_Enemy`,
`BP_Player/ MainGameMode/MainPlayerController/Sword` incl. subfolder
copies) plus `IMC_SAOMMO` fail in every context. Bytes predate this work
(unchanged since entering the repo; `git hash-object` matches history),
headers structurally valid — root cause inside the files, pre-existing
(migration-era damage). The game therefore cannot rely on BP-set input.

## Decisions

### ADR-014a — Broken content quarantined, not deleted

Files stay in place (git history is the backup). Spawners/GameMode use
native classes until the BPs are recreated in the GUI. Redirects
(`ActiveClassRedirects` legacy + `[CoreRedirects]` packages/structs/enums)
stay so old references keep resolving wherever possible. Also fixed: a
glued duplicate redirect line that logged `AddRedirect ... empty name`.

### ADR-014b — Input works with zero content (`IA_Interact` + fallback)

- `Content/Input/IA_Interact.uasset` duplicated headless from `IA_Attack`
  (`duplicate_asset`, saved, loads, Boolean ✓).
- `APlayerCharacter` constructor now `FObjectFinder`-defaults all six
  actions (Move/Look AXIS2D, Jump/Attack/ToggleCamera/Interact Boolean —
  value types verified headless); Blueprint may still override.
- `AMainPlayerController::SetupInputComponent` uses assigned
  `DefaultMappingContexts` when present, else builds a **transient**
  `UInputMappingContext` in C++ (`BuildFallbackMapping`: WASD + mouse +
  Space + LMB + V + E, negate modifiers on S/A/MouseY) and applies it.
  No IMC asset required. `UInputModifierNegate` flags set explicitly per
  key (defaults are all-true; explicit is robust).

## Verify

- `UnrealBuildTool SAOMMOnewEditor + SAOMMOnew Win64 Development` → Succeeded.
- `IA_Interact` loads headless with correct value type.

## Still needs GUI (user session)

- Recreate/repair the 11 BPs (reparent to native classes, reapply the few
  property overrides) OR delete them if native defaults suffice; rebind
  `IMC_Player` or rely on the fallback.
- PIE walkthrough: move/look/jump/V-camera/LMB-attack/E-interact, kill an
  enemy (XP/loot/HUD), die (respawn), checkpoint, save/load.
