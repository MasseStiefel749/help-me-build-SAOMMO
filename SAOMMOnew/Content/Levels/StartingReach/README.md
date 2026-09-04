# Starting Reach — L_StartingReach Blockout

**Level:** `Content/Levels/StartingReach/L_StartingReach.umap` (to create in Editor)
**Spec:** `ki integration/Docs/World/test-arena-spec.md` + `regions/01-starting-reach.md`
**GameMode override:** `BP_SAOMMOGameMode` (World Settings → GameMode Override)
**Purpose:** Vertical slice blockout — Band 3 §20 loop

## Quick Create (Editor)

1. File → New Level → Basic (or Open World template without World Partition).
2. Save as `Content/Levels/StartingReach/L_StartingReach`.
3. World Settings → GameMode Override = `BP_SAOMMOGameMode`.
4. World Settings → Game State / Player Controller = `BP_SAOMMOPlayerController` via GameMode.
5. Build → Build All Levels, place NavMeshBoundsVolume covering Grauwaldrand+Ruine.

## Zones (per test-arena-spec §2)

| Zone | Size guide | Coords (suggested, origin at Brunnfeld center) | Landmark |
|------|------------|-----------------------------------------------|----------|
| **Brunnfeld** (safe) | 50×50m | 0,0 — flat, 2-m walls/scaffolding perimeter, spawn at (0,0,100) facing north | Well/fountain at (0,0) |
| **Klingenhof** (training) | 30×30m | 60,0 (east of Brunnfeld via 8-m road) — flat, 4× `SM_Dummy` / posts, weapon pickup at (60,5) | Training posts |
| **Grauwaldrand** (wilderness) | 80×80m | 60,80 (north), broken gate arch at (60,35), 3× boulders + 2× wall ruins for cover | Broken gate arch |
| **Ruine des Ersten Griffs** (dungeon teaser) | 40×60m interior | 60,160 — collapsed dome entrance at (60,125), linear 3-room hall, sealed niche at (60,190) | Dome/blade monument |

Total footprint ~ 170×240m — one level, **no World Partition** (Band 4 §17).

### Blockout Mesh Kit (use `LevelPrototyping` / `Content/ThirdPerson` as placeholders)
- Floor: `SM_Cube` scaled (or `ModelingTools` planes)
- Walls: `SM_Cube` 2-m thick or `Cube` brushes
- Landmarks: cylinders (well), cubes (arch), spheres (artifact niche light)
- PlayerStart: 1× in Brunnfeld (PlayerStart tag `Brunnfeld_Spawn`), 1× checkpoint volume at Klingenhof (optional)
- NavMeshBoundsVolume: 1× covering Grauwaldrand+Ruine (green nav visible)
- Light: Directional + SkyLight + 1× Point Light accent on sealed niche (intensity 3000, warm)
- KillZ: `KillZVolume` at Z=-5000

## Actor Placement (Slice Minimum)

| Actor | Class | Location | Notes |
|-------|-------|----------|-------|
| PlayerStart | `PlayerStart` | Brunnfeld (0,0,100) | |
| 2–4 enemies | `BP_SAOEnemy` | Grauwaldrand random (40-80,70-120) | `DetectRange=1000, AttackRange=120` |
| 1–2 enemies | `BP_SAOEnemy` | Ruine outer hall | |
| Sword pickup (optional) | `BP_SAOSword` placed | Klingenhof (60,5,30) | If not auto-spawned; else rely on `SAOMMOCharacter::EquippedSword` |
| Sealed niche | Static mesh + point light | Ruine end (60,190,80) | No collision to artifact, just teaser |
| WorldSubsystem check | — | — | In Level Blueprint on BeginPlay call `GetWorldSubsystem SAOMMOWorldSubsystem → SetCurrentRegion "Starting Reach"` |

## Lighting Guides (VR-readable, Band 5 §10)

- Brunnfeld: Temp 5500K warm, Lumen GI 1, no flicker
- Grauwaldrand: cooler 7000K, dappled via tree shadows (blockout cubes)
- Ruine: dim exposure -1 stop, accent niche 2700K

## Verification (mirrors test-arena-spec §8)

- [ ] Play in Editor → spawn Brunnfeld, walk road to Klingenhof without cheats
- [ ] Toggle FP/TP (key bound to `IA_ToggleCamera`) without pawn recreate
- [ ] Pick up / see sword on hand (socket `hand_rSocket` or mesh fallback)
- [ ] Swing (`IA_Attack` or fast swipe) → `SAOMMOCombatComponent` arms 0.25s, `OverlapMultiByChannel ECC_Pawn` hits
- [ ] Hit enemy → `ASAOEnemy::TakeDamage` 1 dmg, 3 hits kill (MaxHealth=3), respawn works
- [ ] Ruine niche visible but not obtainable
- [ ] VR preview: hands visible via `ASAOVRCharacter` anchors (if using VR pawn)

## Next Commits

```text
Add Starting Reach test arena blockout (world layout)
```
then
```text
Starting Reach vertical slice: town, training, wilderness, ruin teaser
```
