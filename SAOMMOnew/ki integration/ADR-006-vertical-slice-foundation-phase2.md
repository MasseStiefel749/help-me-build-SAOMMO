# SAOMMO — ADR-006: Vertical Slice Foundation, Phase 2 (Wiring & Systems)

**Date:** 2026-08-15
**Status:** Active
**Builds on:** ADR-005

## Context

Phase 1 created the isolated foundation classes (input frame, character, sword,
enemy). Phase 2 connects them and adds the next systems from Band 2 / Band 3 so
the architecture is coherent rather than a set of orphan classes.

## Decisions

### ADR-006a — Shared combat contract in Core
`SAOCombatInterfaces.h` defines `ISAOCombatAttacker` / `ISAOCombatDamageable`
in `Core/` so Combat, Enemies, Character, and VR share one contract (mirrors the
Variant_Combat interfaces but in the target module). `ASAOEnemy` now implements
`ISAOCombatDamageable` (health, healing, danger notification, death).

### ADR-006b — GameMode / PlayerController wired
`ASAOMMOGameMode` and `ASAOMMOPlayerController` (in `Core/`) replace the template
variants as the target-architecture entry points: the controller adds Enhanced
Input mapping contexts and respawns the character on death (Band 3 §11). The
character class is still abstract, so a Blueprint subclass + an
`UInputMappingContext` asset must be assigned in the editor (blocked: requires
UE 5.8 + content).

### ADR-006c — Combat component maps input to armed swings
`USAOMMOCombatComponent` (in `Combat/`) is owned by `ASAOMMOCharacter`. Each tick
it reads `FSAOInputFrame` and the sword's swing velocity; an attack intent edge
or a swing above `SwingArmThreshold` arms the sword for `ArmedDuration`, during
which `ASAOSword::bHitEnabled` is true. This implements Band 3 §7 (distinguish
intentional attack from movement) and Band 3 §8 (velocity-based swing) without
putting combat in the character.

### ADR-006d — VR base hierarchy
`ASAOVRCharacter` (in `VR/`) establishes the Band 2 §7 hierarchy
(`VROrigin → Camera / LeftHand / RightHand`) and reuses the same
`FSAOInputFrame`. Hand anchors are `USceneComponent` placeholders; real OpenXR
motion-controller components are added during VR integration (blocked: requires
OpenXR runtime + headset).

### ADR-006e — Interaction component
`USAOMMOInteractionComponent` (in `Interaction/`) does a forward line trace and
broadcasts `OnInteraction(Actor, Location)` so pickup / activate / talk behavior
can be layered on later (Band 3 §6). Data-only on purpose.

## Verification

- Must compile against **UE 5.8** (not possible in this environment).
- Local steps: assign `ASAOMMOCharacter` Blueprint + InputMappingContext to
  `ASAOMMOGameMode`/`ASAOMMOPlayerController`, place `ASAOSword` (armed by the
  combat component) and `ASAOEnemy` in a level, then run the Band 3 §20 loop.

## Remaining (blocked without editor / engine / content)

1. Blueprint subclasses + InputMappingContext asset and assignment.
2. Spawn/attach a real sword to the character's hand socket.
3. OpenXR motion-controller components in `ASAOVRCharacter`.
4. NavMesh + BehaviorTree/StateTree to replace the enemy FSM (ADR-005d).
5. Test level grounded in Starting Reach (Band 4 §17).
