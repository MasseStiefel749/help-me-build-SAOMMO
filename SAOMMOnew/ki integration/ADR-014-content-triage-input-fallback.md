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

## Audit addendum (2026-09-25, Dauerbetrieb R19/R20 — backlog #29b)

Fresh headless load probe (`UnrealEditor-Cmd -run=pythonscript`, evidence
`Saved/BpLoadProbe.txt`, `ok=True`, 15 paths) re-confirms the quarantine
and adds facts:

- **All 10 SAO blueprint copies still fail to load** ("exists but was not
  able to be loaded"): the root copies `BP_SAOEnemy`,
  `BP_SAOMMOCharacter`, `BP_SAOMMOGameMode`, `BP_SAOMMOPlayerController`,
  `BP_SAOSword` **and** their `Enemies/`, `Blueprints/`, `Weapons/`
  folder twins. Template BPs (`BP_ThirdPersonCharacter`, `BP_JumpPad`)
  and `IMC_Default` load fine — the damage is specific to the SAO-era
  files, exactly as diagnosed above.
- `IMC_SAOMMO` still fails to load and has **zero** external references
  (string scan over every uasset/umap). Decision stands: **rely on the
  transient `BuildFallbackMapping()`**; do not bind `IMC_SAOMMO` until it
  is repaired in a GUI session (see "Still needs GUI"). The redirect
  target name `IMC_Player` never existed on disk — the rename was
  reverted (Widerspruchstagebuch item 13).
- Two live `IA_*` sets exist on purpose and must **not** be deduplicated:
  `Content/Input/IA_*` (C++ `FObjectFinder` defaults in
  `PlayerCharacter`/`MainPlayerController`/`VRCharacter`) and
  `Content/Input/Actions/IA_*` (bound by `IMC_Default`, used by the
  ThirdPerson template controller).
- `L_StartingReach_Ruine.umap` still overrides its GameMode with the
  load-broken `Blueprints/BP_SAOMMOGameMode` (silently falls back to the
  native `MainGameMode` global — harmless today because native is the
  intended mode). Cleanup decision: backlog #26 (needs owner approval;
  ADR-014a quarantine stands until then).
