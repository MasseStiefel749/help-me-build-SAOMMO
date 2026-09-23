# SAOMMO — ADR-013: Headless Blockout Build of L_StartingReach

**Date:** 2026-09-04
**Status:** Active
**Builds on:** ADR-008 (slice), ADR-012 (naming/redirects)
**Bands:** B3 test-arena-spec, B4 Starting Reach, B6 headless workflow

## Context

`L_StartingReach` was an empty copy of `Lvl_ThirdPerson` (1 actor, no
lights) and could not even be saved. All work done headless via
`UnrealEditor-Cmd` + Python (`Content/Python/build_blockout.py`), no GUI.

## Decisions

### ADR-013a — Map rebuilt, 17 actors, verified

Fresh `L_StartingReach.umap` (non-partitioned; World Partition deferred per
Band 1): ground slab (top z=0, x −3500..21500), 3 training dummies at
Klingenhof, broken-arch gate, blade monument at the Ruine teaser,
`Spawn_Brunnfeld` PlayerStart + Checkpoint, `Pickup_Herb_Klingenhof`
(HealthHerb x2 via new `AItemPickup::Configure`), two spawners
(Grauwaldrand 3, Ruine approach 2, native `Enemy` class), Sun + SkyLight +
SkyAtmosphere, NavMesh bounds (auto-built RecastNavMesh observed),
World-Settings GameMode override. Verified: reload shows 18 actors
(+RecastNavMesh), 0 alien refs, re-save clean.

### ADR-013b — UClass names never carry the C++ prefix (lesson)

Native `AEnemy` registers as `/Script/SAOMMOnew.Enemy`, `ASword` as
`...Sword`, structs likewise (`FInventoryItem` → `...InventoryItem`);
only `UENUM`s keep `E` (`...EItemType`). Proven by spawn/load probes after
a long misdiagnosis as "missing game classes". Consequences:
- Editor scripts must query **prefix-less** native paths.
- Legacy `ActiveClassRedirects` Old/New names are therefore correct
  prefix-less (matching TP_ThirdPerson style).
- `AItemPickup::Configure(FName, int32)` added so scripts can set payloads.

### ADR-013c — Copied-map repair notes

- The copied map referenced `Lvl_ThirdPerson`'s `WorldDataLayers` actor;
  saving failed with "Illegal reference to private object". Destroy+GC is
  NOT enough (the property keeps the alien alive).
- `ULevel::SetWorldDataLayers` asserts null-or-same, and the member is
  private to scripts → added `UGameWorldSubsystem::RepairLevelDataLayers`
  (public setter wrapper; currently unused, kept as utility).
- Fix used: evict → delete → rescan → GC → `new_level` (fails while the
  deleted package is still known — purge first) → rebuild → save.
- `-run=pythonscript` was NOT the problem; `-ExecutePythonScript` runs
  post-init and is the correct vehicle. (`MapCheck` commandlet does not
  exist in 5.8.)

## Verify

- `Content/Python/build_blockout.py` result: `success=True`, `placed=17`,
  `save_asset=True`; independent reload run: 18 actors, re-save clean.
- `UnrealBuildTool SAOMMOnewEditor + SAOMMOnew Win64 Development` → Succeeded.

## Still open (see ADR-014)

- 11 BPs + `IMC_SAOMMO` do not load (file-specific, pre-existing); spawners
  and GameMode use native fallbacks until then.
- Packaged-game BufferReader crash unchanged (needs cook investigation).
