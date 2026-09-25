"""Decisive experiment: WHOSE BodySetup decides - the mesh's or the data?

Backlog #24 collision forensics. Fresh process, in-memory only (nothing is
saved):

  1. rack mesh  + rack BodySetup   -> MISS     (known: no body in fresh)
  2. cube mesh  + cube BodySetup   -> HIT      (known: works)
  3. rack mesh  + CUBE BodySetup   -> ?  if HIT: mesh data is fine, the
                                       rack's BodySetup payload (the
                                       non-exposed bytes) is the killer
  4. cube mesh  + RACK BodySetup   -> ?  if MISS: confirms BodySetup is
                                       decisive and it travels with the
                                       data, not with the mesh

Swap is done via set_editor_property("body_setup", ...) on the asset in
memory; the component body is recreated between sweeps. Result:
<Saved>/MeshBodySwapProbe.txt
"""
import traceback

RACK = "/Game/Props/SM_BladeRack_Klingenhof"
CUBE = "/Engine/BasicShapes/Cube.Cube"

lines = []


def log(msg):
    lines.append(msg)
    print("[SWAP] " + msg)


def sweep(subsys, sm_cls, mesh, tag):
    import unreal
    actor = None
    try:
        actor = subsys.spawn_actor_from_class(
            sm_cls, unreal.Vector(0.0, 0.0, 95000.0),
            unreal.Rotator(0.0, 0.0, 0.0))
        comp = actor.get_component_by_class(unreal.StaticMeshComponent)
        comp.set_static_mesh(mesh)
        o, e = actor.get_actor_bounds(False, False)
        start = unreal.Vector(o.x, o.y, o.z + e.z + 100.0)
        end = unreal.Vector(o.x, o.y, o.z - e.z + 40.0)
        res = []
        for name, complex_trace in (("S", False), ("C", True)):
            hit = unreal.SystemLibrary.capsule_trace_single(
                actor.get_world(), start, end, 20.0, 30.0,
                unreal.TraceTypeQuery.TRACE_TYPE_QUERY1,
                complex_trace, [], unreal.DrawDebugTrace.NONE)
            res.append("%s=%s" % (name, "HIT" if hit is not None else "MISS"))
        log("%s: %s" % (tag, "/".join(res)))
    except Exception:
        log("%s FAIL\n%s" % (tag, traceback.format_exc().replace("\n", " | ")))
    finally:
        if actor is not None:
            subsys.destroy_actor(actor)


def main():
    import unreal
    result_path = unreal.Paths.project_saved_dir() + "MeshBodySwapProbe.txt"
    try:
        subsys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        sm_cls = unreal.load_class(None, "/Script/Engine.StaticMeshActor")

        rack = unreal.EditorAssetLibrary.load_asset(RACK)
        cube = unreal.EditorAssetLibrary.load_asset(CUBE)
        rack_bs = rack.get_editor_property("body_setup")
        cube_bs = cube.get_editor_property("body_setup")
        log("rack_bs=%r" % (rack_bs,))
        log("cube_bs=%r" % (cube_bs,))

        # 1 + 2: controls
        sweep(subsys, sm_cls, rack, "rack+rack_bs")
        sweep(subsys, sm_cls, cube, "cube+cube_bs")

        # 3: rack mesh wearing the cube's BodySetup
        try:
            rack.set_editor_property("body_setup", cube_bs)
            sweep(subsys, sm_cls, rack, "rack+cube_bs")
        except Exception:
            log("swap rack<-cube FAIL: " + traceback.format_exc().replace(
                "\n", " | "))

        # 4: cube mesh wearing the rack's BodySetup
        try:
            cube.set_editor_property("body_setup", rack_bs)
            sweep(subsys, sm_cls, cube, "cube+rack_bs")
        except Exception:
            log("swap cube<-rack FAIL: " + traceback.format_exc().replace(
                "\n", " | "))

        # Restore originals in memory (process exits without saving anyway).
        rack.set_editor_property("body_setup", rack_bs)
        cube.set_editor_property("body_setup", cube_bs)
        log("restored (nothing was saved)")
    except Exception:
        log("TOP-FAIL\n" + traceback.format_exc())

    with open(result_path, "w") as f:
        f.write("\n".join(lines) + "\n")
    print("RESULT: lines=%d" % len(lines))


if __name__ == "__main__":
    main()
