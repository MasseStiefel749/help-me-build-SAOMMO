# SAOMMO — ADR-016: Death screen, knockback and creator appearance loop

**Date:** 2026-09-24
**Status:** Active
**Builds on:** ADR-015
**Bands:** B3 §11 (death → Game Over → Restart), §20 (vertical slice points 1–4),
B5 §3 (character representation), B6 §15 (ADRs for decisions that outrun a band)

## Context

Band 3 §11 only specifies "Health = 0 → Death → Game Over → Restart" and explicitly
defers respawn systems to later. The vertical-slice playtest showed the restart
beat reads as one moment without any visible death state, enemies never react to
hits, and the character creator had saved data that never reached the pawn
(nothing loaded `CharacterSave`, `ApplyToMesh` only knew one mesh path, and the
creator widget had no layout without a Blueprint asset). The bands also never
specify creator colors or body types (spec gap).

## Decisions

### ADR-016a — Player-driven death screen

New code-only `UI/DeathScreenWidget` (dark red full-screen dimmer, "YOU DIED",
"PRESS ANY KEY TO RESPAWN") shown by `AMainPlayerController` when the possessed
pawn is destroyed (local players only). The respawn is triggered by **any key
press** (`InputKey(FInputKeyEventArgs)` override, `IE_Pressed`, consumed);
`RespawnDelay` (default 15 s, EditAnywhere) is only the no-input **fallback**
timer. Rationale: a fixed 3 s delay felt arbitrary in review ("already alive
again"); letting the player press a key keeps the two beats (die / restart)
distinct without forcing a wait. Remote/failed-widget cases still respawn
immediately. Deviation from §11 is purely presentational — no penalties,
checkpoints or resurrection semantics are introduced (reserved per Band 3 §11).

### ADR-016b — Knockback on both sides

`AEnemy` gains `HitKnockbackSpeed` (350, pushed when the enemy is hit) and
`AttackKnockbackSpeed` (300, pushed on the victim during `PerformAttack`), both
EditAnywhere UPROPERTYs applied via a shared static `PushAway` helper that uses
`LaunchCharacter(dir, XZOnly)`. Bands do not specify knockback (spec gap); it is
a feel-level addition, disabled per-instance by setting the speed to 0.

### ADR-016c — Creator appearance loop closed (code-only)

- The creator widget now builds its whole layout in C++ when no designer tree
  exists (HUD pattern), so it works with zero Editor assets: name, body type
  combo, skin/hair/eye color cycling buttons with swatches, CONFIRM/CANCEL.
- `AMainPlayerController` auto-opens it ~0.3 s after BeginPlay **when no
  `CharacterSave` slot exists** (first boot); the **C** key re-opens it later.
  The overlay runs paused in UIOnly input mode; CONFIRM saves and reloads the
  level, CANCEL restores game input.
- `APlayerCharacter::BeginPlay` loads `CharacterSave` and applies
  `BodyType` (Manny/Quinn mesh + anim class) plus a best-effort **skin tint**
  via a runtime MID (candidate parameter names; no-op when the material exposes
  none — mannequin materials have no guaranteed tint parameter).
- Hair/eye colors are persisted but have no visual target on the bald mannequin
  until hair/eye assets exist (Band 5 §3–§5 representation work).
- Dead `/Game/SAO/...` armor paths removed (folder never existed); the weapon
  list points at the one real mesh, `/Game/Weapons/SM_SAOSword`.

## Consequences

- Death beat, hit feedback and first-boot customization are playable in the
  §20 desktop chain (points 1–4) with no Editor setup.
- Slice test list grows: death → any-key respawn (15 s fallback), knockback on
  hit and on being hit, first-boot creator, C to re-edit.
- `CharacterSave` (creator) and `PlayerSave` (progress) remain separate slots;
  merging them is a later decision (Band 2 §15 keeps USaveGame as-is).
- Band 3 §11/§20 docs should mention the death-screen presentational layer at
  the next docs pass; creator color/body-type behavior is still a spec gap.
