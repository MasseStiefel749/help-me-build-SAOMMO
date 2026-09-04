# SAOMMO — ADR-007: Vertical Slice Foundation, Phase 3 (Supporting Systems)

**Date:** 2026-08-15
**Status:** Active
**Builds on:** ADR-005, ADR-006

## Context

The vertical-slice gameplay systems (input, character, combat, enemy, VR base,
interaction) are in place. Phase 3 adds the remaining small, self-contained
target-architecture systems that do **not** require the UE editor or content
assets, so the module has homes for inventory, progression, save, and world
state.

## Decisions

### ADR-007a — Shared item types in Core
`SAOItemTypes.h` defines `ESAOItemType` and `FSAOItem` in `Core/` so Inventory
and SaveGame share one definition.

### ADR-007b — Inventory component (Band 3 §13)
`USAOMMOInventoryComponent` (in `Inventory/`) is a flat, stackable item list with
add/remove/query and an `OnInventoryChanged` delegate. Deliberately does no
item-behavior logic; that is layered on later.

### ADR-007c — Progression component (Band 3 §12)
`USAOMMOProgressionComponent` (in `Progression/`) tracks level/experience with a
simple threshold and an `OnLevelUp` delegate. Exists so the combat loop can
record XP; no balancing.

### ADR-007d — Save wrapper (Band 2 §15)
`USAOMMOSaveGame` (in `Core/`) wraps `USaveGame` (player transform, health,
inventory, progression). No custom save framework, per the architecture.

### ADR-007e — World subsystem stub (Band 2 §12, Band 4)
`USAOMMOWorldSubsystem` (in `World/`) is a minimal `UWorldSubsystem` tracking the
active region. Heavy world tech (World Partition, streaming, PCG) is intentionally
deferred until gameplay is proven.

## Verification

- Must compile against **UE 5.8** (not possible in this environment).
- These classes are additive and not yet referenced by a GameMode/level.

## Status of the implementable backlog

With Phase 3 the non-deferred, non-editor-dependent target-architecture systems
are implemented. Remaining work is either **blocked** (needs UE 5.8 editor,
content assets, or a VR headset) or **deferred by design** (per Band 2 / Band 3):

- Editor/content: Blueprint subclasses, InputMappingContext, sword-to-hand
  attachment, test level (Starting Reach, Band 4 §17).
- VR: OpenXR motion-controller components in `ASAOVRCharacter` (needs headset).
- Deferred: Network/multiplayer (Band 2 §13), MMO infrastructure, World
  Partition (Band 2 §12), GAS (Band 2 §14), Budget VR bridge (Band 2 §16),
  BehaviorTree/StateTree enemy AI (ADR-005d), full World/Quest/NPC systems.

The vertical-slice loop (Band 3 §20) is now code-complete on the systems side;
finishing it end-to-end requires the steps above in the editor.
