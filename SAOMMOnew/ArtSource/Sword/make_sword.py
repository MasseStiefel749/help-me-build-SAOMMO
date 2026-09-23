"""
SAOMMO — procedural one-handed sword (Band 5 §14 naming, Band 5 §18 licensing).

Generates SM_SAOSword.fbx:
  - Origin at grip center (hand position, socket attach point)
  - Blade tip along local +Z (matches ASword aim code: blade UpVector = tip dir)
  - Real-world size in meters (1 BU = 1 m -> UE imports as x100 = cm)
  - Total length ~1.05 m (pommel to tip), one-handed arming sword proportions
  - Single UV set via smart project (ready for texturing later)

License: authored for SAOMMO, released CC0. Source of truth for the asset.

Usage (headless):
  blender.exe --background --python make_sword.py
"""

import bpy
import bmesh
import os

# ---------------------------------------------------------------- parameters
BLADE_W = 0.045        # blade width (m)
BLADE_T = 0.009        # blade thickness (m)
BLADE_BASE_Z = 0.10    # blade starts above the guard
BLADE_SHOULDER_Z = 0.88  # taper starts here
BLADE_TIP_Z = 0.955    # point tip

GUARD_W = 0.17         # crossguard span
GUARD_D = 0.032
GUARD_H = 0.04
GUARD_Z = 0.11

GRIP_R = 0.0135
GRIP_BOTTOM = -0.06
GRIP_TOP = 0.09

POMMEL_R = 0.022
POMMEL_Z = -0.072

OUT_FBX = os.path.join(
    r"C:\Users\simon\OneDrive\Documents\GitHub\help-me-build-SAOMMO\SAOMMOnew\ArtSource\Sword",
    "SM_SAOSword.fbx")

# ---------------------------------------------------------------- fresh scene
bpy.ops.wm.read_factory_settings(use_empty=True)
scene = bpy.context.scene
scene.unit_settings.system = 'METRIC'
scene.unit_settings.scale_length = 1.0


def make_obj(name, bm):
    me = bpy.data.meshes.new(name)
    bm.to_mesh(me)
    bm.free()
    ob = bpy.data.objects.new(name, me)
    bpy.context.collection.objects.link(ob)
    return ob


# ---------------------------------------------------------------- blade
def build_blade():
    bm = bmesh.new()
    # Diamond-ish cross-section (6 points): sharp edges left/right, flat center.
    w, t = BLADE_W, BLADE_T
    profile = [
        ( w / 2, 0.0),
        ( w * 0.18,  t / 2),
        (-w * 0.18,  t / 2),
        (-w / 2, 0.0),
        (-w * 0.18, -t / 2),
        ( w * 0.18, -t / 2),
    ]
    verts = [bm.verts.new((x, y, BLADE_BASE_Z)) for x, y in profile]
    face = bm.faces.new(verts)

    # Main blade body up to the shoulder (slight taper, blade is axis-centered
    # so scaling x/y around the origin is safe).
    res = bmesh.ops.extrude_face_region(bm, geom=[face])
    top = [g for g in res["geom"] if isinstance(g, bmesh.types.BMVert)]
    bmesh.ops.translate(bm, verts=top, vec=(0, 0, BLADE_SHOULDER_Z - BLADE_BASE_Z))
    for v in top:
        v.co.x *= 0.72
        v.co.y *= 0.85

    # Taper section toward the tip.
    face2 = max(bm.faces, key=lambda f: f.calc_center_median().z)
    res2 = bmesh.ops.extrude_face_region(bm, geom=[face2])
    top2 = [g for g in res2["geom"] if isinstance(g, bmesh.types.BMVert)]
    tip_z = BLADE_TIP_Z - 0.035
    bmesh.ops.translate(bm, verts=top2, vec=(0, 0, tip_z - BLADE_SHOULDER_Z))
    for v in top2:
        v.co.x *= 0.45
        v.co.y *= 0.70

    # Final point: poke the top face and pull its center vertex to the tip.
    face3 = max(bm.faces, key=lambda f: f.calc_center_median().z)
    poke = bmesh.ops.poke(bm, faces=[face3])
    center = poke["verts"][0]
    center.co = (0.0, 0.0, BLADE_TIP_Z)

    bm.normal_update()
    return make_obj("SAOSword_Blade", bm)


# ---------------------------------------------------------------- guard
def build_guard():
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    bmesh.ops.scale(bm, verts=bm.verts[:],
                    vec=(GUARD_W, GUARD_D, GUARD_H))
    bmesh.ops.translate(bm, verts=bm.verts[:], vec=(0, 0, GUARD_Z))
    return make_obj("SAOSword_Guard", bm)


# ---------------------------------------------------------------- grip
def build_grip():
    bm = bmesh.new()
    depth = GRIP_TOP - GRIP_BOTTOM
    center_z = (GRIP_TOP + GRIP_BOTTOM) / 2.0
    bmesh.ops.create_cone(bm, cap_ends=True, cap_tris=False, segments=16,
                          radius1=GRIP_R, radius2=GRIP_R * 0.92, depth=depth)
    bmesh.ops.translate(bm, verts=bm.verts[:], vec=(0, 0, center_z))
    return make_obj("SAOSword_Grip", bm)


# ---------------------------------------------------------------- pommel
def build_pommel():
    bm = bmesh.new()
    bmesh.ops.create_uvsphere(bm, u_segments=16, v_segments=12,
                              radius=POMMEL_R)
    bmesh.ops.scale(bm, verts=bm.verts[:], vec=(1.0, 0.8, 1.15))
    bmesh.ops.translate(bm, verts=bm.verts[:], vec=(0, 0, POMMEL_Z))
    return make_obj("SAOSword_Pommel", bm)


blade = build_blade()
guard = build_guard()
grip = build_grip()
pommel = build_pommel()

# ---------------------------------------------------------------- join
bpy.ops.object.select_all(action='DESELECT')
for ob in (blade, guard, grip, pommel):
    ob.select_set(True)
bpy.context.view_layer.objects.active = blade
bpy.ops.object.join()
sword = bpy.context.view_layer.objects.active
sword.name = "SM_SAOSword"
sword.data.name = "SM_SAOSword"

# Origin at grip center = world origin (hand attach point).
bpy.context.scene.cursor.location = (0.0, 0.0, 0.0)
bpy.ops.object.origin_set(type='ORIGIN_CURSOR')

# Gentle bevel so the low-poly edges catch light.
bev = sword.modifiers.new("EdgeBevel", 'BEVEL')
bev.width = 0.0016
bev.segments = 2
bev.angle_limit = 0.7
bpy.ops.object.modifier_apply(modifier=bev.name)

# UVs: one smart-projected set (texturing comes later, Band 5 pipeline).
bpy.ops.object.mode_set(mode='EDIT')
bpy.ops.mesh.select_all(action='SELECT')
bpy.ops.uv.smart_project(angle_limit=1.15, island_margin=0.02)
bpy.ops.object.mode_set(mode='OBJECT')

# ---------------------------------------------------------------- verify + export
os.makedirs(os.path.dirname(OUT_FBX), exist_ok=True)
bpy.ops.object.select_all(action='DESELECT')
sword.select_set(True)
bpy.context.view_layer.objects.active = sword

bb = [sword.matrix_world @ __import__("mathutils").Vector(c) for c in sword.bound_box]
dims = (max(v.x for v in bb) - min(v.x for v in bb),
        max(v.y for v in bb) - min(v.y for v in bb),
        max(v.z for v in bb) - min(v.z for v in bb))
print(f"[SAOMMO] verts={len(sword.data.vertices)} dims_m=({dims[0]:.3f}, {dims[1]:.3f}, {dims[2]:.3f}) "
      f"-> UE cm=({dims[0]*100:.1f}, {dims[1]*100:.1f}, {dims[2]*100:.1f})")

bpy.ops.export_scene.fbx(
    filepath=OUT_FBX,
    use_selection=True,
    object_types={'MESH'},
    use_mesh_modifiers=True,
    bake_space_transform=True,
    axis_forward='-Z',
    axis_up='Y',
)
print(f"[SAOMMO] exported: {OUT_FBX} ({os.path.getsize(OUT_FBX)} bytes)")
