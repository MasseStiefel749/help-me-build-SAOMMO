"""Diff every readable property of a WORKING mesh vs a NON-WORKING mesh.

Backlog #24 collision forensics: in a fresh (non-ticking) process the CC0
props never get a physics body - get_closest_point_on_collision -> -1,
world sweep and component line trace all MISS - while a freshly spawned
/Engine/BasicShapes/Cube hits in the same process, with identical
component state (BlockAll / ECR_BLOCK / QUERY_AND_PHYSICS) and identical
AggGeom shape on disk (box=1, convex=0, flag=CTF_USE_DEFAULT).

So the difference must live in the asset data that does NOT survive the
python round trip to disk. This script dumps every non-callable, readable
property (dir() + get_editor_property) of:
    mesh, body_setup, box_elems[0], spawned component, body_instance
for both assets and writes the two dumps side by side to
<Saved>/MeshCollisionDiff.txt for eyeball diffing.

Run: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<this>"
"""
import traceback

WORKING = "/Engine/BasicShapes/Cube.Cube"
BROKEN = "/Game/Props/SM_BladeRack_Klingenhof"

lines = []


def log(msg):
    lines.append(msg)
    print("[DIFF] " + msg)


def dump(obj, label):
    import unreal
    log("--- %s (%s) ---" % (label, type(obj).__name__))
    try:
        log("  class=%s" % obj.get_class().get_name())
    except Exception:
        pass
    names = []
    for n in dir(obj):
        if n.startswith("__"):
            continue
        try:
            v = getattr(obj, n)
        except Exception:
            continue
        if callable(v):
            continue
        names.append(n)
    for n in sorted(names):
        try:
            v = obj.get_editor_property(n)
            s = str(v)
            log("  %s = %s" % (n, s[:160]))
        except Exception as exc:
            log("  %s = <unreadable: %s>" % (n, str(exc)[:80]))


def dump_box_elems(bs, label):
    import unreal
    try:
        agg = bs.get_editor_property("agg_geom")
    except Exception as exc:
        log("%s agg_geom unreadable: %s" % (label, exc))
        return
    for arr_name in ("box_elems", "convex_elems", "sphyl_elems",
                     "tape_elems", "tris_elems"):
        try:
            arr = agg.get_editor_property(arr_name) or []
        except Exception:
            continue
        log("%s %s: %d" % (label, arr_name, len(arr)))
        for i, el in enumerate(arr):
            fields = {}
            for f in ("center", "x", "y", "z", "orientation", "rotation",
                      "radius", "half_height", "valid"):
                try:
                    fields[f] = str(el.get_editor_property(f))[:60]
                except Exception:
                    pass
            log("  [%d] %s" % (i, fields))


COOKED_SPELLINGS = (
    "has_cooked_collision_data", "b_has_cooked_collision_data",
    "bHasCookedCollisionData", "has_cooked_physics_data",
    "b_has_cooked_physics_data", "bCooked", "cooked",
)


def main():
    import unreal
    result_path = unreal.Paths.project_saved_dir() + "MeshCollisionDiff.txt"
    try:
        subsys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        sm_cls = unreal.load_class(None, "/Script/Engine.StaticMeshActor")
        for path in (WORKING, BROKEN):
            mesh = unreal.EditorAssetLibrary.load_asset(path)
            log("========== %s ==========" % path)
            if mesh is None:
                log("LOAD FAILED")
                continue
            dump(mesh, "mesh")
            bs = mesh.get_editor_property("body_setup")
            log("body_setup = %r" % (bs,))
            if bs is not None:
                dump(bs, "body_setup")
                dump_box_elems(bs, path)
                for sp in COOKED_SPELLINGS:
                    try:
                        log("cooked-probe %s = %s" % (
                            sp, bs.get_editor_property(sp)))
                    except Exception:
                        pass

            # Spawn + read component / body_instance state in THIS process.
            actor = subsys.spawn_actor_from_class(
                sm_cls, unreal.Vector(0.0, 0.0, 90000.0),
                unreal.Rotator(0.0, 0.0, 0.0))
            comp = actor.get_component_by_class(unreal.StaticMeshComponent)
            comp.set_static_mesh(mesh)
            dump(comp, "component")
            try:
                bi = comp.get_editor_property("body_instance")
                log("body_instance = %r" % (bi,))
                dump(bi, "body_instance")
            except Exception as exc:
                log("body_instance unreadable: %s" % exc)
            subsys.destroy_actor(actor)
    except Exception:
        log("TOP-FAIL\n" + traceback.format_exc())

    with open(result_path, "w") as f:
        f.write("\n".join(lines) + "\n")
    print("RESULT: lines=%d" % len(lines))


if __name__ == "__main__":
    main()
