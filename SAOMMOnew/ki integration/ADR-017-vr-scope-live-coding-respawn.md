# SAOMMO — ADR-017: VR slice scope, per-commit compile state, respawn model

**Date:** 2026-09-24
**Status:** Active
**Builds on:** ADR-016
**Bands:** B2 §7 (VR hierarchy), §3/§4 (input abstraction/flow), §8 (combat), B3 §5 (VR movement), §20 (vertical slice), B6 §15 (Decision Log)
**Resolves audit forward-references:** `VERTICAL_SLICE_AUDIT_5.8.md:196` (G5 "planned ADR-017") and `:216` (compile state per Block 3 commit)

## Context

Block 3 is code-complete (3a spawn-under-XR `3b106d7`, 3b tracked hands `a5cd28c`,
3c sword + combat `645c16c`), which forced the three decisions the audit deliberately
left open: what the slice actually requires in VR, how the verification state of each
C++ commit is recorded now that the overnight no-build rule was relaxed for Live
Coding, and how respawn (gap G5) can express a VR pawn — `AVRCharacter` derives
`ACharacter`, while both ownership fields are typed `TSubclassOf<APlayerCharacter>`
(`MainPlayerController.h:65`, `MainGameMode.h:29`; the game-mode field is bypassed
under XR by the 3a override, the controller's `DoRespawn` at
`MainPlayerController.cpp:448` is still desktop-only).

## Decisions

### ADR-017a — VR scope of the vertical slice

- **P5–P8 are the VR-native slice points and are code-complete.** Runtime proof is
  headset-bound and tracked as such (audit P5–P8, backlog headset row). One activation
  gate decides XR mode: `GEngine->XRSystem` validity (the 3a pattern), reused by
  ADR-017c; the desktop path stays untouched when no XR system is present.
- **Input remains device-independent.** Motion-controller poses are mirrored into the
  shared `FInputFrame` (B2 §3/§4); no OpenXR type crosses into gameplay.
- **P8's designed feed is swing velocity**, not a trigger button:
  `CombatComponent.cpp:55-62` arms only on motion devices above `SwingArmThreshold`.
  No trigger-button attack binding ships in the slice; whether a trigger attack is
  wanted at all is a post-slice feel decision (B3 §5: VR movement stays simple
  initially). This is why Block 3c could add the sword without touching
  `CombatComponent`.
- **P9–P11 (hit/damage/kill) run on VR today** through the shared chain (sword overlap
  → `Enemy::ApplyDamage`, kill credit via `SetOwnerActor`). **P12–P13 (be attacked /
  die) are desktop-proven only**: `AVRCharacter` has no health, `TakeDamage` or `Die`
  — those live on `APlayerCharacter` (`PlayerCharacter.cpp:291-301`, `:322`). This is
  a new, source-derived gap (B3 §20 lists one linear flow), recorded as its own
  backlog row; it, plus ADR-017c, gates the P14-VR proof.

### ADR-017b — Compile state recorded per commit

- Every C++ commit message states its verification state explicitly:
  `Build.bat SAOMMOnewEditor Win64 Development → Result: Succeeded`, or
  `Live Coding patch → <log evidence>`, or the word **unverified** plus the reason
  (typically: external UBT gated by a running editor).
- Every C++ commit gets a row in `[[99 AI/Live Coding Status]]` (build-per-commit
  table); that file remains the machine-readable history — this ADR fixes the rule,
  not the format. Rows may be completed retroactively after a later verification, but
  the original state stays visible (no silent backfill).
- Gate handling while an editor + `LiveCodingConsole` is running (UBT refuses on the
  `Global\LiveCoding_C++…` mutex): allowed triggers are the editor hotkey
  `Ctrl+Alt+F11` (focus-independent raw-input hook, `LiveCodingModule2.cpp:101-157`),
  the console commands `LiveCoding.Compile` / `LiveCoding.CompileSync`
  (`LiveCodingModule.cpp:423-440`, via editor console or MCP), or a plain build after
  the editor closes. Live Coding is the primary path; Hot Reload only as ABI fallback
  (project rule). An unverified commit must be verified at the next plain-build window.

### ADR-017c — Respawn picks the class from the same XR gate

- **Decision: XR-conditional respawn class with one shared gate.**
  `AMainPlayerController` keeps `TSubclassOf<APlayerCharacter> CharacterClass` (the
  desktop contract: health, death screen, save expectations stay compile-checked) and
  gains a VR counterpart `TSubclassOf<AVRCharacter> VRCharacterClass` (default
  `AVRCharacter::StaticClass()`). `DoRespawn` selects between them using the **same**
  XR-system check as `MainGameMode::GetDefaultPawnClassForController_Implementation`;
  both call sites share one static helper so the gate cannot drift.
- **Rejected (A): widen both fields to `TSubclassOf<APawn>`.** Loses the typed
  player-character contract at compile time — any pawn would be assignable, and the
  death-screen/save expectations of the respawned actor become unchecked.
- **Rejected (B): rebase `AVRCharacter` onto `APlayerCharacter`.** Forces desktop
  assumptions (Manny mesh, boom/camera layout, health defaults) onto the B2 §7 VR
  hierarchy and rewrites a working class to satisfy a type — against the band rule of
  changing only what a task needs.
- **Dependency:** the G5 code fix is now unblocked (type expressibility) but the
  P14-VR *proof* additionally needs ADR-017a's VR health/death work, because a pawn
  that cannot die cannot restart.

## Consequences

- Backlog #7 (G5 respawn) is unblocked and must implement the 017c shape; the new
  backlog row (VR health/death) gates P12–P13-VR, and P14-VR needs both.
- Compliance checked at write time: `1d86b01`, `3b106d7`, `a5cd28c`, `645c16c` all
  carry `Build.bat → Result: Succeeded` in their messages and
  `[[99 AI/Live Coding Status]]` rows — the rule formalizes existing practice.
- The audit keeps its point-in-time form; its two "planned ADR-017" forward references
  resolve to this document (tracked in `[[10 Projects/SAOMMO — Widerspruchstagebuch]]`).
- Slice test list in VR grows honestly: P5–P8 (headset), P9–P11 (shared chain),
  P12–P14 blocked on VR health/death + G5.
