"""Headless blockout builder for L_StartingReach (ADR-013).

Run: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<this>"
Only ADDS actors to /Game/Levels/StartingReach/L_StartingReach, sets the
GameMode override, saves. Writes a result file for the caller.
"""
import traceback

RESULT_PATH = None  # resolved inside main() to <Project>/Saved/BlockoutResult.txt
MAP_PATH = "/Game/Levels/StartingReach/L_StartingReach"

results = []


def log(msg):
    results.append(msg)
    print("[BLOCKOUT] " + msg)


def main():
    import unreal
    global RESULT_PATH
    RESULT_PATH = unreal.Paths.project_saved_dir() + "BlockoutResult.txt"

    # 0. Fresh asset scan so renamed BPs resolve (cmd sessions use cache).
    try:
        reg = unreal.AssetRegistryHelpers.get_asset_registry()
        reg.search_all_assets(True)
        log("asset rescan done")
    except Exception:
        log("rescan skipped\n%s" % traceback.format_exc())

    # 1. Fresh map: evict ours, delete it, purge registry+memory, recreate.
    # (The copied .umap carries an alien WorldDataLayers ref that can neither
    # be overwritten (engine assert) nor saved (illegal ref). Old file stays
    # recoverable in git.)
    try:
        unreal.EditorLoadingAndSavingUtils.load_map("/Engine/Maps/Templates/Template_Default")
        log("evict: template loaded")
    except Exception:
        log("evict EXC\n%s" % traceback.format_exc())
    try:
        unreal.EditorAssetLibrary.delete_asset(MAP_PATH)
        log("old map deleted")
    except Exception:
        log("delete EXC\n%s" % traceback.format_exc())
    try:
        unreal.AssetRegistryHelpers.get_asset_registry().search_all_assets(True)
        _w = unreal.EditorLevelLibrary.get_editor_world()
        unreal.SystemLibrary.execute_console_command(_w, "obj gc")
        log("rescan+gc done")
    except Exception:
        log("purge EXC\n%s" % traceback.format_exc())
    made = False
    try:
        made = unreal.EditorLevelLibrary.new_level(MAP_PATH)
    except Exception:
        log("new_level EXC\n%s" % traceback.format_exc())
    log("new_level %s -> %s" % (MAP_PATH, made))
    if not made:
        return False
    try:
        ok = unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH)
    except Exception:
        ok = False
        log("load_map EXC\n%s" % traceback.format_exc())
    log("load_map %s -> %s" % (MAP_PATH, ok))
    if not ok:
        return False
    actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    try:
        all_actors = actor_sub.get_all_level_actors()
    except Exception:
        all_actors = []
        log("list actors EXC\n%s" % traceback.format_exc())
    log("level actors before cleanup=%d" % len(all_actors))
    for a in all_actors:
        try:
            path = a.get_path_name()
        except Exception:
            continue
        if "Lvl_ThirdPerson" in path or "/Game/ThirdPerson/" in path:
            try:
                label = a.get_actor_label()
                actor_sub.destroy_actor(a)
                log("destroyed alien %s (%s)" % (label, path))
            except Exception:
                log("destroy FAIL %s\n%s" % (path, traceback.format_exc()))
    # Destroy is deferred until GC: purge now so the save validator no
    # longer sees the illegal cross-map reference.
    try:
        _w = unreal.EditorLevelLibrary.get_editor_world()
        unreal.SystemLibrary.execute_console_command(_w, "obj gc")
        log("obj gc done")
    except Exception:
        log("gc EXC\n%s" % traceback.format_exc())

    def load_cls(path):
        cls = unreal.load_class(None, path)
        log("load_class %s -> %s" % (path, "OK" if cls else "MISS"))
        return cls

    # 2. Resolve classes (BP preferred, native fallback).
    # NOTE (ADR-013): native UClasses register WITHOUT C++ prefixes in
    # this project (AEnemy -> /Script/SAOMMOnew.Enemy). Always query
    # prefix-less native paths.
    bp_enemy = load_cls("/Game/Enemies/BP_Enemy.BP_Enemy_C")
    native_enemy = load_cls("/Script/SAOMMOnew.Enemy")
    enemy_cls = bp_enemy or native_enemy
    bp_gamemode = load_cls("/Game/Blueprints/BP_MainGameMode.BP_MainGameMode_C")
    native_gamemode = load_cls("/Script/SAOMMOnew.MainGameMode")
    gamemode_cls = bp_gamemode or native_gamemode
    spawner_cls = load_cls("/Script/SAOMMOnew.EnemySpawner")
    checkpoint_cls = load_cls("/Script/SAOMMOnew.Checkpoint")
    pickup_cls = load_cls("/Script/SAOMMOnew.ItemPickup")
    sm_actor_cls = load_cls("/Script/Engine.StaticMeshActor")
    playerstart_cls = load_cls("/Script/Engine.PlayerStart")
    dirlight_cls = load_cls("/Script/Engine.DirectionalLight")
    skylight_cls = load_cls("/Script/Engine.SkyLight")
    skyatmo_cls = load_cls("/Script/Engine.SkyAtmosphere")
    try:
        nav_cls = unreal.load_class(None, "/Script/NavigationSystem.NavMeshBoundsVolume")
    except Exception:
        nav_cls = None
    if nav_cls is None:
        try:
            nav_cls = unreal.load_class(None, "/Script/Engine.NavMeshBoundsVolume")
        except Exception:
            nav_cls = None
    log("navmesh class -> %s" % ("OK" if nav_cls else "MISS (skipped)"))

    if not all([spawner_cls, checkpoint_cls, pickup_cls, sm_actor_cls,
                playerstart_cls, enemy_cls, gamemode_cls]):
        log("FATAL: a required class failed to load, aborting")
        return False

    cube_mesh = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
    log("cube mesh -> %s" % ("OK" if cube_mesh else "MISS"))
    if not cube_mesh:
        return False

    placed = []

    def spawn(cls, label, loc, rot=(0.0, 0.0, 0.0)):
        try:
            a = actor_sub.spawn_actor_from_class(
                cls, unreal.Vector(*loc), unreal.Rotator(*rot))
            if a:
                a.set_actor_label(label)
                placed.append(label)
                log("spawned %s" % label)
                return a
            log("SPAWN-NULL %s" % label)
        except Exception:
            log("SPAWN-FAIL %s\n%s" % (label, traceback.format_exc()))
        return None

    def box(label, loc, scale):
        a = spawn(sm_actor_cls, label, loc)
        if a:
            try:
                comp = a.get_component_by_class(unreal.StaticMeshComponent)
                comp.set_static_mesh(cube_mesh)
            except Exception:
                log("MESH-FAIL %s" % label)
            a.set_actor_scale3d(unreal.Vector(*scale))
        return a

    # 3. Ground: one slab, top surface at z=0, x -3500..21500, y +-5500.
    box("Ground_BrunnfeldSlab", (9000.0, 0.0, -100.0), (250.0, 110.0, 2.0))

    # 4. Landmarks (test-arena-spec): dummies, arch, monument.
    for i, dx in enumerate((-400.0, 0.0, 400.0)):
        box("Dummy_%d" % (i + 1), (6000.0 + dx, 300.0, 100.0), (1.0, 1.0, 4.0))
    box("ArchPillar_L", (10000.0, -400.0, 200.0), (1.5, 1.5, 8.0))
    box("ArchPillar_R", (10000.0, 400.0, 200.0), (1.5, 1.5, 8.0))
    box("ArchLintel", (10000.0, 0.0, 620.0), (1.5, 10.0, 1.5))
    box("BladeMonument", (20000.0, 0.0, 300.0), (2.0, 2.0, 12.0))

    # 5. Player start + checkpoint at Brunnfeld spawn, facing +X.
    spawn(playerstart_cls, "Spawn_Brunnfeld", (-1500.0, 0.0, 100.0), (0.0, 0.0, 0.0))
    spawn(checkpoint_cls, "Checkpoint_Brunnfeld", (-1500.0, 0.0, 100.0), (0.0, 0.0, 0.0))

    # 6. Health pickup at Klingenhof (class defaults: HealthHerb x2).
    pickup = spawn(pickup_cls, "Pickup_Herb_Klingenhof", (6000.0, 800.0, 100.0))
    if pickup:
        try:
            pickup.call_method("Configure", ("HealthHerb", 2))
            log("pickup Configure OK")
        except Exception:
            log("pickup Configure skipped\n%s" % traceback.format_exc())

    # 7. Spawners: Grauwaldrand pack + Ruine approach pair.
    for label, loc, max_alive in (("Spawner_Grauwaldrand", (14000.0, 0.0, 150.0), 3),
                                  ("Spawner_RuineApproach", (19500.0, 1500.0, 150.0), 2)):
        s = spawn(spawner_cls, label, loc)
        if s:
            try:
                s.set_editor_property("EnemyClass", enemy_cls)
                s.set_editor_property("MaxAlive", max_alive)
                s.set_editor_property("SpawnRadius", 1200.0)
                s.set_editor_property("SpawnInterval", 5.0)
            except Exception:
                log("SPAWNER-PROP-FAIL %s" % label)

    # 8. Lighting + atmosphere.
    spawn(dirlight_cls, "Sun", (0.0, 0.0, 1000.0), (-55.0, -35.0, 0.0))
    spawn(skylight_cls, "SkyLight", (0.0, 0.0, 500.0))
    spawn(skyatmo_cls, "SkyAtmosphere", (0.0, 0.0, 0.0))

    # 9. NavMesh bounds over the play area.
    if nav_cls:
        nav = spawn(nav_cls, "NavMesh_StartingReach", (9000.0, 0.0, 200.0))
        if nav:
            nav.set_actor_scale3d(unreal.Vector(60.0, 30.0, 6.0))

    # 10. World Settings GameMode override.
    try:
        world = unreal.EditorLevelLibrary.get_editor_world()
        settings = world.get_world_settings()
        settings.set_editor_property("DefaultGameMode", gamemode_cls)
        log("world gamemode -> %s" % gamemode_cls.get_name())
    except Exception:
        log("GAMEMODE-FAIL\n%s" % traceback.format_exc())

    # 11. Save.
    try:
        saved = unreal.EditorAssetLibrary.save_asset(MAP_PATH)
        log("save_asset -> %s" % saved)
    except Exception:
        saved = False
        log("SAVE-FAIL\n%s" % traceback.format_exc())
    log("placed=%d" % len(placed))
    return bool(saved)


try:
    success = main()
except Exception:
    results.append("TOP-FAIL\n" + traceback.format_exc())
    success = False

with open(RESULT_PATH, "w") as f:
    f.write("success=%s\n" % success)
    f.write("\n".join(results) + "\n")
print("[BLOCKOUT] done success=%s" % success)

# Request a clean editor exit (post-init -ExecutePythonScript context).
try:
    import unreal as _u
    _w = _u.EditorLevelLibrary.get_editor_world()
    _u.SystemLibrary.execute_console_command(_w, "quit")
except Exception:
    pass
