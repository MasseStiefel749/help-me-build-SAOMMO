"""Test-arena dressing pass: Band 4 §20 spec §6/§7 + §8 zone fixes.

Run headless: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<this>"

Idempotent (dedupe by actor label, materials re-applied every run):

  * blockout dressing that dressing.py never got into the map save
    (training platform, path slabs, Grauwaldrand rocks, ruin walls)
  * material assignment MI_Ground037 / MI_Rock063 / MI_Planks009 /
    MI_PracticeBlade (backlog #9 instances + #20 metal MI, Band 5 §9 rule)
  * spec §3 landmark 1: Brunnfeld well (stone)
  * spec §6: warm accent light at the Ruine Artifact niche (BladeMonument)
  * spec §7: scaffolds (Brunnfeld reconstruction tone), blade rack with
    practice blades (Klingenhof)
  * backlog #24: the cube/cylinder blade-rack proxies are retired and
    replaced by the real CC0 meshes (import_cc0_meshes.py): rack at
    Klingenhof (actor scale 4x - katana_stand_01 imports 30x18cm),
    barrel + bench in Brunnfeld; bottom-aligned via world bounds, MI on
    every material slot (Band 5 §9, no material build)
  * spec §5 zones: EnemySpawner spawns inside SpawnRadius around its own
    actor location - both spawners stood at (0,0,0), i.e. inside the safe
    town. Move them into Grauwaldrand / Ruine approach, add a second
    Grauwaldrand spawner (spec: 2-4), set EnemyClass when unset.

Writes <Project>/Saved/ArenaDressResult.txt.
"""
import traceback

MAP_PATH = "/Game/Levels/StartingReach/L_StartingReach"
MI = {
    "ground": "/Game/Materials/MI_Ground037",
    "rock": "/Game/Materials/MI_Rock063",
    "planks": "/Game/Materials/MI_Planks009",
    "metal": "/Game/Materials/MI_PracticeBlade",
}
CUBE = "/Engine/BasicShapes/Cube.Cube"
CYLINDER = "/Engine/BasicShapes/Cylinder.Cylinder"
ENEMY_CLASS = "/Script/SAOMMOnew.Enemy"

# (label, location, scale, material key or None)
BOXES = [
    # Klingenhof training platform (dressing.py).
    ("Platform_Klingenhof", (6000.0, 0.0, 15.0), (30.0, 30.0, 0.3), "planks"),
    # Path slabs Brunnfeld -> Klingenhof (dressing.py).
    ("Path_01", (750.0, 0.0, 5.0), (6.0, 3.0, 0.1), "planks"),
    ("Path_02", (2250.0, 0.0, 5.0), (6.0, 3.0, 0.1), "planks"),
    ("Path_03", (3750.0, 0.0, 5.0), (6.0, 3.0, 0.1), "planks"),
    ("Path_04", (5250.0, 0.0, 5.0), (6.0, 3.0, 0.1), "planks"),
    # Path Klingenhof -> Grauwaldrand.
    ("Path_05", (7500.0, 0.0, 5.0), (6.0, 3.0, 0.1), "planks"),
    ("Path_06", (9000.0, 0.0, 5.0), (6.0, 3.0, 0.1), "planks"),
    ("Path_07", (10500.0, 0.0, 5.0), (6.0, 3.0, 0.1), "planks"),
    ("Path_08", (12000.0, 0.0, 5.0), (6.0, 3.0, 0.1), "planks"),
    ("Path_09", (13000.0, 0.0, 5.0), (6.0, 3.0, 0.1), "planks"),
    # Grauwaldrand rocks (combat cover, dressing.py).
    ("Rock_01", (13200.0, 900.0, 100.0), (3.0, 2.5, 2.0), "rock"),
    ("Rock_02", (14800.0, -700.0, 80.0), (2.0, 2.0, 1.6), "rock"),
    ("Rock_03", (13600.0, -1100.0, 120.0), (2.5, 3.0, 2.4), "rock"),
    ("Rock_04", (14400.0, 1000.0, 90.0), (1.8, 1.8, 1.8), "rock"),
    # Ruine teaser wall chunks (dressing.py).
    ("RuineWall_01", (19500.0, -800.0, 150.0), (6.0, 0.8, 3.0), "rock"),
    ("RuineWall_02", (20500.0, 800.0, 150.0), (6.0, 0.8, 3.0), "rock"),
    # Spec §3.1 landmark: town well (stone).
    ("Well_Brunnfeld", (-400.0, -900.0, 50.0), (2.4, 2.4, 1.0), "rock"),
    # Spec §7: reconstruction scaffolds in Brunnfeld (wood).
    ("Scaffold_Brunnfeld_01_Post1", (180.0, 900.0, 150.0), (0.2, 0.2, 3.0), "planks"),
    ("Scaffold_Brunnfeld_01_Post2", (420.0, 900.0, 150.0), (0.2, 0.2, 3.0), "planks"),
    ("Scaffold_Brunnfeld_01_Deck", (300.0, 900.0, 315.0), (2.8, 1.0, 0.15), "planks"),
    ("Scaffold_Brunnfeld_02_Post1", (560.0, -950.0, 150.0), (0.2, 0.2, 3.0), "planks"),
    ("Scaffold_Brunnfeld_02_Post2", (760.0, -950.0, 150.0), (0.2, 0.2, 3.0), "planks"),
    ("Scaffold_Brunnfeld_02_Deck", (660.0, -950.0, 315.0), (2.4, 1.0, 0.15), "planks"),
    # Spec §7: practice blades at Klingenhof, dressed with the metal MI
    # (backlog #20). The rack itself is a real CC0 mesh since backlog #24
    # - see MESH_PROPS + RETIRE below.
    ("PracticeBlade_01", (5500.0, -360.0, 85.0), (0.06, 0.06, 1.1), "metal"),
    ("PracticeBlade_02", (5600.0, -360.0, 85.0), (0.06, 0.06, 1.1), "metal"),
    ("PracticeBlade_03", (5700.0, -360.0, 85.0), (0.06, 0.06, 1.1), "metal"),
]

# Backlog #24: cube/cylinder proxies replaced by the CC0 blade rack mesh.
# Destructive on purpose (Band 6 §21): named here, in task row #24 and in
# the commit message. The practice blades stay (spec §7, metal MI).
RETIRE = [
    "BladeRack_Klingenhof_Post1",
    "BladeRack_Klingenhof_Post2",
    "BladeRack_Klingenhof_Bar",
]

# (label, mesh asset, x/y/ground-z, actor scale, material key).
# ground-z = target plane for the mesh BOTTOM: the prop is world-bounds
# aligned after scaling, so pivot differences cannot sink or float it.
MESH_PROPS = [
    # Klingenhof platform top is z=30 (cube 100cm * 0.3 scale + center 15).
    ("BladeRack_Klingenhof", "/Game/Props/SM_BladeRack_Klingenhof",
     (5600.0, -300.0, 30.0), 4.0, "planks"),
    # Brunnfeld ground slab top is z=0 (build_blockout.py, scale z 2.0,
    # center z -100). Barrel next to the reconstruction scaffolds ...
    ("Barrel_Brunnfeld", "/Game/Props/SM_Barrel_Brunnfeld",
     (950.0, -700.0, 0.0), 1.0, "planks"),
    # ... bench at the town well (spec §3.1 landmark).
    ("Bench_Brunnfeld", "/Game/Props/SM_Bench_Brunnfeld",
     (-400.0, -1450.0, 0.0), 1.0, "planks"),
]

# Existing actors that get their MI re-applied every run.
MATERIAL_MAP = {
    "Ground_BrunnfeldSlab": "ground",
    "BladeMonument": "rock",
}

# label -> zone movement / creation (spec §5).
SPAWNER_MOVES = {
    "Spawner_Grauwaldrand": (14000.0, 200.0, 100.0),
    "Spawner_RuineApproach": (17800.0, 0.0, 100.0),
}
SPAWNER_NEW = {
    "Spawner_Grauwaldrand_02": (15400.0, -600.0, 100.0),
}

ACCENT_LABEL = "Light_RuineAccent"
ACCENT_LOC = (20000.0, 0.0, 900.0)

lines = []


def log(msg):
    lines.append(msg)
    print("[DRESS] " + msg)


def set_prop(obj, name, value):
    """set_editor_property with a loud fallback log."""
    try:
        obj.set_editor_property(name, value)
        return True
    except Exception as exc:
        log("PROP-FAIL %s.%s: %s" % (obj.get_name() if hasattr(obj, "get_name") else obj, name, exc))
        return False


def main():
    import unreal

    result_path = unreal.Paths.project_saved_dir() + "ArenaDressResult.txt"
    success = False
    try:
        ok_load = unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH)
        log("load -> %s" % ok_load)
        if not ok_load:
            raise RuntimeError("map load failed")

        subsys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        by_label = {}
        for a in subsys.get_all_level_actors():
            try:
                by_label[a.get_actor_label()] = a
            except Exception:
                pass

        mats = {k: unreal.EditorAssetLibrary.load_asset(p) for k, p in MI.items()}
        if not all(mats.values()):
            raise RuntimeError("MI asset missing: %s" %
                               {k: MI[k] for k, v in mats.items() if v is None})
        cube = unreal.load_asset(CUBE)
        cylinder = unreal.load_asset(CYLINDER)
        sm_cls = unreal.load_class(None, "/Script/Engine.StaticMeshActor")
        if not (cube and cylinder and sm_cls):
            raise RuntimeError("basic shape/class missing")

        # 1) Dressing boxes (dedupe by label), material applied when placed or re-run.
        placed = skipped = 0
        for label, loc, scale, mat_key in BOXES:
            actor = by_label.get(label)
            first_place = actor is None
            if first_place:
                try:
                    actor = subsys.spawn_actor_from_class(
                        sm_cls, unreal.Vector(*loc), unreal.Rotator(0.0, 0.0, 0.0))
                    actor.set_actor_label(label)
                    actor.set_actor_scale3d(unreal.Vector(*scale))
                    by_label[label] = actor
                    placed += 1
                except Exception:
                    log("SPAWN-FAIL %s\n%s" % (label, traceback.format_exc()))
                    continue
            else:
                skipped += 1
            comp = actor.get_component_by_class(unreal.StaticMeshComponent)
            if comp is not None:
                if first_place:
                    comp.set_static_mesh(cylinder if label.startswith("Well_") else cube)
                if mat_key:
                    comp.set_material(0, mats[mat_key])
        log("boxes placed=%d skipped-existing=%d/%d" % (placed, skipped, len(BOXES)))

        # 1b) Backlog #24: retire the cube proxies the CC0 rack replaces.
        retired = 0
        for label in RETIRE:
            stale = by_label.pop(label, None)
            if stale is not None:
                subsys.destroy_actor(stale)
                retired += 1
                log("retired proxy " + label)
        log("proxies retired=%d/%d" % (retired, len(RETIRE)))

        # 1c) Backlog #24: real CC0 meshes, bottom-aligned to ground z,
        # MI on every material slot (Band 5 §9 - slots stay consistent
        # even if an import ever yields more than one).
        mesh_placed = mesh_kept = 0
        for label, mesh_path, loc, scale, mat_key in MESH_PROPS:
            mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
            if mesh is None:
                log("MESH-MISSING " + mesh_path)
                continue
            actor = by_label.get(label)
            first_place = actor is None
            if first_place:
                try:
                    actor = subsys.spawn_actor_from_class(
                        sm_cls, unreal.Vector(*loc), unreal.Rotator(0.0, 0.0, 0.0))
                    actor.set_actor_label(label)
                    by_label[label] = actor
                    mesh_placed += 1
                except Exception:
                    log("MESH-SPAWN-FAIL %s\n%s" % (label, traceback.format_exc()))
                    continue
            else:
                mesh_kept += 1
            comp = actor.get_component_by_class(unreal.StaticMeshComponent)
            if comp is None:
                log("MESH-NO-COMP " + label)
                continue
            comp.set_static_mesh(mesh)
            actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
            try:
                origin, extent = actor.get_actor_bounds(False, False)
                bottom = origin.z - extent.z
                dz = loc[2] - bottom
                actor.set_actor_location(
                    unreal.Vector(loc[0], loc[1], loc[2] + dz), False, False)
                log("mesh %s placed=%s scale=%.1f bottom %.1f -> %.1f" % (
                    label, first_place, scale, bottom, loc[2]))
            except Exception:
                log("MESH-ALIGN-FAIL %s\n%s" % (label, traceback.format_exc()))
            mat = mats[mat_key]
            for slot in range(max(1, comp.get_num_materials())):
                comp.set_material(slot, mat)
        log("mesh props placed=%d kept-existing=%d/%d" % (
            mesh_placed, mesh_kept, len(MESH_PROPS)))

        # 2) Re-apply MI on pre-existing landmark props.
        for label, mat_key in MATERIAL_MAP.items():
            actor = by_label.get(label)
            if actor is None:
                log("MISSING existing actor: " + label)
                continue
            comp = actor.get_component_by_class(unreal.StaticMeshComponent)
            if comp is not None:
                comp.set_material(0, mats[mat_key])
                log("material %s -> %s" % (label, MI[mat_key]))

        # 3) Spec §6: warm accent light over the Ruine Artifact niche.
        accent = by_label.get(ACCENT_LABEL)
        if accent is None:
            light_cls = unreal.load_class(None, "/Script/Engine.PointLight")
            accent = subsys.spawn_actor_from_class(
                light_cls, unreal.Vector(*ACCENT_LOC), unreal.Rotator(0.0, 0.0, 0.0))
            accent.set_actor_label(ACCENT_LABEL)
            by_label[ACCENT_LABEL] = accent
            log("accent light spawned")
        lcomp = accent.get_component_by_class(unreal.PointLightComponent)
        if lcomp is None:
            log("ACCENT-FAIL no PointLightComponent")
        else:
            set_prop(lcomp, "intensity", 1500.0)
            set_prop(lcomp, "attenuation_radius", 1500.0)
            # Warm tone (§6) via color temperature: LightColor is an FColor
            # struct the python binding refuses to nativize from LinearColor,
            # while temperature/usage are plain float/bool properties.
            set_prop(lcomp, "use_temperature", True)
            set_prop(lcomp, "temperature", 3500.0)
            log("accent warm 3500K set")

        # 4) Spec §5: spawners into their zones + EnemyClass when unset.
        # Instances serialized before the root-component fix carry
        # RootComponent=null, which overwrites the constructor root on load -
        # set_actor_location then fails silently (no exception). Detect by
        # read-back and re-spawn offenders with the fixed class; spawners hold
        # no per-instance data beyond EnemyClass (CDO default), so nothing is
        # lost by replacing them.
        enemy_cls = unreal.load_class(None, ENEMY_CLASS)
        spawner_cls = unreal.load_class(None, "/Script/SAOMMOnew.EnemySpawner")
        all_moves = dict(SPAWNER_MOVES)
        all_moves.update(SPAWNER_NEW)

        def try_place(actor, loc):
            v = unreal.Vector(*loc)
            try:
                actor.set_actor_location(v, False, False)
            except Exception:
                try:
                    actor.set_actor_location(v)
                except Exception as exc:
                    log("loc call failed: %s" % exc)
                    return False
            cur = actor.get_actor_location()
            ok = (abs(cur.x - loc[0]) < 1.0 and abs(cur.y - loc[1]) < 1.0
                  and abs(cur.z - loc[2]) < 1.0)
            log("place check %s -> (%.0f, %.0f, %.0f) ok=%s" % (
                actor.get_actor_label(), cur.x, cur.y, cur.z, ok))
            return ok

        for label, loc in sorted(all_moves.items()):
            actor = by_label.get(label)
            if actor is not None and not try_place(actor, loc):
                subsys.destroy_actor(actor)
                log("replaced rootless spawner " + label)
                actor = None
            if actor is None:
                if spawner_cls is None:
                    log("SPAWNER-CLASS MISSING: " + label)
                    continue
                actor = subsys.spawn_actor_from_class(
                    spawner_cls,
                    unreal.Vector(*loc), unreal.Rotator(0.0, 0.0, 0.0))
                actor.set_actor_label(label)
                by_label[label] = actor
                log("spawner created at zone: " + label)

        for label, actor in sorted(by_label.items()):
            if not hasattr(actor, "get_alive_count"):
                continue  # not an AEnemySpawner
            cur = actor.get_editor_property("enemy_class")
            if cur is None and enemy_cls is not None:
                set_prop(actor, "enemy_class", enemy_cls)
                log("EnemyClass set on " + label)
            elif cur is None:
                log("EnemyClass STILL NONE on " + label)
            else:
                log("EnemyClass ok on %s (%s)" % (label, cur.get_name()))

        # 5) Save. EditorAssetLibrary.save_asset() reported True without ever
        # touching the .umap after the 00:40 rebuild (verified via mtime), so
        # use the level-specific save_map(world, path) which demonstrably
        # writes (mtime probe 00:57:57).
        w = unreal.EditorLevelLibrary.get_editor_world()
        saved = unreal.EditorLoadingAndSavingUtils.save_map(w, MAP_PATH)
        log("save_map -> %s" % saved)
        success = bool(saved)

    except Exception:
        log("TOP-FAIL\n" + traceback.format_exc())

    with open(result_path, "w") as f:
        f.write("success=%s\n" % success)
        f.write("\n".join(lines) + "\n")
    print("[DRESS] done success=%s" % success)
    return success


if __name__ == "__main__":
    main()
