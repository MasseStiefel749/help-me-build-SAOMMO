"""Add a UE-compatible UBX_ box collision object to a source FBX.

Run (headless, one process for all jobs):

  blender.exe --background --python add_ubx_collision.py -- <src> <dst> [<src> <dst> ...]

Backlog #24 (CC0-Prop-Pass), evidence from 2026-09-25 (ArenaCheckResult
check [7]): the FBX import only ever produced an auto-generated convex hull
(DISK-AGG box=0 convex=1) and that hull never yields a queryable body in a
non-ticking process - get_closest_point_on_collision returned -1, both the
world capsule sweep and the component-level line trace missed, while a
freshly spawned /Engine/BasicShapes/Cube (a Box primitive) hit in the very
same process. Editor-side fixes are all dead ends in a commandlet:
StaticMeshEditorSubsystem is not registered (subsys SM=False ->
add_simple_collisions returns -1), FKAggregateGeom/FKBoxElem are not
exposed to python.

So the box must come from the DCC: the UE FBX importer turns a mesh named
UBX_<RenderMeshName>_00 into an FKBoxElem in the static mesh's AggGeom
(official FBX Static Mesh Pipeline naming rules), and Box primitives build
their physics body synchronously in any process.

Script per source FBX:
  1. import, log object names + dimensions (scale round-trip evidence)
  2. drop any collision-prefixed objects that came with the file
  3. join multiple render objects (UE combines meshes anyway)
  4. add UBX_<RenderMeshName>_00: a plain rectangular prism fitted to the
     render mesh's local bounding box (+0.5 cm), sharing the render
     object's transform exactly
  5. export render + collision to <dst> (Blender FBX defaults, UE axes)
"""
import sys

import bpy
from mathutils import Vector


def log(msg):
    print("[UBX] " + msg)


def add_box_for(render, taken_name):
    bb = [Vector(c) for c in render.bound_box]
    mn = Vector((min(v[i] for v in bb) for i in range(3)))
    mx = Vector((max(v[i] for v in bb) for i in range(3)))
    # 0.5 cm inflation, converted from metres to scene units (1 BU = 1 m in
    # factory settings; a raw +0.5 would balloon a 30 cm rack to 80 cm).
    scale_len = bpy.context.scene.unit_settings.scale_length or 1.0
    inflate = 0.005 / scale_len
    size = [mx[i] - mn[i] + inflate for i in range(3)]
    center = [(mx[i] + mn[i]) / 2.0 for i in range(3)]

    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.0, 0.0, 0.0))
    box = bpy.context.active_object
    box.name = "UBX_%s_00" % taken_name
    box.data.name = box.name
    # Identical transform to the render mesh; fit happens in local space so
    # the result is a plain rectangular prism (the only shape UBX accepts).
    box.matrix_world = render.matrix_world.copy()
    for v in box.data.vertices:
        v.co = Vector((center[0] + v.co.x * size[0],
                       center[1] + v.co.y * size[1],
                       center[2] + v.co.z * size[2]))
    box.data.update()
    log("box %s size=(%.2f, %.2f, %.2f) local" % (box.name, size[0], size[1], size[2]))
    # UE 5.8 imports FBX through Interchange, which only treats a
    # UBX_/UCX_ node as collision when it is nested under the render mesh
    # (sibling nodes show up as separate geometry instead - UE forum
    # reports for 5.5+). Keep the world transform unchanged.
    box.parent = render
    box.matrix_parent_inverse = render.matrix_world.inverted()
    return box


def process(src, dst):
    log("src=%s" % src)
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=src)

    objs = [o for o in bpy.context.scene.objects if o.type == "MESH"]
    log("objects=%s" % [(o.name, tuple(round(d, 3) for d in o.dimensions))
                        for o in objs])
    if not objs:
        raise RuntimeError("no mesh in " + src)

    # Strip collision objects the source may already carry; we re-add ours
    # so the naming/transform is always exactly right.
    kept = []
    for o in objs:
        if o.name[:4] in ("UCX_", "UBX_", "UCP_", "USP_"):
            log("dropping source collision object %s" % o.name)
            bpy.data.objects.remove(o, do_unlink=True)
        else:
            kept.append(o)
    if not kept:
        raise RuntimeError("no render mesh left in " + src)

    if len(kept) > 1:
        bpy.ops.object.select_all(action="DESELECT")
        for o in kept:
            o.select_set(True)
        bpy.context.view_layer.objects.active = kept[0]
        bpy.ops.object.join()
        log("joined %d render objects" % len(kept))
    render = bpy.context.view_layer.objects.active
    if render is None or render.type != "MESH":
        render = kept[0]
    # Interchange matches UBX_ nodes against the mesh/geometry datablock
    # name (UE logged 'katana_stand_01_blockout_001' / 'Mesh' / 'bench_001'
    # for the originals, none of which equal the object name) - align data
    # name with the object name so both match conventions hold.
    if render.data.name != render.name:
        log("renaming mesh data %r -> %r" % (render.data.name, render.name))
        render.data.name = render.name
    log("render=%s dims=(%.3f, %.3f, %.3f)" % (
        render.name, render.dimensions[0], render.dimensions[1],
        render.dimensions[2]))

    box = add_box_for(render, render.name)

    bpy.ops.object.select_all(action="DESELECT")
    render.select_set(True)
    box.select_set(True)
    bpy.context.view_layer.objects.active = render
    bpy.ops.export_scene.fbx(
        filepath=dst,
        use_selection=True,
        object_types={"MESH"},
        use_mesh_modifiers=True,
        axis_forward="-Z",
        axis_up="Y",
        apply_unit_scale=True,
    )
    log("dst=%s" % dst)


def main():
    args = sys.argv[sys.argv.index("--") + 1:]
    if len(args) < 2 or len(args) % 2 != 0:
        raise RuntimeError("expected <src> <dst> pairs, got: %r" % (args,))
    for i in range(0, len(args), 2):
        process(args[i], args[i + 1])
    log("done jobs=%d" % (len(args) // 2))


main()
