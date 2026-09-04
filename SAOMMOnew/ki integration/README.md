# SAOMMO — KI Integration & Project Prompts

**Project:** SAOMMO (open-source VR MMORPG, Unreal Engine 5.8)  
**Purpose:** Design documentation and AI coordination for contributors  
**Live code:** `../` (SAOMMOnew Unreal project)

---

## Quick Start

1. Read **Band 1** (Master Vision) and **Band 2** (Technical Architecture).
2. Pick the band matching your work (see table below).
3. If touching narrative or world: read **Band 7** and `Docs/World/lore/`.
4. Inspect `../Source/SAOMMOnew/` before changing code.
5. Follow **Band 6** (AI Development Manual) for scope and Git hygiene.

**Want to contribute?** Use a prompt band below, or email the project owner with what you can offer (see Band 8).

---

## Prompt Bands

| File | Band | Use when |
| ---- | ---- | -------- |
| `projekt-ki-promts-1-of-6` | 1 — Master Vision | Overall project context |
| `projekt-ki-promts-2-of-6` | 2 — Technical Architecture | C++, engine, architecture |
| `projekt-ki-promts-3-of-6` | 3 — Gameplay Bible | Combat, movement, slice |
| `projekt-ki-promts-4-of-6` | 4 — World Bible | Regions, ecology, locations |
| `projekt-ki-promts-5-of-6` | 5 — Art & Asset Bible | Meshes, materials, VFX |
| `projekt-ki-promts-6-of-6` | 6 — AI Development Manual | AI agent rules |
| `projekt-ki-promts-7-of-6-story-lore` | 7 — Story & Lore Bible | Narrative, Artifacts, themes |
| `projekt-ki-promts-8-of-6-contributor-onboarding` | 8 — Contributor Onboarding | New humans / agents |
| `projekt-ki-promts-9-of-6-onboarding-course` | 9 — Onboarding Course | Step-by-step UE setup |

---

## World Documentation (`Docs/World/`)

Band 4 world content lives here for level design and World AI:

| Path | Contents |
| ---- | -------- |
| `Docs/World/lore/lore-anchor.en.md` | Canonical lore (English) |
| `Docs/World/lore/lore-anchor.de.md` | Canonical lore (German) |
| `Docs/World/regions/01-starting-reach.md` | First test region (Brunnfeld belt) |
| `Docs/World/factions/initial-factions.md` | Faction archetypes |
| `Docs/World/terminology.md` | EN/DE world terms |
| `Docs/World/test-arena-spec.md` | Vertical slice level brief |
| `Docs/World/open-questions.md` | Unresolved lore — do not guess |

**Legacy lore files** (same text, kept for reference):

* `(en) storry vor promt 4`
* `(de) storry vor promt 4`

---

## Engine Note

Documentation may reference UE 5.4.4 historically. The live project uses **UE 5.8** — match the installed engine (Band 2 §0.1, Band 8 §5).

---

## First Gameplay Goal

From Band 3 §20 — vertical slice is done when the player can move, switch cameras, enter VR, swing a sword, fight an enemy, die, and restart in a test level grounded in **Starting Reach** world design.

---

## Contact

Open-source contributions welcome via GitHub or email to the project owner (describe skills and first concrete task).
