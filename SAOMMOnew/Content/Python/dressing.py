"""Dressing pass for L_StartingReach: platform, paths, rocks, ruine walls.

Run headless: UnrealEditor-Cmd.exe <uproject> -ExecutePythonScript=<this>
Only ADDS StaticMeshActors, then saves. Safe to re-run (no dedupe!).
"""
import traceback

OUT = "C:/Users/Simon/AppData/Local/Temp/opencode/dressing_result.txt"
MAP_PATH = "/Game/Levels/StartingReach/L_StartingReach"

BOXES = [
    # Klingenhof training platform (30x30m, 30cm step).
    ("Platform_Klingenhof", (6000.0, 0.0, 15.0), (30.0, 30.0, 0.3)),
    # Path slabs Brunnfeld -> Klingenhof.
    ("Path_01", (750.0, 0.0, 5.0), (6.0, 3.0, 0.1)),
    ("Path_02", (2250.0, 0.0, 5.0), (6.0, 3.0, 0.1)),
    ("Path_03", (3750.0, 0.0, 5.0), (6.0, 3.0, 0.1)),
    ("Path_04", (5250.0, 0.0, 5.0), (6.0, 3.0, 0.1)),
    # Path Klingenhof -> Grauwaldrand.
    ("Path_05", (7500.0, 0.0, 5.0), (6.0, 3.0, 0.1)),
    ("Path_06", (9000.0, 0.0, 5.0), (6.0, 3.0, 0.1)),
    ("Path_07", (10500.0, 0.0, 5.0), (6.0, 3.0, 0.1)),
    ("Path_08", (12000.0, 0.0, 5.0), (6.0, 3.0, 0.1)),
    ("Path_09", (13000.0, 0.0, 5.0), (6.0, 3.0, 0.1)),
    # Grauwaldrand rocks (combat cover).
    ("Rock_01", (13200.0, 900.0, 100.0), (3.0, 2.5, 2.0)),
    ("Rock_02", (14800.0, -700.0, 80.0), (2.0, 2.0, 1.6)),
    ("Rock_03", (13600.0, -1100.0, 120.0), (2.5, 3.0, 2.4)),
    ("Rock_04", (14400.0, 1000.0, 90.0), (1.8, 1.8, 1.8)),
    # Ruine teaser wall chunks.
    ("RuineWall_01", (19500.0, -800.0, 150.0), (6.0, 0.8, 3.0)),
    ("RuineWall_02", (20500.0, 800.0, 150.0), (6.0, 0.8, 3.0)),
]
lines = []


def log(msg):
    lines.append(msg)
    print("[DRESSING] " + msg)


success = False
try:
    import unreal
    ok = unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH)
    log("load -> %s" % ok)
    if ok:
        subsys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        sm_cls = unreal.load_class(None, "/Script/Engine.StaticMeshActor")
        cube = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
        n = 0
        if sm_cls and cube:
            for label, loc, scale in BOXES:
                try:
                    a = subsys.spawn_actor_from_class(
                        sm_cls, unreal.Vector(*loc), unreal.Rotator(0.0, 0.0, 0.0))
                    a.set_actor_label(label)
                    comp = a.get_component_by_class(unreal.StaticMeshComponent)
                    comp.set_static_mesh(cube)
                    a.set_actor_scale3d(unreal.Vector(*scale))
                    n += 1
                except Exception:
                    log("SPAWN-FAIL %s\n%s" % (label, traceback.format_exc()))
        log("placed=%d/%d" % (n, len(BOXES)))
        try:
            saved = unreal.EditorAssetLibrary.save_asset(MAP_PATH)
            log("save -> %s" % saved)
            success = bool(saved and n == len(BOXES))
        except Exception:
            log("SAVE-EXC\n%s" % traceback.format_exc())
except Exception:
    lines.append("TOP-FAIL\n" + traceback.format_exc())

with open(OUT, "w") as f:
    f.write("success=%s\n" % success)
    f.write("\n".join(lines) + "\n")
print("[DRESSING] done success=%s" % success)
try:
    import unreal as _u
    _w = _u.EditorLevelLibrary.get_editor_world()
    _u.SystemLibrary.execute_console_command(_w, "quit")
except Exception:
    pass
