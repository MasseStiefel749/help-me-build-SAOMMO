# Region 01 — Starting Reach (Brunnfeld Belt)

**Status:** Provisional — first playable test region
**Threat tier:** Safe → Low → Medium (progression within one connected area)
**Companion:** Band 7 §7, Band 3 §20 (vertical slice)
**Date:** 2026-08-15

> All proper names in this document are **working names** until approved by the project author. See `open-questions.md`.

---

## 1. Region Purpose

This region exists to support the **first vertical slice**:

* safe hub for services and orientation
* training space for sword basics
* first real combat in wilderness
* first dungeon teaser pointing toward Artifacts

It must be **small** — enough to test gameplay, not a full MMO zone.

---

## 2. Region Map (Conceptual)

```text
                    [Grauwaldrand — Wilderness Border]
                              │
                    [Ruine des Ersten Griffs — Dungeon]
                              │
    ┌─────────────────────────┴─────────────────────────┐
    │              Brunnfeld Belt (Starting Reach)       │
    │  ┌──────────────┐         ┌─────────────────────┐  │
    │  │  Brunnfeld   │─────────│  Klingenhof         │  │
    │  │  (Safe Town) │  road   │  (Training Grounds) │  │
    │  └──────────────┘         └─────────────────────┘  │
    └─────────────────────────────────────────────────────┘
                              │
                    [Player spawn / tutorial entry]
```

Estimated playable footprint for prototype: **one Unreal level** with sub-areas, not World Partition.

---

## 3. Brunnfeld *(provisional name)*

**Type:** Starting town / safe zone  
**Threat:** None (combat disabled or trivial)  
**Role:** Tutorial anchor, social hub, services

### Features (prototype minimum)

| Feature | Gameplay purpose | Priority |
| ------- | ---------------- | -------- |
| Spawn point | Player entry | Required |
| Inn / rest marker | Future respawn anchor | Placeholder |
| General merchant | Future economy | Placeholder |
| Quest board or NPC | Orientation | Optional for slice |
| Reconstruction visuals | Lore: rebuilding civilization | Art direction |

### Lore tone

Brunnfeld is a **reconstruction settlement** — wooden scaffolds, repaired stone, mixed old and new architecture. Sword culture is respected but not worshipped. Guards exist but the town feels cautious, not militarized.

### NPC ecology (concept)

| Archetype | Role |
| --------- | ---- |
| Town elder / council voice | Explains reconstruction theme once |
| Blade instructor (redirect) | Points player toward Klingenhof |
| Merchant | Basic supplies later |
| Guard captain | Local authority presence |

Full schedules and dialogue trees are **post-slice**.

---

## 4. Klingenhof *(provisional name)*

**Type:** Training grounds  
**Threat:** None  
**Role:** Safe combat practice before wilderness

### Features

* Training dummies or passive targets
* Open yard with clear boundaries
* Optional: non-lethal sparring NPC (post-slice)
* Visual link to Blade Traditions faction (Band 7 §6)

### Gameplay link

First place the player **holds and swings a sword** without lethal stakes. Connects Band 3 sword combat to world space.

---

## 5. Grauwaldrand *(provisional name)*

**Type:** Wilderness border  
**Threat:** Low  
**Role:** First real enemy encounters

### Environment

* Forest edge, broken wall segments, old road ruins
* Visibility varies — readable combat space for VR and desktop
* Environmental storytelling: abandoned farmstead, collapsed watchtower (no deep lore yet)

### Enemy ecology (initial)

| Enemy | Behavior | Notes |
| ----- | -------- | ----- |
| Scavenger beast | Simple melee, low HP | First kill target for slice |
| Bandit scout *(optional)* | Ranged or melee, slightly smarter | Introduces human threat |

Patrols, day/night cycles, and faction relationships: **deferred** until basic AI loop works (Band 4 §9).

### Resources

* Basic gathering nodes (placeholder) — wood, herbs — for future crafting only

---

## 6. Ruine des Ersten Griffs *(provisional name)*

**Type:** First dungeon / ruin  
**Threat:** Medium (within starter context)  
**Role:** Artifact teaser — narrative landmark, not loot pinata (Band 7 §8)

### Structure

```text
Entry (collapsed gate)
  ↓
Outer hall — 2–3 enemy rooms
  ↓
Inner chamber — sealed Artifact niche (not obtainable in slice)
  ↓
Exit or return path
```

### Lore rules

* Player learns Artifacts **exist** and are **guarded**
* No Artifact obtained in vertical slice — only visual/teaser
* Guardian concept: stone construct or bound blade spirit — **design TBD**, document before implementation

### Gameplay

* Short dungeon (10–15 minutes first clear)
* One miniboss or elite encounter optional
* Environmental hazard placeholder (pit, collapsing floor) — only if core combat already stable

---

## 7. Progression Flow (Player Path)

```text
Spawn in Brunnfeld
  ↓
Orientation (movement, camera, VR entry)
  ↓
Klingenhof — pick up sword, practice swing
  ↓
Grauwaldrand — first enemy, first kill
  ↓
Ruine des Ersten Griffs — dungeon teaser, Artifact glimpse
  ↓
Return to Brunnfeld / restart on death
```

Matches Band 3 §20 vertical slice checklist.

---

## 8. Multiplayer Notes (Future)

Brunnfeld should eventually support multiple players in hub. For prototype: **single-player only**. Do not block future hub design with non-replicated singletons.

---

## 9. Art & Level Direction

* Readable lighting for VR (Band 5 §10)
* Modular kit: town buildings, fence segments, forest edge, ruin stones
* No World Partition, PCG landscape, or streaming for this region yet (Band 4 §11–12)

---

## 10. Unreal Implementation Path

| Phase | Deliverable |
| ----- | ----------- |
| Slice | One level `L_StartingReach` or sub-levels instanced |
| Post-slice | Data layers for town / wilderness / dungeon if needed |
| MMO scale | World Partition only when region count justifies it |

Suggested Content path (when assets exist):

```text
Content/Levels/StartingReach/
Content/World/StartingReach/   (data assets, spawn tables)
```

---

## 11. Consistency Checklist

Before adding content to this region, verify (Band 4 §14):

- [ ] Does not contradict `lore/lore-anchor.*.md`
- [ ] Does not contradict Band 7 themes
- [ ] Location name not duplicated elsewhere
- [ ] Enemy fits threat tier
- [ ] Uncertainty logged in `open-questions.md`
