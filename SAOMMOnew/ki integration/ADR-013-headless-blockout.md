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

### ADR-013d — Level saves must use save_map (lesson, R9)

- `EditorAssetLibrary.save_asset()` reported `True` **without touching the
  `.umap`** (verified via file mtime across five runs; no file lock, no
  read-only flag, no `LogSavePackage` output — root cause unknown, started
  after a routine module rebuild).
- `EditorLoadingAndSavingUtils.save_map(world, asset_path)` writes
  reliably (mtime probe 00:57:57) → **mandatory for level saves in
  headless scripts**; keep `save_asset` for non-level assets only.
- Freshly spawned actors seemed to persist while edits to pre-existing
  actors did not — do not trust `save_asset`'s boolean, verify by mtime or
  a fresh-process reload.
- Actors without a root component (e.g. `AEnemySpawner` before the R9 fix)
  ignore spawn/set locations and report no error. Ctor-created roots are
  present on load after the C++ fix, so old instances only needed
  re-placing, not re-spawning.

### ADR-013e - Headless navmesh build + save (R11, backlog #17)

- `RebuildNavigation` (FNavigationSystemExec) -> `UNavigationSystemV1::Build()`
  exists and blocks synchronously (`RebuildAll` + `EnsureBuildCompletion` ->
  `AsyncTask::EnsureCompletion`), so the build itself needs no engine ticks.
- In python commandlets the build is refused: `flags: 0x20` =
  `ENavigationBuildLock::AsyncLoadLock`, added by `DoInitialSetup()` when
  `bWaitForAsyncLoadingBeforeBuildingNavigationAutomatically` (UPROPERTY
  config, UCLASS config=Engine) && auto-update && EditorMode. The lock is only
  released by an FTSTicker path (>=16 ticks + 2 s) which commandlets never
  pump (frame counter stays 0). Recipe: temporarily append to
  `Config/DefaultEngine.ini` BEFORE launching the commandlet, restore after
  (see build_navmesh.py docstring):
  `[/Script/NavigationSystem.NavigationSystemV1]` /
  `bWaitForAsyncLoadingBeforeBuildingNavigationAutomatically=False`.
- Queries need the build in the same process: `ProcessRegistrationCandidates()`
  runs inside `Build()`; a non-ticking commandlet never registers loaded nav
  data otherwise (all path queries come back invalid).
- Python signatures (UE 5.8): statics do NOT auto-bind the instance -
  `is_navigation_being_built(world)`, `project_point_to_navigation(world,
  point, nav_data, filter_class, query_extent)` (out-params hoisted to the
  return value), `find_path_to_location_synchronously(world, start, end)`.
  `get_actor_bounds` returns a plain `(origin, extent)` tuple;
  `EditorActorSubsystem` has `get_all_level_actors()` (no
  `get_all_actors_of_class`).
- Path endpoints are projected internally with a limited z extent: queries from
  z=60 (floor top at z=0) are valid; synthetic z=300 endpoints fail - not a
  data problem.
- `NavMeshBoundsVolume NavMesh_StartingReach` covered only x 3000..15000;
  resized to x -2000..21000 (scale 60,30,6 -> 115,30,6) so spawn (-1500), all
  three spawners (14000/15400/17800) and monument/ruin (19500..20500) sit on
  nav.
- Verification: `Build total execution time 0.05s`, path Spawn->Monument valid
  (21381uu), `-game` smoke: 0x `SpawnMissingNavigationData` + 0x
  `CrowdFollowing: Unable to find RecastNavMesh` (before: every boot + 16x).

## Verify

- `Content/Python/build_blockout.py` result: `success=True`, `placed=17`,
  `save_asset=True`; independent reload run: 18 actors, re-save clean.
- `UnrealBuildTool SAOMMOnewEditor + SAOMMOnew Win64 Development` → Succeeded.

## Still open (see ADR-014)

- 11 BPs + `IMC_SAOMMO` do not load (file-specific, pre-existing); spawners
  and GameMode use native fallbacks until then.
- Packaged-game BufferReader crash unchanged (needs cook investigation).
