# Vertical Slice Audit — UE 5.8 (Band 3 §20)

**Date:** 2026-09-24
**Basis:** `projekt-ki-promts-3-of-6` §20 "Vertical Slice Definition" (14 points)
**Method:** Code-level audit only. No PIE run, no headset, no build during this audit
(overnight rules at the time of writing: no-build). Every claim below names file + line.
**Branch:** `main` @ `2712723` (ADR-016) as the audited baseline.

## How to read this

| Status | Meaning |
|--------|---------|
| ✅ **Code evidence** | The code path exists and is wired end-to-end. Not the same as *playtested*. |
| 🟡 **Partial / unproven** | Code exists but a link in the chain is missing, unverified, or editor-configured only. |
| ❌ **Not implemented** | No code path found. |
| 🚫 **Headset-blocked** | Cannot be proven without an HMD, regardless of code state. |

> **Honesty rule (Band 6 §26):** "Code evidence" is *not* "works". Points 1–4 are
> considered *implemented but not runtime-proven in this audit*; the last known play proof
> predates ADR-016 and is only as good as the last manual test recorded in the ADRs.

---

## Status board

| # | Slice point | Status | Evidence / gap |
|---|-------------|--------|----------------|
| 1 | Start the game | 🟡 | Boot config exists, but `GlobalDefaultGameMode` points at quarantined content (see P1 detail). |
| 2 | Enter a test level | ✅ code | `GameDefaultMap=/Game/Levels/StartingReach/L_StartingReach` (`Config/DefaultEngine.ini:2`). Level is a native blockout (ADR-013). |
| 3 | Move around | ✅ code / 🟡 runtime | `APlayerCharacter` movement input → `AddMovementInput` (`PlayerCharacter.cpp:231-232`). Runtime feel unproven in this audit. |
| 4 | Switch FP/TP on desktop | ✅ code / 🟡 runtime | `OnToggleCamera` (`PlayerCharacter.cpp:251`), bound at `:351`, fallback key `V` (`MainPlayerController.cpp:199`). |
| 5 | Enter VR | 🟡 | `AVRCharacter` exists but is spawned by nothing. Not referenced by `AMainGameMode`, no level instance, no XR activation. |
| 6 | See tracked hands | ❌→🟡 | Hand anchors are plain `USceneComponent`s (`VRCharacter.cpp:21-27`), no `UMotionControllerComponent`, so nothing tracks. |
| 7 | Hold a sword | 🟡 | `APlayerCharacter` spawns + attaches a sword (`PlayerCharacter.cpp:125-152`); `AVRCharacter` has **no** sword path at all. |
| 8 | Swing the sword | 🟡 | `CombatComponent` swing detection exists and is *already VR-aware* (`CombatComponent.cpp:59`, `EInputDevice::VRController`), but no VR pawn feeds it a hand. |
| 9 | Hit an enemy | ✅ code | Sweep hit → `Damageable->ApplyDamage(...)` (`CombatComponent.cpp:178`). |
| 10 | Deal damage | ✅ code | `ICombatDamageable::ApplyDamage` (`CombatInterfaces.h:52`), `AEnemy::ApplyDamage` (`Enemy.cpp:371`), player `TakeDamage` (`PlayerCharacter.cpp:291`). |
| 11 | Kill an enemy | ✅ code | `AEnemy::HandleDeath` (`Enemy.cpp:423`) with double-death guard (`Enemy.h:127`). |
| 12 | Be attacked | ✅ code | `AEnemy::PerformAttack` → `TargetPawn->TakeDamage(...)` + knockback (`Enemy.cpp:216-218`). |
| 13 | Die | ✅ code | `APlayerCharacter::Die` (`PlayerCharacter.cpp:322`) → pawn destroyed → `AMainPlayerController::OnPawnDestroyed` → death screen (`MainPlayerController.cpp:367-389`, ADR-016). |
| 14 | Restart | ✅ code / 🟡 runtime | Any-key or `RespawnDelay` → `DoRespawn()` (`MainPlayerController.cpp:391-412`, `:427`). Spawns `APlayerCharacter` only — VR respawn gap (see Gaps). |

**Score:** 8 points code-complete, 4 partial, 1 not implemented, plus points 5–7 which are
headset-blocked for their *proof* even where code lands tonight.

---

## Point details (only where the status is not a plain ✅)

### P1 — Start the game 🟡

`SAOMMOnew/Config/DefaultEngine.ini:4`:

```ini
GlobalDefaultGameMode=/Game/Blueprints/BP_MainGameMode.BP_MainGameMode_C
```

`BP_MainGameMode` is quarantined content per **ADR-014a** (broken content stays in place, is
not used). So the *global default* GameMode references a Blueprint the project has declared
unusable. The slice level does not depend on it — `L_StartingReach.umap` carries a native
`AMainGameMode` World-Settings override (verified by string scan of the package: `/Script/SAOMMOnew`
actors present, no `BP_MainGameMode` string). Boot therefore works **because of a level
override that rescues a broken global default**. That is a latent failure: any new map
without an explicit override inherits the broken class.

**Verdict:** game starts today, by accident of the level override. Tracked as a separate,
own-commit fix (one logical change per commit, Band 8).

### P2 — Enter a test level ✅

`GameDefaultMap` → `L_StartingReach` (`DefaultEngine.ini:2`). Level built as headless
blockout per ADR-013 / `Docs/World/test-arena-spec.md`.

### P3 — Move around ✅ code, 🟡 runtime

Movement IA → `AddMovementInput` with controller-relative direction
(`PlayerCharacter.cpp:231-232`). Input fallback mapping lives in `AMainPlayerController`
(ADR-014b), so keyboard/mouse work even before Enhanced Input assets are assigned.

**Not proven tonight:** no PIE run was executed in this audit. Prior ADRs record desktop
movement as working; that record was not re-verified here.

### P4 — FP/TP switch ✅ code, 🟡 runtime

`APlayerCharacter::OnToggleCamera` (`PlayerCharacter.cpp:251`) bound to `IA_ToggleCamera`
(`:349-351`), fallback key `V` (`MainPlayerController.cpp:199`). Camera attach paths for both
modes at `PlayerCharacter.cpp:178-214` (including sword shadow-casting toggling per view).

**Not proven tonight:** same as P3 — code evidence only.

### P5 — Enter VR 🟡

`AVRCharacter` (`VR/VRCharacter.h`) is a complete, self-contained pawn with the Band 2 §7
hierarchy, but:

- `AMainGameMode` sets `DefaultPawnClass = APlayerCharacter::StaticClass()` and knows nothing
  about VR (`MainGameMode.cpp:7-12`). There is no `GetDefaultPawnClassForController` override.
- No code checks `GEngine->XRSystem` anywhere in `Source/SAOMMOnew` (grep: 0 matches).
- Nothing references `AVRCharacter` outside its own header/implementation (grep: 0 matches).

So the pawn cannot be entered. This is exactly what Block 3 of tonight's work targets.

### P6 — Tracked hands ❌→🟡

Current hand anchors are static:

```cpp
// VRCharacter.cpp:21-27
LeftHand  = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHand"));
LeftHand->SetupAttachment(VROrigin);
...
```

Static `USceneComponent`s never move. The header itself admits the intent
(`VRCharacter.h:24-27`: "fed by real OpenXR motion controllers later"). Until a
`UMotionControllerComponent` with `MotionSource = "Left"/"Right"` is attached, point 6 is
not implementable, let alone provable.

**Also:** the hand layout is wrong per Band 2 §7 as it stands — hands are attached to
`VROrigin`, so they sit at fixed offsets from the origin rather than from the HMD/camera, and
they are not mirrored into `FInputFrame` (no VR input provider exists; `InputFrame` only gets
`SourceDevice = VRController` at `VRCharacter.cpp:38`).

### P7 — Hold a sword 🟡

Desktop path is complete: `DefaultSwordClass` default-constructed to `ASword`
(`PlayerCharacter.cpp:57`), spawned at BeginPlay (`:125-129`), attached to the hand socket
with fallbacks down to the root (`:138-147`), registered with `CombatComponent` (`:152`).

`AVRCharacter` has **no** equivalent — no `DefaultSwordClass`, no `EquippedSword`, no attach,
no `CombatComponent` at all. Points 7 and 8 both fail on the VR pawn.

### P8 — Swing the sword 🟡 (encouraging)

The swing/damage gate in `CombatComponent` already anticipates VR:

```cpp
// CombatComponent.cpp:59
&& (InputFrame->CurrentFrame.SourceDevice == EInputDevice::VRController
```

So the *gameplay* layer is device-independent as Band 2 §4 demands. What is missing is purely
the feed: no VR pawn owns a `CombatComponent`, and no hand produces hand motion. Note this is
a deliberate architecture win, not a gap — do not touch `CombatComponent` for Block 3.

### P9–P12 — Hit / damage / kill / be attacked ✅

- Hit: `CombatComponent.cpp:178` → `Damageable->ApplyDamage(Sword->Damage, Sword, ...)`.
- Damage contract: `CombatInterfaces.h:52` (`ApplyDamage`), `:56` (`HandleDeath`).
- Enemy: `Enemy.cpp:371` (`ApplyDamage`), `:423` (`HandleDeath`), double-death guard `Enemy.h:127`, kill credit `Enemy.h:133`.
- Enemy attacks player: `Enemy.cpp:206-219` (`PerformAttack` → `TakeDamage` + `PushAway`).
- Player receives: `PlayerCharacter.cpp:291-301` (`TakeDamage`, clamps, death at 0).

No runtime proof taken in this audit; the code chain is complete and reviewed line-by-line.

### P13 — Die ✅

`APlayerCharacter::Die` (`PlayerCharacter.cpp:322`) → pawn destroyed →
`AMainPlayerController::OnPawnDestroyed` (`MainPlayerController.cpp:367`) → death screen
widget + respawn timer (ADR-016). Death overlay is presentational only, as ADR-016 states.

### P14 — Restart 🟡

`InputKey` any-key fast path (`MainPlayerController.cpp:391-402`) or `RespawnDelay` timer
(`:383-384`) → `DoRespawn()` (`:427`).

Gap: `DoRespawn` hard-codes the desktop type:

```cpp
// MainPlayerController.cpp:431, :448
TSubclassOf<APlayerCharacter> SpawnClass = CharacterClass;
...
if (APlayerCharacter* Respawned = World->SpawnActor<APlayerCharacter>(SpawnClass, RespawnTransform))
```

`AVRCharacter` derives from `ACharacter`, **not** `APlayerCharacter` — so a VR player who
dies would either fail to respawn or respawn as desktop. Not fixable "properly" without
widening the controller's ownership model; documented as a known gap rather than patched
tonight (ADR-017 records the decision).

---

## Gaps and follow-ups

| ID | Gap | Severity | Where |
|----|-----|----------|-------|
| G1 | `GlobalDefaultGameMode` → quarantined `BP_MainGameMode` | High (latent boot failure) | `DefaultEngine.ini:4` |
| G2 | `AVRCharacter` not spawned by anything | Blocker for slice P5 | `MainGameMode.cpp` |
| G3 | Hands are static `USceneComponent`s, no motion controllers | Blocker for P6 | `VRCharacter.cpp:21-27` |
| G4 | No sword / no `CombatComponent` on VR pawn | Blocker for P7/P8 | `VR/` |
| G5 | `DoRespawn` spawns `APlayerCharacter` only | Blocker for VR P14 | `MainPlayerController.cpp:448` |
| G6 | No XR activation check (`GEngine->XRSystem`) anywhere | P5 | whole module |
| G7 | AGENTS.md names branch `going-to-make-an-project`; actual work is on `main` | Process/docs | `AGENTS.md` |
| G8 | Audit could not be runtime-verified (overnight no-build, no PIE, no headset) | Meta | this doc |

## What this audit deliberately did NOT do

- No builds, no PIE runs, no editor restarts (overnight constraints; the no-build rule was
  later relaxed to allow Live Coding — see ADR-017 for the state at commit time).
- No gameplay, balance or lore changes.
- No fixes applied inside the audit commit itself; every fix is its own commit (Band 8).
