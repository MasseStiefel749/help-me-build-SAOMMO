import bpy, math, os
from mathutils import Vector

ROOT=os.path.dirname(os.path.abspath(__file__))
KIT=os.path.join(ROOT,"SAOMMO_World_Builder_Kit.blend")
PROPS=os.path.join(ROOT,"SAOMMO_World_Props.blend")
OUT=os.path.join(ROOT,"SAOMMO_Starting_Reach.blend")
PREVIEW=os.path.join(ROOT,"renders","SAOMMO_Starting_Reach.png")
os.makedirs(os.path.dirname(PREVIEW),exist_ok=True)

bpy.ops.object.select_all(action='SELECT'); bpy.ops.object.delete(use_global=False)
for db in (bpy.data.meshes,bpy.data.curves,bpy.data.materials):
    pass
scene=bpy.context.scene
scene.render.engine='CYCLES'; scene.cycles.samples=24
scene.render.resolution_x=2400; scene.render.resolution_y=1700; scene.render.resolution_percentage=100
scene.render.image_settings.file_format='PNG'; scene.render.film_transparent=False
scene.view_settings.view_transform='AgX'
scene.view_settings.look='AgX - Medium High Contrast'
scene.view_settings.exposure=.45

def mat(n,c,metal=0,rough=.82,em=0):
    m=bpy.data.materials.new(n); m.diffuse_color=(*c,1); m.use_nodes=True
    nt=m.node_tree; p=nt.nodes.get('Principled BSDF'); p.inputs['Base Color'].default_value=(*c,1); p.inputs['Metallic'].default_value=metal; p.inputs['Roughness'].default_value=rough
    tc=nt.nodes.new('ShaderNodeTexCoord'); noise=nt.nodes.new('ShaderNodeTexNoise'); noise.inputs['Scale'].default_value=28; noise.inputs['Detail'].default_value=3.3; noise.inputs['Roughness'].default_value=.72; nt.links.new(tc.outputs['Generated'],noise.inputs['Vector'])
    ramp=nt.nodes.new('ShaderNodeValToRGB'); ramp.color_ramp.elements[0].position=.23; ramp.color_ramp.elements[0].color=(*(v*.76 for v in c),1); ramp.color_ramp.elements[1].position=.82; ramp.color_ramp.elements[1].color=(*(min(v*1.17,1) for v in c),1); nt.links.new(noise.outputs['Fac'],ramp.inputs['Fac']); nt.links.new(ramp.outputs['Color'],p.inputs['Base Color'])
    bump=nt.nodes.new('ShaderNodeBump'); bump.inputs['Strength'].default_value=.1; bump.inputs['Distance'].default_value=.035; nt.links.new(noise.outputs['Fac'],bump.inputs['Height']); nt.links.new(bump.outputs['Normal'],p.inputs['Normal'])
    if em: p.inputs['Emission Color'].default_value=(*c,1); p.inputs['Emission Strength'].default_value=em
    return m
GRASS=mat('Terrain • meadow',(.23,.40,.17)); MEADOW=mat('Terrain • Brunnfeld',(.39,.46,.23)); FOREST=mat('Terrain • forest floor',(.11,.27,.15)); DIRT=mat('Packed earth',(.35,.24,.15)); STONE=mat('Weathered limestone',(.49,.47,.39)); PALE=mat('Worn stone edges',(.68,.62,.49)); DARK=mat('Deep stone',(.18,.24,.27)); WOOD=mat('Aged oak',(.32,.19,.10)); WOODL=mat('Fresh repair timber',(.55,.36,.19)); ROOF=mat('Roof slate',(.22,.30,.36)); LEAF=mat('Pine needles',(.07,.27,.16)); LEAF2=mat('Forest canopy',(.18,.40,.18)); GOLD=mat('Warm brass',(.72,.51,.22),.62,.29); GLOW=mat('Sealed relic glow',(.95,.47,.12),.15,.35,3); WATER=mat('Well water',(.08,.41,.53),.38,.22)
def cube(n,loc,dim,m,bev=0):
    bpy.ops.mesh.primitive_cube_add(size=1,location=loc); o=bpy.context.object; o.name=n; o.dimensions=dim; bpy.ops.object.transform_apply(location=False,rotation=False,scale=True); o.data.materials.append(m)
    if bev:
        mod=o.modifiers.new('Soft worn edges','BEVEL'); mod.width=bev; mod.segments=2
        mod=o.modifiers.new('Corner normals','WEIGHTED_NORMAL')
    return o
def cyl(n,loc,r,d,m,v=12,r2=None):
    if r2 is None: bpy.ops.mesh.primitive_cylinder_add(vertices=v,radius=r,depth=d,location=loc)
    else: bpy.ops.mesh.primitive_cone_add(vertices=v,radius1=r,radius2=r2,depth=d,location=loc)
    o=bpy.context.object; o.name=n; o.data.materials.append(m); return o
def ico(n,loc,sc,m,sub=1):
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=sub,radius=1,location=loc); o=bpy.context.object; o.name=n; o.scale=sc; bpy.ops.object.transform_apply(location=False,rotation=False,scale=True); o.data.materials.append(m); return o
def beam(n,a,b,w,m):
    a,b=Vector(a),Vector(b); d=b-a; o=cube(n,(a+b)/2,(w,w,d.length),m,.015); o.rotation_euler=d.to_track_quat('Z','Y').to_euler(); return o
def text(n,s,loc,size,m):
    c=bpy.data.curves.new(n,'FONT'); c.body=s; c.size=size; c.extrude=.006; o=bpy.data.objects.new(n,c); bpy.context.collection.objects.link(o); o.location=loc; o.data.materials.append(m); return o

# load modular meshes; instances share mesh data for efficient iteration
templates={}
for filepath in (KIT,PROPS):
    with bpy.data.libraries.load(filepath,link=False) as (src,dst): dst.objects=[n for n in src.objects if n.startswith('SM_')]
    for ob in dst.objects:
        if ob: templates[ob.name]=ob
def inst(asset,loc,scale=1,rot=0,name=None):
    src=templates.get(asset)
    if not src: return None
    o=bpy.data.objects.new(name or asset,src.data); bpy.context.collection.objects.link(o); o.location=loc; o.rotation_euler[2]=rot; o.scale=(scale,)*3; return o

# A single connected landscape footprint (single-level slice, no invented continents)
cube('Continuous playable footprint',(40,78,-1.35),(190,270,2.7),GRASS)
cube('Brunnfeld • 50 × 50 m',(0,0,.03),(50,50,.18),MEADOW)
cube('Klingenhof • 30 × 30 m',(60,0,.06),(30,30,.22),DIRT)
cube('Grauwaldrand • 80 × 80 m',(60,80,.02),(80,80,.20),FOREST)
cube('Ruin approach',(60,139,.02),(34,38,.20),DIRT)
cube('Ruin interior • 40 × 60 m',(60,177,.05),(40,60,.30),STONE)
# connected 8m main road and onward path
cube('Road • Brunnfeld to Klingenhof',(30,0,.18),(35,8,.20),DIRT)
cube('Road • north gate',(60,34,.18),(8,35,.20),DIRT)
cube('Forest trail',(60,78,.17),(5,53,.16),DIRT)
cube('Ruin approach path',(60,125,.17),(6,44,.16),DIRT)

# Brunnfeld: rebuildable palisade, open north/east gates, fountain and scaffolded homes
for x in [-24,-16,-8,0,8,16,24]:
    if abs(x)>4:
        cube('Brunnfeld low stone wall',(x,-24,1.05),(7.7,.65,2.0),STONE,.06)
for y in [-16,-8,0,8,16,24]:
    cube('Brunnfeld west wall',(-24,y,1.05),(.65,7.7,2),STONE,.06)
    if y<16: cube('Brunnfeld east wall',(24,y,1.05),(.65,7.7,2),STONE,.06)
for x in [-20,-12,-4,4,12,20]:
    cube('Reconstruction scaffold post',(x,-20,2.6),(.24,.24,5.2),WOODL)
    cube('Reconstruction scaffold rail',(x,-20,4.2),(2.2,.22,.22),WOODL)
cube('Scaffold plank',(-12,-19.75,2.2),(16,.65,.16),WOOD)
# modest timber/stone house shells, kept low to preserve clear sightlines
for x,y in [(-15,13),(15,13),(-15,-11)]:
    for side in [-1,1]: cube('Brunnfeld house wall',(x+side*3,y,2),(0.35,7,4),STONE)
    for side in [-1,1]: cube('Brunnfeld house wall',(x,y+side*3,2),(6,.35,4),STONE)
    cube('House roof wing',(x,y,4.7),(7.5,4,0.25),ROOF).rotation_euler[1]=-.35
    cube('House roof wing',(x,y,4.7),(7.5,4,0.25),ROOF).rotation_euler[1]=.35
# well centerpiece and settlement props
cyl('Ancient well stone rim',(0,0,1),2.5,1.8,STONE,16); cyl('Well inner water',(0,0,1.95),1.65,.08,WATER,16)
for x in [-1.8,1.8]: cube('Well timber support',(x,0,3.0),(.18,.2,3),WOODL)
beam('Well crossbar',(-1.8,0,4),(1.8,0,4),.18,WOODL)
for x,y in [(-5,-4),(5,-4),(-6,5),(6,5)]: inst('SM_KlingenhofBench_01',(x,y,.2),.8)
for x,y in [(-9,2),(9,-2),(10,10),(-9,-11)]: inst('SM_OldSupplyCrate_01',(x,y,.2),.8)
inst('SM_LanternPost_01',(-8,18,.2),.9); inst('SM_Signpost_01',(8,18,.2),.9)
inst('SM_Waystone_Checkpoint_01',(0,-17,.2),.75)

# Klingenhof: clearly marked training square with four readable practice targets
for x in [43,77]:
    cube('Training yard boundary',(x,0,.7),(.5,29,1.4),STONE)
for y in [-14,14]: cube('Training yard boundary',(60,y,.7),(34,.5,1.4),STONE)
for x,y in [(50,-7),(70,-7),(50,7),(70,7)]:
    cyl('Training dummy base',(x,y,.6),.75,.45,WOOD,10)
    cube('Training dummy body',(x,y,1.9),(.9,.55,2.25),WOODL,.08)
    ico('Training dummy head',(x,y,3.25),(.55,.48,.55),WOOD,1)
    beam('Dummy crossbar',(x-1,y,2.35),(x+1,y,2.35),.20,WOOD)
inst('SM_SwordMonument_01',(60,0,.2),.65)
for x,y in [(45,11),(75,11)]: inst('SM_LanternPost_01',(x,y,.2),.8)
text('Klingenhof sign','KLINGENHOF',(48,-19,.35),2.4,PALE)

# Gate arch at the documented threshold. Trees frame rather than block the route.
inst('SM_RuinArch_Broken_01',(60,34,.25),1.5)
for x,y,s in [(25,48,1.3),(92,45,1.4),(24,77,1.2),(96,81,1.35),(29,108,1.25),(91,111,1.2),(40,119,1),(80,119,1),(31,63,1.15),(89,95,1.2),(42,45,1.05),(82,54,1.15),(32,91,1.3),(87,69,1.15),(49,58,1),(72,61,1.1),(51,91,1.1),(70,111,1),(36,72,1.1),(84,78,1.1)]:
    inst('SM_Tree_Pine_Tall',(x,y,0),s,rot=(x+y)%6)
for x,y,s in [(38,57,.8),(83,63,1),(43,101,.8),(76,107,.8),(35,112,.75),(88,54,.8)]: inst('SM_Tree_Deciduous_01',(x,y,0),s)
for x,y,s in [(43,50,1.4),(78,52,1.2),(39,85,1.1),(80,91,1.5),(46,111,1.1),(74,72,1.2),(94,101,.9)]: inst('SM_RockCluster_Medium',(x,y,.2),s)
for x,y,ang in [(42,71,.2),(79,87,-.2),(43,103,.4)]:
    inst('SM_Ruin_RubbleCluster_01',(x,y,.25),1,ang)
    inst('SM_RuinPillar_Broken_01',(x+2,y,.25),.8,ang)
for x,y in [(53,67),(70,96),(52,108)]: inst('SM_Grass_Clump_01',(x,y,.2),1.5)

# Collapsed dome threshold / linear three-room ruin; light marks a sealed teaser only.
inst('SM_RuinArch_Broken_01',(60,125,.25),1.7)
for x in [39,81]: cube('Ruin outer wall',(x,177,3),(2,60,6),STONE,.08)
for y in [148,206]: cube('Ruin end wall',(60,y,3),(42,2,6),STONE,.08)
# hall dividers with wide central door gaps create three visible rooms
for y in [166,187]:
    cube('Hall partition west',(49,y,2.3),(20,1.5,4.6),STONE,.05)
    cube('Hall partition east',(71,y,2.3),(20,1.5,4.6),STONE,.05)
for x,y in [(40,153),(80,155),(40,190),(80,195),(44,202),(76,151)]:
    inst('SM_Pillar_Stone_3m',(x,y,.2),1.3)
for x,y,a in [(46,158,.2),(75,173,-.5),(47,195,.4),(75,202,-.2)]:
    inst('SM_Ruin_RubbleCluster_01',(x,y,.2),1.2,a)
# fractured roof remnants around perimeter, central opening remains clear
for x,y,a in [(45,151,0),(75,151,0),(45,203,0),(75,203,0),(40,177,math.pi/2),(80,177,math.pi/2)]:
    cube('Collapsed dome remnant',(x,y,8),(14,4,.65),ROOF).rotation_euler[2]=a
inst('SM_SwordMonument_01',(60,197,.35),1.3)
cyl('Sealed niche plinth',(60,197,1),2.2,1.1,STONE,12)
ico('Artifact glimpse • sealed',(60,197,3.7),(.38,.38,2.0),GLOW,2)
for x in [57,63]: cyl('Niche flanking pillar',(x,197,2.0),.35,4,PALE,8)
text('Ruin label','RUINE DES ERSTEN GRIFFS',(42,211,.35),2.1,PALE)

# Zone labels above ground; no lore inventions and no collectible reward.
text('Town label','BRUNNFELD',( -19,27,.25),3,PALE)
text('Forest label','GRAUWALDRAND',(39,116,.3),3,PALE)

# Visual locator marks and soft warm lighting
for x,y in [(0,0),(60,0),(60,80),(60,177)]:
    cyl('Zone marker',(x,y,.20),.28,.12,GOLD,12)
world=bpy.data.worlds.new('Starting Reach • clear late afternoon'); scene.world=world; world.use_nodes=True
world.node_tree.nodes['Background'].inputs['Color'].default_value=(.44,.56,.73,1); world.node_tree.nodes['Background'].inputs['Strength'].default_value=.42
bpy.ops.object.light_add(type='SUN',location=(30,-50,180)); sun=bpy.context.object; sun.name='Golden hour sun'; sun.data.energy=1.65; sun.rotation_euler=(math.radians(28),math.radians(-18),math.radians(-25))
bpy.ops.object.light_add(type='AREA',location=(10,-35,180)); key=bpy.context.object; key.name='Soft golden daylight'; key.data.energy=16500; key.data.shape='DISK'; key.data.size=140; key.rotation_euler=(math.radians(22),0,math.radians(18))
bpy.ops.object.light_add(type='AREA',location=(60,193,12)); accent=bpy.context.object; accent.name='Niche warm beacon'; accent.data.energy=380; accent.data.color=(1,.36,.09); accent.data.shape='DISK'; accent.data.size=7; accent.rotation_euler=(0,0,0)
# Camera faces diagonally across the entire long slice, with labels and all four areas in frame.
bpy.ops.object.camera_add(location=(255,-245,310)); cam=bpy.context.object; cam.name='CAM_StartingReach_Overview'; target=Vector((40,83,0)); cam.rotation_euler=(target-cam.location).to_track_quat('-Z','Y').to_euler(); cam.data.type='ORTHO'; cam.data.ortho_scale=310; scene.camera=cam
scene.render.filepath=PREVIEW
scene.render.image_settings.color_mode='RGBA'
scene.render.resolution_percentage=100
scene.render.image_settings.file_format='PNG'
scene.render.film_transparent=False
# Render with Eevee for a fast, clean overview; the saved file remains easy to edit.
scene.render.engine='CYCLES'; scene.cycles.samples=16
bpy.ops.wm.save_as_mainfile(filepath=OUT)
bpy.ops.render.render(write_still=True)
# One scene FBX keeps every mesh as a separately named static mesh, in Unreal centimeters.
FBX_DIR=os.path.join(ROOT,'Exports','Starting_Reach'); os.makedirs(FBX_DIR,exist_ok=True)
bpy.ops.object.select_all(action='DESELECT')
mesh_objects=[o for o in scene.objects if o.type=='MESH']
for o in mesh_objects:
    o.select_set(True)
    # Unique export datablocks prevent FBX material-index ambiguity on linked kit instances.
    o.data=o.data.copy()
if mesh_objects:
    bpy.context.view_layer.objects.active=mesh_objects[0]
    bpy.ops.export_scene.fbx(filepath=os.path.join(FBX_DIR,'SM_StartingReach_Blockout.fbx'),use_selection=True,
        object_types={'MESH'},apply_unit_scale=True,apply_scale_options='FBX_SCALE_ALL',axis_forward='-Z',axis_up='Y',
        use_mesh_modifiers=True,add_leaf_bones=False,bake_anim=False,mesh_smooth_type='FACE',path_mode='AUTO')
print('Saved',OUT,'and',PREVIEW,'mesh objects:',len(mesh_objects))
