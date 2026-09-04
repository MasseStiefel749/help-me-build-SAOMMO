# Test Arena Specification — Vertical Slice Level

**Status:** Active — level design brief for first playable world  
**Links:** Band 3 §20, `regions/01-starting-reach.md`, Band 9 Chapter 10  
**Date:** 2026-08-15

This document answers: **What must exist in the first test level for world design purposes?**

---

## 1. Scope Boundary

**In scope for test arena:**

* One connected level (or main level + instanced dungeon sub-level)
* Brunnfeld → Klingenhof → Grauwaldrand → Ruine teaser path
* NavMesh for player and one enemy type
* Spawn points, boundaries, kill volumes

**Out of scope:**

* Multiple regions, open world streaming
* Full NPC schedules, shops, quests
* Obtainable Artifacts
* Multiplayer replication

---

## 2. Spatial Requirements

| Zone | Min. size (guide) | Layout needs |
| ---- | ----------------- | ------------ |
| Brunnfeld | 50×50 m playable | Clear spawn, exit roads, readable landmarks |
| Klingenhof | 30×30 m | Flat training area, weapon pickup point |
| Grauwaldrand | 80×80 m | Cover, enemy spawn, return path to town |
| Ruine (teaser) | 40×60 m interior | Linear or lightly branching, sealed Artifact room |

Sizes are **guides** for blockout — adjust for fun and VR comfort.

---

## 3. Landmarks (Navigation)

Players must identify areas without minimap (slice has no full UI):

1. **Town well or fountain** — Brunnfeld center
2. **Training posts / dummies** — Klingenhof
3. **Broken gate arch** — wilderness entry
4. **Collapsed dome or blade monument** — ruin entrance

---

## 4. Boundaries

* Hard boundaries (cliffs, walls, fog) preferred over invisible walls where possible
* Kill plane below terrain for out-of-bounds
* No soft-lock geometry in VR play space

---

## 5. Enemy Placement (Slice)

| Location | Count | Type |
| -------- | ----- | ---- |
| Grauwaldrand | 2–4 | Scavenger beast |
| Ruine outer | 1–2 | Same or bandit scout |
| Ruine inner | 0–1 | Elite optional |

Respawn rules: simple timer or clear-on-death for prototype.

---

## 6. Lighting & Atmosphere

* Brunnfeld: warm, morning/afternoon — hopeful reconstruction tone
* Wilderness: cooler, dappled — danger without horror
* Ruine: dim, accent light on Artifact niche — mystery

VR: maintain readable contrast; avoid flickering strobe VFX.

---

## 7. Lore Environmental Storytelling (Minimal)

Use props only — no mandatory lore dumps:

* Scaffolding and half-repaired walls (reconstruction)
* Worn practice blades on racks (blade tradition)
* Ancient stone with blade motif at ruin (Artifact tease)

---

## 8. Verification Checklist (World Design)

Level passes world-design review when:

- [ ] Player can traverse full path without cheats
- [ ] Each zone matches threat tier in region doc
- [ ] Landmarks visible from zone entry
- [ ] Ruine shows Artifact teaser without granting item
- [ ] No lore contradictions with lore anchor
- [ ] Blockout playable in VR and desktop camera modes

---

## 9. Suggested Commit Milestone

When level blockout exists:

```text
Add Starting Reach test arena blockout (world layout)
```

When vertical slice complete:

```text
Starting Reach vertical slice: town, training, wilderness, ruin teaser
```
