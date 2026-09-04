# Starting Reach — World Data Scaffold

**Region:** `ki integration/Docs/World/regions/01-starting-reach.md`  
**Level:** `Content/Levels/StartingReach/L_StartingReach.umap` (main) + `L_StartingReach_Ruine` (sub-level, optional instanced)  
**Spec:** `ki integration/Docs/World/test-arena-spec.md`

## Data Flow (future, post-slice)

```text
DA_StartingReach (DataAsset)
  → Region = "Starting Reach"
  → Zones: Brunnfeld / Klingenhof / Grauwaldrand / Ruine
  → SpawnTables (enemy counts per test-arena-spec §5)
  → Lore tags (reconstruction / blade tradition / artifact teaser)
```

For vertical slice, data is **hardcoded spawns in level** — this folder holds the future DA path so `USAOMMOWorldSubsystem::SetCurrentRegion` has a home.

## Suggested Data Assets (create when needed)

- `DA_StartingReach` (PrimaryDataAsset, holds FSAOItem tables)
- `DT_EnemySpawns` (DataTable, rows: Grauwaldrand 2-4 scavenger, Ruine 1-2)
- `DT_LoreProps` (scaffolding / practice blades / blade motif meshes)

No World Partition / PCG / streaming yet (Band 4 §11-12).
