# SAOMMO — ADR-005: Vertical Slice Foundation (Target Architecture)

**Date:** 2026-08-15
**Status:** Active
**Supersedes:** none (builds on example ADR-001..004 in Band 6 §15)

## Context

The live `SAOMMOnew` module ships Epic Lyra-style template variants
(`Variant_Combat`, `Variant_Platforming`, `Variant_SideScrolling`). Band 2 §0.2
and Band 8 §7 state these are exploration starting points, **not** the target
architecture. Band 2 §2 defines the target functional areas:
`Core / Character / Input / VR / Combat / Interaction / Enemies / UI / World /
Inventory / Progression / Network`.

The first concrete deliverable (per the chosen scope and Band 3 §20 vertical
slice) is the **foundation** that later systems attach to.

## Decisions

### ADR-005a — Input abstraction is the backbone
Introduce `FSAOInputFrame` (Band 2 §3) as a device-independent input snapshot,
produced by `USAOMMOInputFrameComponent` (Band 2 §4). Gameplay reads the frame
instead of binding directly to keyboard/mouse, so VR and Budget VR (Band 2 §16)
become additional input providers without touching gameplay.

### ADR-005b — Base character supports desktop FP/TP without recreation
`ASAOMMOCharacter` (Band 2 §5) owns a camera boom + follow camera and switches
First Person / Third Person at runtime by re-attaching the camera, never by
recreating the pawn (Band 2 §6).

### ADR-005c — Combat lives on components/actors, not the character
`ASAOSword` (Band 2 §8) is a separate actor carrying mesh, collision, damage,
owner, movement tracking, and overlap hit detection (Band 2 §9, Band 3 §8).
The character stays free of combat logic per Band 2 §8.

### ADR-005d — Enemy FSM before BehaviorTree
`ASAOEnemy` (Band 2 §10/§11) uses a lightweight tick-driven state machine
(Idle → Detect → Approach → Attack → Recover). This is intentional for the
first prototype: it runs without Navigation assets and is easy to test. A full
AI Controller / BehaviorTree / StateTree (already a project dependency) can
replace the FSM later without changing the health/combat contract.

### ADR-005e — Additive, non-destructive foundation
New classes are added alongside the existing variants (Band 8 §10, Band 6 §11).
They are **not yet wired** into a GameMode or level, so the existing playable
template still builds and runs. Wiring (GameMode, input mapping context,
Blueprint subclasses, test level) is deferred to a follow-up task that requires
the UE 5.8 editor and content assets.

### ADR-005f — GAS deferred
Reaffirms example ADR-003: Gameplay Ability System is not used for the first
prototype (Band 2 §14).

## Verification

- Code must compile against **UE 5.8** (matches `SAOMMOnew.uproject`).
- Cannot be verified in this environment (no UnrealBuildTool / engine install).
- Local steps: open `SAOMMOnew`, build the `SAOMMOnew` module, create Blueprint
  subclasses of `ASAOMMOCharacter` / `ASAOSword` / `ASAOEnemy`, assign input
  actions, place them in a test level, and confirm the vertical-slice loop
  (Band 3 §20).

## Open follow-ups

1. Wire `ASAOMMOCharacter` into a SAOMMO GameMode + PlayerController.
2. Map `FSAOInputFrame` into actual combat (swing detection from sword velocity).
3. Replace enemy FSM with BehaviorTree/StateTree once a NavMesh exists.
4. Introduce the VR origin hierarchy (Band 2 §7) reusing the same input frame.
