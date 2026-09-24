# AGENTS.md — help-me-build-SAOMMO

Live code: `SAOMMOnew/` (Unreal Engine 5.8, C++). Docs may mention 5.4.4 historically — match the installed engine.

## First read (in order)

1. `SAOMMOnew/ki integration/projekt-ki-promts-6-of-6` (Band 6 — AI Development Manual, binding)
2. `SAOMMOnew/ki integration/projekt-ki-promts-2-of-6` (Band 2 — Technical Architecture)
3. The band matching the task (1 vision, 3 gameplay, 4 world, 5 art, 7 lore, 8 onboarding)
4. Role prompt: `SAOMMOnew/ki integration/Docs/AI/agents/<role>.md`
5. Local runtime: `SAOMMOnew/ki integration/Docs/AI/LOCAL-SETUP.md`

## Hard rules (from Band 6)

- Scope: only files the task needs. Never touch `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`, `.vs/`, `Backup/`.
- Do not change what you do not understand. No rewrites of working systems.
- Technical changes must not silently alter lore, balance, or visual direction.
- Verify before completion: compiles, no regressions, changes in scope, docs still accurate.
- Small reviewable commits with meaningful messages. Never force-push, never rewrite history.
- `generated.h` stays the last include. Prefer C++ over Blueprints, Enhanced Input over legacy.
- Branch: work on `main` (default branch, kept in sync with origin). `going-to-make-an-project` exists only on origin and is fully merged into `main` (0 unique commits; `main` 33 ahead as of 2026-09-25) — never check it out. Working tree must be clean before agent edits (`git status`).
