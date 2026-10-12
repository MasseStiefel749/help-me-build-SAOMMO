import bpy
import os
import math

OUT = r"C:\Users\simon\OneDrive\Documents\GitHub\help-me-build-SAOMMO\SAOMMOnew\ArtSource\Blender\Exports\Armor"
RENDER = r"C:\Users\simon\OneDrive\Documents\GitHub\help-me-build-SAOMMO\SAOMMOnew\ArtSource\Blender\renders\SAOMMO_armor_kit.png"
os.makedirs(OUT, exist_ok=True)

# clean scene
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
for coll in (bpy.data.meshes, bpy.data.materials):
    for x in list(coll):
        coll.remove(x)

IRON = (0.32, 0.34, 0.38, 1.0)
TRIM = (0.72, 0.45, 0.16, 1.0)

def make_mat(name, color):
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    bsdf = m.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = color
    bsdf.inputs["Metallic"].default_value = 0.85
    bsdf.inputs["Roughness"].default_value = 0.45
    return m

iron = make_mat("SAOMMO_Iron", IRON)
trim = make_mat("SAOMMO_Trim", TRIM)

def finish(obj, name):
    obj.name = name
    obj.data.materials.append(iron)
    obj.data.materials.append(trim)
    # trim slot on second material index where possible
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    return obj

def pauldron(side):
    # half-dome shell centered on origin (sits on the shoulder joint)
    bpy.ops.mesh.primitive_uv_sphere_add(radius=0.11, location=(0, 0, 0))
    o = bpy.context.active_object
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    # keep upper hemisphere
    bpy.ops.mesh.bisect(plane_co=(0, 0, 0), plane_no=(0, 0, -1), clear_inner=True)
    bpy.ops.object.mode_set(mode='OBJECT')
    # flatten slightly + rim band via scale
    o.scale = (1.0, 1.15, 0.8)
    bpy.ops.object.transform_apply(scale=True)
    # trim rim: second material on lower ring is skipped (single mesh) - keep slots
    return finish(o, "SM_ShoulderPad_%s" % side)

def chest():
    # chest plate: rounded box hanging below origin (spine_03 joint)
    bpy.ops.mesh.primitive_cube_add(size=0.30, location=(0, 0, -0.16))
    o = bpy.context.active_object
    o.scale = (1.15, 0.55, 1.25)
    bpy.ops.object.transform_apply(scale=True)
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.bevel(offset=0.03, segments=2)
    bpy.ops.object.mode_set(mode='OBJECT')
    return finish(o, "SM_ChestPlate")

def bracer(side):
    # forearm guard: open cylinder segment centered on origin
    bpy.ops.mesh.primitive_cylinder_add(radius=0.055, depth=0.22, location=(0, 0, 0))
    o = bpy.context.active_object
    return finish(o, "SM_Bracer_%s" % side)

parts = [pauldron("L"), pauldron("R"), chest(), bracer("L"), bracer("R")]
# mirror right-side parts across X
for o in [p for p in parts if p.name.endswith("_R")]:
    o.scale.x *= -1

# export each
for o in parts:
    bpy.ops.object.select_all(action='DESELECT')
    o.select_set(True)
    bpy.context.view_layer.objects.active = o
    bpy.ops.export_scene.fbx(
        filepath=os.path.join(OUT, o.name + ".fbx"),
        use_selection=True, bake_space_transform=True,
        object_types={'MESH'}, use_mesh_modifiers=True)

# gallery render
scene = bpy.context.scene
scene.render.resolution_x = 1280
scene.render.resolution_y = 720
scene.render.film_transparent = False
world = bpy.data.worlds["World"]
world.use_nodes = True
bg = world.node_tree.nodes["Background"]
bg.inputs[0].default_value = (0.10, 0.16, 0.26, 1.0)
bg.inputs[1].default_value = 1.0
sun = bpy.data.objects.new("Sun", None)
bpy.context.collection.objects.link(sun)
# arrange in a row for preview
for i, o in enumerate(parts):
    o.location.x += (i - 2) * 0.45
scene.camera = None
for o in bpy.data.objects:
    if o.type == 'CAMERA':
        scene.camera = o
        break
if scene.camera is None:
    bpy.ops.object.camera_add(location=(0, -2.2, 0.6), rotation=(math.radians(75), 0, 0))
    scene.camera = bpy.context.active_object
scene.render.filepath = RENDER
bpy.ops.render.render(write_still=True)
print("SAOMMO armor done: %d parts" % len(parts))
