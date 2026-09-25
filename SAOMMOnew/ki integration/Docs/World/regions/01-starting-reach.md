# Region 01 — Starting Reach (Brunnfeld Belt)

**Status:** Provisional — first playable test region
**Threat tier:** Safe → Low → Medium (progression within one connected area)
**Companion:** Band 7 §7, Band 3 §20 (vertical slice)
**Date:** 2026-08-15 · **Reconciled:** 2026-09-25 against Band 4 §17/§19 and the built level (status table §10.1)

> All proper names in this document are **working names** until approved by the project author. See `../open-questions.md`.

> Companion documents: `../terminology.md` — Band 4 §19 vocabulary, use these terms in all new content; `../open-questions.md` — provisional-name approvals; `../test-arena-spec.md` §10 — headless implementation status; `../factions/initial-factions.md` — territory/footprint context.

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

Both paths exist as of 2026-09-25: the two maps live in `Content/Levels/StartingReach/`, and `Content/World/StartingReach/` holds a README that reserves `DA_StartingReach` / spawn tables for post-slice — slice spawns stay hardcoded in the level (Band 4 §11–12).

### 10.1 Implementation state (reconciled 2026-09-25)

Concept (this document) versus the built `L_StartingReach`. Evidence = committed headless runs (`verify_arena.py` → `Saved/ArenaCheckResult.txt`, verdict `ok=True`) plus a string scan of the map package; everything playtest-shaped stays open.

| Element | In the level today | Status |
| ------- | ------------------ | ------ |
| Brunnfeld spawn (§3) | `PlayerStart` present in the map package; headless `-game` smokes possess a pawn here | done (boot smokes) |
| Reconstruction visuals (§3) | 2 scaffolds + dressing (platform, path slabs, rocks, ruin walls), all `MI_`-dressed (`dressed=31 bare=[]`) | done (#9/#20) |
| Inn / merchant / quest board (§3) | not built — still the placeholders of the feature table | open (post-slice) |
| Respawn anchor (§3) | `Checkpoint` references present in the map package; death respawn = any key (15 s fallback, ADR-011) — the inn marker stays a future nicety | checkpoint flow done; inn = placeholder |
| Klingenhof training (§4) | blade rack (CC0, scale 4.0) + 3 practice blades on `MI_PracticeBlade` + 3 training dummies | done (#20/#24) |
| Grauwaldrand enemies (§5) | spawners zoned x 14000 / 15400 (+ x 17800 at the Ruine approach), counts 2 + 1 inside the §5 ranges, `EnemyClass = Enemy` placeholder mannequin — "Scavenger beast" not authored yet | placeholder done; real enemy open |
| Ruine teaser (§6) | `BladeMonument` + warm 3500 K accent light on the niche, zero Artifact pickups | done (teaser only) |
| Ruine interior structure (§6) | **not built** — the separate `L_StartingReach_Ruine.umap` is a template leftover (ThirdPerson/GameMode strings, no `PlayerStart`), quarantined by ADR-014a → backlog #26 | open (#26) |
| Landmarks (spec §3) | well, 3 dummies, gate arch, blade monument placed — *visibility from zone entry* needs a playtest look | placement done; visibility → #2 |
| Progression path (§7) | unverified — needs PIE traversal | open (#2) |
| Per-zone atmosphere (§6) | one directional light for the whole map; Brunnfeld-warm / wilderness-cool / ruin-dim needs an atmosphere pass | open (backlog candidate) |

Terms and threat tiers in this document were checked against `../terminology.md` (Band 4 §19): Starting Reach / Brunnfeld-Gürtel, Safe → Low → Medium, and the place types match one-to-one. Band 4 §17's four working names match §3–§6 exactly; approvals still live in `../open-questions.md`.

---

## 11. Consistency Checklist

Before adding content to this region, verify (Band 4 §14):

- [ ] Does not contradict `lore/lore-anchor.*.md`
- [ ] Does not contradict Band 7 themes
- [ ] Location name not duplicated elsewhere
- [ ] Enemy fits threat tier
- [ ] Uncertainty logged in `open-questions.md`
- [x] Terms and threat tiers match `../terminology.md` (Band 4 §19, checked 2026-09-25 — see §10.1)
- [x] Cross-references present: `../terminology.md`, `../open-questions.md`, `../test-arena-spec.md`, `../factions/initial-factions.md`
