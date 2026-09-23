# SAOMMO — ADR-010: Kill-Reward Loop, Interaction, Combat Fixes, Code HUD

**Date:** 2026-09-04
**Status:** Active
**Builds on:** ADR-009
**Bands:** B2 §4/§8/§10/§11, B3 §6/§7/§12/§13 (Explore→Encounter→Fight→Loot→Improve), B6 scoped-task workflow

## Context

Systems-side C++ was "complete" per ADR-007 but the loop was unwired:
player owned no Inventory/Progression/Interaction, enemy death granted no
XP/loot, combat could hit the wielder and bypassed the damageable contract,
interaction was a one-shot broadcast with no focus, and there was no HUD.

## Decisions

### ADR-010a — Player owns the loop (Character)

`Character/SAOMMOCharacter.h,.cpp`
- New subobjects: `InventoryComponent`, `ProgressionComponent`,
  `InteractionComponent` + Blueprint getters.
- New `InteractAction` input + `OnInteract` binding (was the only core
  action without a bind).
- `TakeDamage` clamps at 0 and ignores non-positive damage; new `Heal()`;
  `Die()` destroys the equipped sword so respawns leave no orphan weapons.
- New `GetHealth()`/`GetMaxHealth()` for HUD polling.

### ADR-010b — Kill rewards (Enemy)

`Enemies/SAOEnemy.h,.cpp`
- New `XPReward` (default 10), `LootItemId`, `LootCount`; `Die()` grants XP
  to the killer pawn's progression component and loot to its inventory.
- Killer tracked via `EventInstigator`, falling back to
  `DamageCauser->GetInstigatorController()` (melee path passes the sword).
- `bDead` guard ends the TakeDamage/ApplyDamage double-death; attack
  cooldown is now `RecoverTime`-based (was `SetTimerForNextTick` = none).
- Added missing `TimerManager.h`, `Controller.h`, progression/inventory includes.

### ADR-010c — Combat correctness

`Combat/SAOMMOCombatComponent.h,.cpp`, `Combat/SAOSword.h`
- Sweep skips the owner and the sword's `OwnerActor`; new `GetOwnerActor()`.
- `SetOwnerActor()` now also calls `SetOwner()` so the sword's instigator
  chain resolves to the wielder (kill credit through the interface path).
- Prefers `ISAOCombatDamageable::ApplyDamage` when implemented, else
  `TakeDamage`; new `OnHit(Target, Applied, Location)` event for damage
  numbers / HUD without touching the sweep.
- `LastHitTime` key changed `TWeakObjectPtr` → `TObjectPtr` (hash guarantee).

### ADR-010d — Interaction focus + pickups

`Interaction/SAOMMOInteractionComponent.h,.cpp`, new `World/SAOItemPickup.h,.cpp`
- Component now ticks and maintains `FocusedActor` (null-guarded world).
- `Interact()` resolves `ASAOItemPickup` directly via `TryPickup(Owner)`.
- Pickup: sphere volume (`Pawn`+`Visibility` overlap), `FSAOItem` payload,
  transfers into caller pawn inventory and destroys itself.

### ADR-010e — Code-only HUD (no Editor widget needed)

New `UI/SAOMMOHudWidget.h,.cpp` (`SAOMMOnew/UI` added to
`SAOMMOnew.Build.cs:54` include paths).
- Builds health bar + HP / level-XP / `[E] focus` texts in `NativeConstruct`;
  `NativeTick` polls the possessed `ASAOMMOCharacter`.
- `ASAOMMOPlayerController` defaults `HudWidgetClass` to the code HUD
  (Blueprint children can override) and spawns it for local players in
  `BeginPlay`. Survives respawn by re-resolving the pawn each tick.
- Gotcha: `AActor::GetActorLabel()` is editor-only — Game build failed on
  it; HUD uses `GetName()` instead. Always compile **both** targets.

## Verify

- `UnrealBuildTool SAOMMOnewEditor Win64 Development` → Succeeded
- `UnrealBuildTool SAOMMOnew Win64 Development` → Succeeded
  (`Binaries/Win64/SAOMMOnew.exe` rebuilt)

## Known issue (pre-existing, needs Editor session)

Running the uncooked exe headless
(`SAOMMOnew.exe /Game/Levels/StartingReach/L_StartingReach -nullrhi ...`)
crashes in async package IO (`FBufferReaderBase`, after
`LogAssetRegistry: Error: Failed to load premade asset registry`)
**before any SAOMMO code runs**. Reproduces with cleared
`Intermediate/CachedAssetRegistry*`, so it is not a stale cache: one of the
`Content/` packages (likely a 5.4-era asset) fails to serialize under 5.8.
C++ is exonerated (no module errors in the log). Fix in Editor: open
`L_StartingReach` in UE 5.8, run Map Check, resave packages. Do NOT bulk-
resave headless without a content backup.

## Revert

- All changes are additive C++ under `Source/SAOMMOnew/` (+1 Build.cs line).
  Revert per file via git; no content/Blueprint was touched.
