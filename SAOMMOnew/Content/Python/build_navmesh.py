"""Backlog #17: resize bounds + build + save navigation for L_StartingReach.

History:
  v1: `RebuildNavigation` exists but Build() refused: flags 0x20 =
      ENavigationBuildLock::AsyncLoadLock, added by DoInitialSetup() when
      bWaitForAsyncLoadingBeforeBuildingNavigationAutomatically (UPROPERTY
      config, UCLASS config=Engine); released only via FTSTicker (>=16 ticks
      + 2s) which python commandlets never pump.
  v2: config override recipe works, Build() blocks itself
      (RebuildAll + EnsureBuildCompletion -> AsyncTask::EnsureCompletion, no
      engine tick needed), nav saved (umap +25KB) - BUT NavMeshBoundsVolume
      only covered x 3000..15000: spawn (-1500), spawners 15400/17800 and
      monument (20000) had no nav; queries need Build() first because
      ProcessRegistrationCandidates() never runs in a non-ticking commandlet.

v3: resize the volume to the full playfield (x -2000..21000, y +/-3000,
    z -400..800), rebuild, verify spawn->monument path, save via save_map.

Launch recipe (transient DefaultEngine.ini override, restore afterwards):
    [/Script/NavigationSystem.NavigationSystemV1]
    bWaitForAsyncLoadingBeforeBuildingNavigationAutomatically=False

Writes <Project>/Saved/NavBuildResult.txt.
"""
import os
import time
import traceback

MAP_PATH = "/Game/Levels/StartingReach/L_StartingReach"
PROJ_DIR = os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.abspath(__file__))))
UMAP = os.path.join(PROJ_DIR, "Content", "Levels", "StartingReach",
                    "L_StartingReach.umap")

# desired bounds volume box (world space, Unreal units)
WANT_CENTER = (9500.0, 0.0, 200.0)
WANT_EXTENT = (11500.0, 3000.0, 600.0)

lines = []


def log(msg):
    lines.append(msg)
    print("[NAV] " + str(msg))


def umap_stat(tag):
    try:
        st = os.stat(UMAP)
        log("umap %s: size=%d mtime=%s" % (tag, st.st_size,
                                           time.strftime("%H:%M:%S",
                                                         time.localtime(st.st_mtime))))
        return st.st_size
    except Exception as exc:
        log("umap stat fail (%s): %s" % (tag, exc))
        return -1


def safe_call(obj, name, args=()):
    try:
        return getattr(obj, name)(*args)
    except Exception as exc:
        return "EXC: %s" % exc


def vec(v):
    try:
        return "(%.0f, %.0f, %.0f)" % (v.x, v.y, v.z)
    except Exception:
        return str(v)


def main():
    import unreal

    result_path = unreal.Paths.project_saved_dir() + "NavBuildResult.txt"
    success = False
    try:
        size_before = umap_stat("before")
        ok_load = unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH)
        log("load -> %s" % ok_load)
        if not ok_load:
            raise RuntimeError("map load failed")

        try:
            world = unreal.UnrealEditorSubsystem().get_editor_world()
        except Exception:
            world = unreal.EditorLevelLibrary.get_editor_world()
        navsys = unreal.NavigationSystemV1.get_navigation_system(world)
        log("world=%s navsys=%s" % (world, navsys))
        if not navsys:
            raise RuntimeError("no navigation system in world")

        eas = unreal.EditorActorSubsystem()
        all_actors = eas.get_all_level_actors()
        vol = next((a for a in all_actors
                    if isinstance(a, unreal.NavMeshBoundsVolume)), None)
        recast = next((a for a in all_actors
                       if isinstance(a, unreal.RecastNavMesh)), None)
        log("volume=%s recast=%s" % (vol, recast))
        if not vol:
            raise RuntimeError("no NavMeshBoundsVolume in level")

        # --- resize bounds volume to the full playfield ---------------------
        origin, extent = vol.get_actor_bounds(False, False)
        log("volume before: origin=%s extent=%s scale=%s" % (
            vec(origin), vec(extent), vec(vol.get_actor_scale3d())))
        scale = vol.get_actor_scale3d()
        if extent.x > 0 and extent.y > 0 and extent.z > 0:
            new_scale = unreal.Vector(
                scale.x * WANT_EXTENT[0] / extent.x,
                scale.y * WANT_EXTENT[1] / extent.y,
                scale.z * WANT_EXTENT[2] / extent.z)
            vol.set_actor_scale3d(new_scale)
        vol.set_actor_location(unreal.Vector(*WANT_CENTER), False, False)
        origin2, extent2 = vol.get_actor_bounds(False, False)
        log("volume after: origin=%s extent=%s scale=%s" % (
            vec(origin2), vec(extent2), vec(vol.get_actor_scale3d())))
        log("volume covers x %.0f..%.0f y %.0f..%.0f" % (
            origin2.x - extent2.x, origin2.x + extent2.x,
            origin2.y - extent2.y, origin2.y + extent2.y))

        # --- build (blocks until finished) ---------------------------------
        log("pre is_navigation_being_built -> %s" %
            safe_call(navsys, "is_navigation_being_built", (world,)))
        unreal.SystemLibrary.execute_console_command(world, "RebuildNavigation")
        log("console 'RebuildNavigation' executed")
        log("post is_navigation_being_built -> %s" %
            safe_call(navsys, "is_navigation_being_built", (world,)))
        log("post is_navigation_being_built_or_locked -> %s" %
            safe_call(navsys, "is_navigation_being_built_or_locked", (world,)))

        # --- functional checks ------------------------------------------------
        # K2_ProjectPointToNavigation python sig (out-params hoisted to return):
        #   (world_context_object, point, nav_data, filter_class, query_extent)
        if recast:
            for name, x in (("spawn", -1500.0), ("center", 9000.0),
                            ("monument", 20000.0)):
                try:
                    r = navsys.project_point_to_navigation(
                        world, unreal.Vector(x, 0.0, 300.0), recast, None,
                        unreal.Vector(1000.0, 1000.0, 1000.0))
                    log("project %s -> %s" % (name, r))
                except Exception as exc:
                    log("project %s exc: %s" % (name, exc))

        # random-point oracle: returns a nav point iff walkable polys exist
        for args in ((world, unreal.Vector(9000.0, 0.0, 100.0), 3000.0),
                     (world, unreal.Vector(9000.0, 0.0, 100.0), 3000.0, recast),
                     (world, unreal.Vector(9000.0, 0.0, 100.0), 3000.0,
                      recast, None)):
            try:
                r = navsys.get_random_point_in_navigable_radius(*args)
                log("random point -> %s" % (r,))
                break
            except TypeError as exc:
                log("random point needs more args: %s" % exc)
            except Exception as exc:
                log("random point exc: %s" % exc)
                break

        # z=60 sits inside any sane projection extent; z=300 kept as control
        for label, a, b in (
                ("center z60", (8500, 0, 60), (9500, 0, 60)),
                ("spawn->monument z60", (-1500, 0, 60), (20000, 0, 60)),
                ("dummies z60", (5600, 0, 60), (6400, 0, 60)),
                ("center z300", (8500, 0, 300), (9500, 0, 300)),
                ("spawn->monument z300", (-1500, 0, 300), (20000, 0, 300)),
        ):
            p = safe_call(navsys, "find_path_to_location_synchronously",
                          (world, unreal.Vector(*a), unreal.Vector(*b)))
            if hasattr(p, "is_valid"):
                log("path %s -> valid=%s partial=%s points=%d len=%s" % (
                    label, p.is_valid(), p.is_partial(), len(p.path_points),
                    p.get_path_length()))
            else:
                log("path %s -> %s" % (label, p))

        # --- save (ADR-013d: save_map, never save_asset) ----------------------
        p = safe_call(navsys, "find_path_to_location_synchronously",
                      (world, unreal.Vector(-1500.0, 0.0, 60.0),
                       unreal.Vector(20000.0, 0.0, 60.0)))
        path_ok = bool(hasattr(p, "is_valid") and p.is_valid())
        log("acceptance path spawn->monument valid -> %s" % path_ok)

        saved = unreal.EditorLoadingAndSavingUtils.save_map(world, MAP_PATH)
        log("save_map -> %s" % saved)
        size_after = umap_stat("after")
        grown = size_after > size_before if (size_before > 0 and size_after > 0) else False
        log("umap grew: %s (+%d bytes)" % (grown, size_after - size_before))
        # idempotent re-runs keep the size identical -> success = saved + path
        success = bool(saved) and path_ok

    except Exception:
        log("TOP-FAIL\n" + traceback.format_exc())

    with open(result_path, "w") as f:
        f.write("success=%s\n" % success)
        f.write("\n".join(lines) + "\n")
    print("[NAV] done success=%s" % success)
    return success


if __name__ == "__main__":
    main()
