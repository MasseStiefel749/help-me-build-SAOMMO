# ULTRA-PROMPT — SAOMMO einlesen (paste into Cline, read-only orientation, NO code changes)

You are orienting on the SAOMMO project. This is a READ-ONLY task: do not modify any file.
Local model: qwen3-coder:30b via Ollama. Work folder by folder in the order below.
After each folder, write 3-5 bullet lines: what lives there + what matters for future tasks.
End with: project structure map, open questions, and which agent role (technical/gameplay/world/art/qa, see `SAOMMOnew/ki integration/Docs/AI/agents/`) should own each area.

## Step 0 — entry points (repo root `help-me-build-SAOMMO/`)

1. `AGENTS.md`, `.clinerules`, `README.md`
2. `projekt-ki-promts-1-of-6` (Band 1 Master Vision — exists ONLY here)

## Step 1 — coordination hub (`SAOMMOnew/ki integration/`)

3. `README.md` (band index — your reading map)
4. `projekt-ki-promts-2-of-6` (Band 2 architecture), `-3-of-6` (gameplay), `-5-of-6` (art), `-6-of-6` (AI manual, binding), `-7-story-lore`, `-8-onboarding`, `-9-course`
5. `projekt-ki-promts-4-of-6` (Band 4 World Bible — use THIS copy; the root copy differs and is stale)
6. ADRs `ADR-005` through `ADR-015` (newest first: 015 → 005), then `WIRING_GUIDE_SAOMMO_5.8.md`, `PLUGIN_AUDIT_5.8.md`

## Step 2 — world docs (`SAOMMOnew/ki integration/Docs/World/`)

7. `lore/lore-anchor.en.md`, `regions/01-starting-reach.md`, `test-arena-spec.md`, `terminology.md`, `factions/initial-factions.md`, `open-questions.md` (memorize: never guess these)

## Step 3 — live code (`SAOMMOnew/`)

8. `SAOMMOnew.uproject`, `Config/` (input, game, engine ini)
9. `Source/SAOMMOnew/SAOMMOnew.Build.cs`, `SAOMMOnew.h/.cpp`
10. `Source/SAOMMOnew/` subfolders: `Core/`, `Character/`, `Combat/`, `Input/`, `VR/`, `Enemies/`, `Interaction/`, `Inventory/`, `Progression/`, `UI/`, `World/` (list every .h/.cpp per folder, one line each: class + purpose)
11. `Legacy*.h/.cpp`, `Variant_Combat/`, `Variant_Platforming/`, `Variant_SideScrolling/` (mark legacy vs active)
12. `Plugins/` (folder names + what each plugin does, from its .uplugin)

## Step 4 — assets (names only, binaries unreadable)

13. `Content/` top-level folder names only. Do NOT open `.uasset`/`.umap`.

## Forbidden this task

`Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`, `.vs/`, `Backup/`, `DashLibData/` — never list or open.
No summaries as new files (Band 6 §17). Answer in chat only.
Engine is UE 5.8 live (docs mentioning 5.4.4 are historical).
