import bpy, math, os
from mathutils import Vector

ROOT=os.path.dirname(os.path.abspath(__file__))
EXPORT=os.path.join(ROOT,"Exports","FBX")
os.makedirs(EXPORT,exist_ok=True)

def mat(name,color,metal=0.0,rough=.78,emission=0.0):
    m=bpy.data.materials.new(name); m.diffuse_color=(*color,1); m.use_nodes=True
    bs=m.node_tree.nodes.get("Principled BSDF"); bs.inputs["Base Color"].default_value=(*color,1)
    bs.inputs["Metallic"].default_value=metal; bs.inputs["Roughness"].default_value=rough
    nt=m.node_tree; tc=nt.nodes.new("ShaderNodeTexCoord"); noise=nt.nodes.new("ShaderNodeTexNoise"); noise.inputs["Scale"].default_value=30; noise.inputs["Detail"].default_value=3.4; noise.inputs["Roughness"].default_value=.72; nt.links.new(tc.outputs["Generated"],noise.inputs["Vector"])
    ramp=nt.nodes.new("ShaderNodeValToRGB"); ramp.color_ramp.elements[0].position=.22; ramp.color_ramp.elements[0].color=(*(v*.76 for v in color),1); ramp.color_ramp.elements[1].position=.82; ramp.color_ramp.elements[1].color=(*(min(v*1.16,1) for v in color),1); nt.links.new(noise.outputs["Fac"],ramp.inputs["Fac"]); nt.links.new(ramp.outputs["Color"],bs.inputs["Base Color"])
    bump=nt.nodes.new("ShaderNodeBump"); bump.inputs["Strength"].default_value=.11; bump.inputs["Distance"].default_value=.04; nt.links.new(noise.outputs["Fac"],bump.inputs["Height"]); nt.links.new(bump.outputs["Normal"],bs.inputs["Normal"])
    if emission:
        bs.inputs["Emission Color"].default_value=(*color,1); bs.inputs["Emission Strength"].default_value=emission
    return m

LIMESTONE=mat("M_WeatheredLimestone",(.46,.45,.38),0,.84)
STONE_LIGHT=mat("M_WornStoneEdges",(.66,.60,.48),0,.76)
STONE_DARK=mat("M_DeepStone",(.15,.20,.23),0,.88)
WOOD=mat("M_OldOak",(.31,.18,.095),0,.78)
WOOD_LIGHT=mat("M_OakEdge",(.54,.34,.17),0,.70)
IRON=mat("M_BlackIron",(.17,.23,.27),.58,.36)
STEEL=mat("M_WeatheredBlueSteel",(.48,.61,.70),.48,.29)
GOLD=mat("M_AgedGold",(.70,.49,.19),.72,.28)
LABEL=mat("M_GalleryLabel",(.96,.68,.30),0,.5,1.4)
TEAL=mat("M_RuneTeal",(.04,.63,.91),.4,.22,1.8)
FLAME=mat("M_FireAmber",(1.0,.25,.035),.04,.3,3.0)
FLAME_CORE=mat("M_FireGold",(1.0,.72,.12),.0,.25,4.0)

def cube(name,loc,dim,material,bevel=.04):
    bpy.ops.mesh.primitive_cube_add(size=1,location=loc); o=bpy.context.object; o.name=name
    o.dimensions=dim; bpy.ops.object.transform_apply(location=False,rotation=False,scale=True); o.data.materials.append(material)
    if bevel:
        b=o.modifiers.new("Soft worn corners","BEVEL"); b.width=bevel; b.segments=2
        bpy.context.view_layer.objects.active=o; bpy.ops.object.modifier_apply(modifier=b.name)
        n=o.modifiers.new("Weighted stone normals","WEIGHTED_NORMAL"); bpy.ops.object.modifier_apply(modifier=n.name)
    return o

def cylinder(name,loc,radius,depth,material,verts=12,bevel=.025,radius2=None):
    if radius2 is None: bpy.ops.mesh.primitive_cylinder_add(vertices=verts,radius=radius,depth=depth,location=loc)
    else: bpy.ops.mesh.primitive_cone_add(vertices=verts,radius1=radius,radius2=radius2,depth=depth,location=loc)
    o=bpy.context.object; o.name=name; o.data.materials.append(material)
    if bevel:
        b=o.modifiers.new("Edge wear","BEVEL"); b.width=bevel; b.segments=2
        bpy.context.view_layer.objects.active=o; bpy.ops.object.modifier_apply(modifier=b.name)
        n=o.modifiers.new("Weighted normals","WEIGHTED_NORMAL"); bpy.ops.object.modifier_apply(modifier=n.name)
    return o

def ico(name,loc,scale,material,subdiv=1):
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=subdiv,radius=1,location=loc); o=bpy.context.object
    o.name=name; o.scale=scale; bpy.ops.object.transform_apply(location=False,rotation=False,scale=True); o.data.materials.append(material); return o

def torus(name,loc,major,minor,material,rotation=(0,0,0)):
    bpy.ops.mesh.primitive_torus_add(major_radius=major,minor_radius=minor,major_segments=16,minor_segments=6,location=loc,rotation=rotation)
    o=bpy.context.object; o.name=name; o.data.materials.append(material); return o

def mesh_obj(name,verts,faces,material):
    me=bpy.data.meshes.new(name); me.from_pydata(verts,[],faces); me.materials.append(material); me.update()
    o=bpy.data.objects.new(name,me); bpy.context.collection.objects.link(o); return o

def finish_asset(name,parts):
    bpy.ops.object.select_all(action="DESELECT")
    for o in parts: o.select_set(True)
    bpy.context.view_layer.objects.active=parts[0]; bpy.ops.object.join()
    o=bpy.context.object; o.name=name; o.data.name=name+"_Mesh"
    bpy.ops.object.origin_set(type="ORIGIN_GEOMETRY",center="BOUNDS")
    o.location.x=0; o.location.y=0
    return o

def build_arch():
    p=[]
    for x in (-1.62,1.62):
        p += [cube("Arch plinth",(x,0,.15),(1.05,.94,.30),STONE_DARK,.06)]
        p += [cube("Arch base course",(x,0,.48),(.84,.82,.35),STONE_LIGHT,.045)]
        for i in range(3): p += [cube("Ashlar column",(x,0,.86+i*.57),(.70,.70,.53),LIMESTONE,.045)]
        p += [cube("Capital",(x,0,1.80),(.92,.84,.25),STONE_LIGHT,.045)]
    r=1.62; z0=1.88
    for i in range(11):
        a=math.pi*i/10; x=r*math.cos(a); z=z0+r*math.sin(a)
        o=cube("Arch voussoir",(x,0,z),(.62,.76,.48),STONE_LIGHT if i%3==0 else LIMESTONE,.045)
        o.rotation_euler[1]=math.radians(90)-a; p.append(o)
    # missing keystone notch reinforces the ruined silhouette
    p.append(cube("Broken crown",(-.23,0,3.64),(.35,.66,.18),STONE_DARK,.025))
    p.append(ico("Fallen block",(2.25,-.15,.17),(.52,.48,.26),LIMESTONE,1))
    return finish_asset("SM_RuinArch_Broken_01",p)

def build_waystone():
    p=[cylinder("Waystone base",(0,0,.14),.88,.28,STONE_DARK,8,.05),cylinder("Inlaid plinth",(0,0,.36),.69,.19,GOLD,8,.025),cylinder("Stone socle",(0,0,.52),.62,.18,STONE_LIGHT,8,.035)]
    o=cylinder("Carved obelisk",(0,0,1.72),.62,2.30,LIMESTONE,6,.03,radius2=.25); o.rotation_euler[2]=math.radians(30); p.append(o)
    p += [cube("Rune channel",(0,-.34,1.55),(.105,.035,1.28),STONE_DARK,.015),cube("Rune inlay",(0,-.37,1.55),(.035,.025,.95),TEAL,.012)]
    for z in (.93,1.48,2.05):
        p.append(cube("Cross rune",(0,-.385,z),(.32,.024,.045),GOLD,.01))
    # cracked side chips
    p += [ico("Stone chip",(.34,-.32,2.83),(.16,.12,.24),STONE_LIGHT,1),ico("Stone chip",(-.43,.24,1.0),(.18,.16,.12),STONE_DARK,1)]
    return finish_asset("SM_Waystone_Checkpoint_01",p)

def build_sword_monument():
    p=[cylinder("Monument dais",(0,0,.16),1.52,.32,STONE_DARK,12,.055),cylinder("Dais upper ring",(0,0,.36),1.28,.16,STONE_LIGHT,12,.035),ico("Broken rock",(0,0,.78),(.93,.76,.68),LIMESTONE,2)]
    # tall sword, blade front is toward -Y
    p.append(mesh_obj("Sword blade",[(-.17,-.48,.95),(.17,-.48,.95),(.13,-.48,3.25),(0,-.48,3.62),(-.13,-.48,3.25)],[(0,1,2,3,4)],STEEL))
    p.append(cube("Sword fuller",(0,-.505,2.22),(.035,.025,1.45),GOLD,.008))
    guard=cube("Sword crossguard",(0,-.48,1.06),(1.20,.20,.16),GOLD,.055); guard.rotation_euler[1]=math.radians(-8); p.append(guard)
    p += [cylinder("Sword grip",(0,-.48,.73),.115,.56,WOOD,8,.018),torus("Grip collar",(0,-.48,.95),.14,.035,GOLD),ico("Sword pommel",(0,-.48,.43),(.18,.16,.18),GOLD,2)]
    return finish_asset("SM_SwordMonument_01",p)

def build_pillar():
    p=[cylinder("Pillar foot",(0,0,.16),.78,.32,STONE_DARK,10,.04),cylinder("Pillar torus",(0,0,.38),.65,.16,STONE_LIGHT,10,.025)]
    for z in (.77,1.31,1.85,2.39):
        p.append(cylinder("Fluted shaft block",(0,0,z),.56,.50,LIMESTONE,10,.028))
        p.append(cylinder("Shaft collar",(0,0,z-.26),.60,.075,STONE_DARK,10,.014))
    # uneven, visibly broken cap with angled slab
    top=cube("Broken angled capital",(0,0,2.76),(1.12,1.08,.33),STONE_LIGHT,.045); top.rotation_euler[1]=math.radians(-13); p.append(top)
    p.append(ico("Missing corner rubble",(.52,.12,2.92),(.27,.29,.15),LIMESTONE,1))
    return finish_asset("SM_RuinPillar_Broken_01",p)

def build_well():
    p=[cylinder("Well foundation",(0,0,.13),1.25,.26,STONE_DARK,12,.04),cylinder("Well lower ring",(0,0,.39),1.05,.25,STONE_LIGHT,12,.035),cylinder("Dark water",(0,0,.55),.74,.035,STONE_DARK,24,.01)]
    # ring stones separated with slight joints
    for row,z in enumerate((.66,.96,1.26)):
        for i in range(12):
            a=math.tau*i/12 + (row%2)*math.pi/12; x=math.cos(a)*.90; y=math.sin(a)*.90
            o=cube("Well masonry",(x,y,z),(.48,.35,.27),STONE_LIGHT if (i+row)%4==0 else LIMESTONE,.035)
            o.rotation_euler[2]=a; p.append(o)
    for x in (-.91,.91):
        p.append(cube("Well upright",(x,0,2.17),(.22,.23,1.65),WOOD,.035))
    p.append(cube("Well crossbeam",(0,0,2.98),(2.18,.24,.23),WOOD_LIGHT,.035))
    p.append(cylinder("Well axle",(0,0,2.78),.12,1.72,WOOD,10,.018))
    p.append(cylinder("Well crank",(1.03,0,2.65),.085,.25,IRON,10,.015))
    p.append(cube("Crank handle",(1.03,-.03,2.38),(.14,.15,.48),WOOD_LIGHT,.025))
    p.append(cylinder("Hanging rope",(0,0,1.84),.025,.95,WOOD_LIGHT,8,.005))
    return finish_asset("SM_AncientWell_01",p)

def build_brazier():
    p=[cylinder("Brazier foot",(0,0,.16),.50,.32,STONE_DARK,8,.035),cylinder("Brazier pedestal",(0,0,.72),.29,.90,IRON,8,.025),cylinder("Brazier collar",(0,0,1.15),.48,.16,GOLD,8,.02)]
    o=cylinder("Brazier bowl",(0,0,1.43),.56,.48,IRON,10,.035,radius2=.30); p.append(o)
    p.append(torus("Brazier rim",(0,0,1.67),.48,.065,GOLD))
    for a in range(4):
        angle=math.tau*a/4
        o=cube("Brazier bracket",(.42*math.cos(angle),.42*math.sin(angle),1.51),(.13,.13,.55),IRON,.02); o.rotation_euler[1]=math.cos(angle)*.20; p.append(o)
    # Three stylized, faceted flame tongues (emissive material slots)
    p.append(mesh_obj("Outer flame",[(-.28,0,1.68),(-.18,0,2.30),(-.02,0,2.04),(.06,0,2.53),(.28,0,1.68)],[(0,1,2,3,4)],FLAME))
    p.append(mesh_obj("Inner flame",[(-.12,-.02,1.70),(0,-.02,2.24),(.16,-.02,1.70)],[(0,1,2)],FLAME_CORE))
    return finish_asset("SM_CampBrazier_01",p)

def build_crate():
    p=[cube("Crate shadow frame",(0,0,.48),(1.35,1.12,.90),WOOD,.035)]
    # front-facing slatted construction; iron corner straps and rivets
    for i in range(5):
        x=-.53+i*.265; p.append(cube("Front plank",(x,-.58,.49),(.235,.075,.76),WOOD_LIGHT if i%2==0 else WOOD,.018))
        p.append(cube("Lid plank",(x,0,.96),(.235,1.08,.085),WOOD_LIGHT if i%2==0 else WOOD,.014))
    for z in (.12,.88): p.append(cube("Front brace",(0,-.64,z),(1.35,.11,.12),GOLD,.018))
    for x in (-.61,.61):
        p.append(cube("Iron corner",(x,-.65,.50),(.11,.035,.78),IRON,.015))
        for z in (.20,.48,.80): p.append(ico("Iron rivet",(x,-.677,z),(.045,.025,.045),GOLD,1))
    return finish_asset("SM_OldSupplyCrate_01",p)

def build_bench():
    p=[cube("Bench stone seat",(0,0,.78),(2.25,.78,.25),STONE_LIGHT,.07)]
    for x in (-.78,.78):
        p.append(cube("Bench trestle",(x,0,.41),(.18,.72,.66),WOOD,.035))
        p.append(cube("Bench crossbar",(x,0,.48),(.60,.16,.16),GOLD,.025))
    p.append(cube("Bench front band",(0,-.405,.75),(2.12,.045,.09),STONE_DARK,.014))
    for x in (-.46,0,.46): p.append(cube("Carved bench rune",(x,-.435,.75),(.10,.02,.035),TEAL,.008))
    return finish_asset("SM_KlingenhofBench_01",p)

BUILDERS=[build_arch,build_waystone,build_sword_monument,build_pillar,build_well,build_brazier,build_crate,build_bench]

# Clean startup scene, then build and export eight self-contained mesh assets.
bpy.ops.object.select_all(action="SELECT"); bpy.ops.object.delete(use_global=False)
for c in list(bpy.data.collections):
    if c.name!="Collection": bpy.data.collections.remove(c)
assets=[]
for build in BUILDERS:
    asset=build(); assets.append(asset)
    bpy.ops.object.select_all(action="DESELECT"); asset.select_set(True); bpy.context.view_layer.objects.active=asset
    path=os.path.join(EXPORT,asset.name+".fbx")
    bpy.ops.export_scene.fbx(filepath=path,use_selection=True,object_types={"MESH"},apply_unit_scale=True,
        apply_scale_options="FBX_SCALE_ALL",axis_forward="-Z",axis_up="Y",use_mesh_modifiers=True,
        add_leaf_bones=False,bake_anim=False,mesh_smooth_type="FACE",path_mode="AUTO")
    print("Exported",path)

# Presentation gallery: 2 rows of four props, individual mesh objects remain editable.
for i,obj in enumerate(assets):
    row=i//4; col=i%4
    obj.location=(col*5.25-7.875, -5.5+row*11.0, obj.location.z)
    obj["AssetType"]="SAOMMO World Prop"
    obj["ExportFBX"]=os.path.join("Exports","FBX",obj.name+".fbx")
    obj["ScaleReference"]="Blender metric units; intended world-scale centimeters on Unreal import"

# Floor and studio lighting, kept outside FBX selections.
bpy.ops.mesh.primitive_plane_add(size=1,location=(0,0,-.03)); floor=bpy.context.object; floor.name="Gallery floor (presentation only)"
floor.dimensions=(200,200,1); bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
floor.data.materials.append(mat("Gallery | charcoal",(.018,.032,.039),0,.92))

def area_light(name,loc,power,color,size,target=(0,0,1)):
    d=bpy.data.lights.new(name,"AREA"); d.energy=power; d.color=color; d.shape="DISK"; d.size=size
    o=bpy.data.objects.new(name,d); bpy.context.collection.objects.link(o); o.location=loc; o.rotation_euler=(Vector(target)-o.location).to_track_quat("-Z","Y").to_euler()
area_light("Key | golden hour",(-10,-14,18),3900,(1.0,.79,.57),10)
area_light("Fill | blue sky",(11,1,16),3100,(.50,.76,1.0),12)
area_light("Rim | teal",(0,13,11),2500,(.25,1.0,.85),9)

scene=bpy.context.scene; scene.name="SAOMMO | World Props Gallery"; scene.render.engine="BLENDER_EEVEE"
scene.render.resolution_x=1920; scene.render.resolution_y=1080; scene.render.resolution_percentage=100
scene.render.image_settings.file_format="PNG"; scene.render.film_transparent=False
scene.view_settings.view_transform="AgX"; scene.view_settings.look="AgX - Medium High Contrast"; scene.view_settings.exposure=.35
scene.world.color=(.055,.075,.095)
scene.render.filepath=os.path.join(ROOT,"renders","SAOMMO_world_props.png")
camd=bpy.data.cameras.new("World Props Gallery Camera"); cam=bpy.data.objects.new("World Props Gallery Camera",camd); bpy.context.collection.objects.link(cam)
cam.location=(0,-36,18); target=Vector((0,0,1.3)); cam.rotation_euler=(target-Vector(cam.location)).to_track_quat("-Z","Y").to_euler(); camd.type="ORTHO"; camd.ortho_scale=22.5; scene.camera=cam

# Facing camera-side label tags in the scene gallery (text does not enter FBX exports).
font=bpy.data.fonts.load("C:/Windows/Fonts/segoeuib.ttf")
for i,obj in enumerate(assets):
    row=i//4; col=i%4; x=col*5.25-7.875; y=-5.5+row*11.0-1.55
    c=bpy.data.curves.new("Label | "+obj.name,"FONT"); c.body=obj.name.replace("SM_","").replace("_"," "); c.size=.24; c.font=font; c.align_x="CENTER"
    t=bpy.data.objects.new("Label | "+obj.name,c); bpy.context.collection.objects.link(t); t.location=(x,y,.12); t.rotation_euler[0]=math.radians(90)
    t.data.materials.append(LABEL)

bpy.context.view_layer.update()
title_curve=bpy.data.curves.new("SAOMMO World Props title","FONT"); title_curve.body="SAOMMO  /  WORLD RELICS"; title_curve.size=.48; title_curve.font=font
title=bpy.data.objects.new("Gallery title",title_curve); bpy.context.collection.objects.link(title)
title.location=cam.matrix_world @ Vector((-10.0,5.7,-18.0)); title.rotation_euler=cam.rotation_euler.copy(); title.data.materials.append(LABEL)

for screen in bpy.data.screens:
    for area in screen.areas:
        if area.type=="VIEW_3D": area.spaces.active.region_3d.view_perspective="CAMERA"

blend=os.path.join(ROOT,"SAOMMO_World_Props.blend")
bpy.ops.wm.save_as_mainfile(filepath=blend)
bpy.ops.render.render(write_still=True)
print("Saved gallery",blend)
