# SAOMMO 5.8 — BP + Input Wiring Guide (ADR-005/006/007 Blockers)

**Goal:** Wire the already-compiled C++ target architecture into playable Blueprints + Enhanced Input. This unblocks Band 3 §20 vertical slice.

**Existing C++:** `ASAOMMOCharacter`, `ASAOSword`, `ASAOEnemy`, `ASAOMMOGameMode`, `ASAOMMOPlayerController`, `USAOMMOCombatComponent`, `USAOMMOInputFrameComponent`, `USAOMMOInteractionComponent`, `ASAOVRCharacter`, `USAOMMOInventoryComponent`, `USAOMMOProgressionComponent`, `USAOMMOSaveGame`, `USAOMMOWorldSubsystem` — all compile via `SAOMMOnew.Build.cs:12` (UE 5.8).

**Existing assets (already in repo):** `Content/Blueprints/BP_SAOMMOCharacter*`, `BP_SAOSword`, `BP_SAOEnemy`, `BP_SAOMMOGameMode`, `BP_SAOMMOPlayerController`, `Content/Input/IA_Move|Look|Jump|Attack|ToggleCamera`, `Content/Input/IMC_SAOMMO` (5KB) + `IMC_Default/MouseLook`.

---

## 1. Input — IMC_SAOMMO (verify in Editor)

Open `Content/Input/IMC_SAOMMO`:

| Mapping | IA | Key | Trigger | Modifier |
|---------|----|-----|---------|----------|
| Move | `IA_Move` (Vector2D) | WASD / Left Stick | Hold | DeadZone 0.2, Swizzle YX |
| Look | `IA_Look` (Vector2D) | Mouse XY / Right Stick | Hold | Negate Y optional, Sens 0.07 |
| Jump | `IA_Jump` (Digital) | Space / Face Button Bottom | Pressed | — |
| Attack | `IA_Attack` (Digital) | LMB / Right Trigger | Started | — |
| ToggleCamera | `IA_ToggleCamera` (Digital) | V / DPAD Up | Started | — |

If bindings missing: add them, save. `Config/DefaultInput.ini:81` already sets `DefaultPlayerInputClass=/Script/EnhancedInput.EnhancedPlayerInput`.

## 2. BP_SAOMMOCharacter wiring

Open `BP_SAOMMOCharacter` (parent `ASAOMMOCharacter`):

- `Details → Pawn → Use Controller Rotation Yaw = true`
- `Class Defaults → Input`:
  - `MoveAction = IA_Move`
  - `LookAction = IA_Look`
  - `JumpAction = IA_Jump`
  - `AttackAction = IA_Attack`
  - `ToggleCameraAction = IA_ToggleCamera`
- `Class Defaults → Combat`:
  - `DefaultSwordClass = BP_SAOSword`
  - `MaxHealth = 10`
- `Class Defaults → Camera`:
  - `FirstPersonEyeHeight = 70`, `ThirdPersonBoomLength = 300`
- Mesh: assign `SKM_Manny` or Epic mannequin; ensure socket `hand_rSocket` exists (or create socket `hand_r` on hand bone for sword).
- Components verify: `CameraBoom`, `FollowCamera`, `InputFrame`, `CombatComponent` present.

> Fix included in `Source/.../Character/SAOMMOCharacter.cpp:42` → sword now attaches to `hand_rSocket/hand_r` if present, else mesh, else root. Previous was root-only.

## 3. BP_SAOSword

Parent `ASAOSword`:
- `Damage = 1.0`, `PerTargetCooldown = 0.4`, `bHitEnabled` initially true but `CombatComponent` drives it (Armed 0.25s).
- `BladeCollision` SphereRadius 15, `Mesh` no collision — visual only.
- Optional: static mesh `SM_Katana` placeholder, scale to hand.

## 4. BP_SAOEnemy

Parent `ASAOEnemy`:
- `MaxHealth = 3`, `DetectRange=1000`, `AttackRange=120`, `ApproachSpeed=200`, `AttackDamage=1`, `RecoverTime=1.0`
- Assign mesh + capsule same as character; no NavMesh needed for FSM (tick-driven `MoveTowardTarget`), but NavMesh helps for future BT.
- Implements `ISAOCombatDamageable` → `ApplyDamage/TakeDamage` already wired via `SAOMMOCombatComponent::OverlapMultiByChannel` (CombatComponent.cpp:85).

## 5. BP_SAOMMOGameMode & BP_SAOMMOPlayerController

- `BP_SAOMMOGameMode` (`ASAOMMOGameMode`):
  - `DefaultPawnClass = BP_SAOMMOCharacter`
  - `PlayerControllerClass = BP_SAOMMOPlayerController`
  - `DefaultCharacterClass = BP_SAOMMOCharacter` (C++ `DefaultCharacterClass` field for respawn)
- `BP_SAOMMOPlayerController` (`ASAOMMOPlayerController`):
  - `DefaultMappingContexts = [IMC_SAOMMO]` (array, index 0)
  - `CharacterClass = BP_SAOMMOCharacter`
  - `RespawnTransform` cached on `BeginPlay`/`OnPossess` (fix in `SAOMMOPlayerController.cpp:11` — no longer spawns at 0,0,0).

**Editor check:**
- Open `BP_SAOMMOPlayerController` → verify IMC_SAOMMO appears.
- If empty, add IMC_SAOMMO and compile/save.
- Confirm `SetupInputComponent` adds context via `UEnhancedInputLocalPlayerSubsystem` (code already does).

## 6. Level World Settings

For `L_StartingReach` (see `Content/Levels/StartingReach/README.md`):
- World Settings → GameMode Override = `BP_SAOMMOGameMode` — this overrides `Config/DefaultEngine.ini:4` (`BP_ThirdPersonGameMode`) for this level only.
- Keep `DefaultEngine.ini:2` as `Lvl_ThirdPerson` until slice proven; no global change needed. When ready, change to:
  ```
  GameDefaultMap=/Game/Levels/StartingReach/L_StartingReach
  EditorStartupMap=/Game/Levels/StartingReach/L_StartingReach
  GlobalDefaultGameMode=/Game/Blueprints/BP_SAOMMOGameMode.BP_SAOMMOGameMode_C
  ```

## 7. VR Pawn (optional)

- Duplicate `BP_SAOMMOCharacter` → `BP_SAOVRCharacter` parent `ASAOVRCharacter`.
- Verify `VROrigin → VRCamera/LeftHand/RightHand` hierarchy. Add `MotionController` components later (blocked: OpenXR headset).

## 8. Verification Checklist (must pass before commit)

- [ ] Build in VS: `SAOMMOnew` module `Build succeeded` (UE 5.8)
- [ ] PIE: WASD+Mouse moves, `V` toggles FP/TP without respawn, `Space` jumps, `LMB` arms sword
- [ ] Sword velocity trace: fast swing OR `IA_Attack` arms `bArmed` 0.25s → overlap hits enemy
- [ ] Enemy: 3 hits kill, `OnDied` destroys actor; player death (`Health<=0` → `Destroy` → `OnPawnDestroyed` respawns at `RespawnTransform`)
- [ ] No log errors: `Ensure` on missing `Move/Look` actions resolved (assigned above)

**Commit message when verified:**
```text
Wire SAOMMO target architecture: BP gamemode/controller/character + IMC_SAOMMO (ADR-006)
```
