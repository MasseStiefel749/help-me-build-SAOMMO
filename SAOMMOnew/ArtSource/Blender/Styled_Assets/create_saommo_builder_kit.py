import bpy, math, os
from mathutils import Vector

ROOT=os.path.dirname(os.path.abspath(__file__))
EXPORT=os.path.join(ROOT,"Exports","World_Builder_Kit","FBX")
RENDERS=os.path.join(ROOT,"renders")
os.makedirs(EXPORT,exist_ok=True); os.makedirs(RENDERS,exist_ok=True)

def material(name,color,metal=0,rough=.82,glow=0):
    m=bpy.data.materials.new(name); m.diffuse_color=(*color,1); m.use_nodes=True
    nt=m.node_tree; p=nt.nodes.get("Principled BSDF"); p.inputs["Base Color"].default_value=(*color,1); p.inputs["Metallic"].default_value=metal; p.inputs["Roughness"].default_value=rough
    tc=nt.nodes.new("ShaderNodeTexCoord"); noise=nt.nodes.new("ShaderNodeTexNoise"); noise.inputs["Scale"].default_value=32; noise.inputs["Detail"].default_value=3.2; noise.inputs["Roughness"].default_value=.72
    nt.links.new(tc.outputs["Generated"],noise.inputs["Vector"])
    ramp=nt.nodes.new("ShaderNodeValToRGB"); ramp.color_ramp.elements[0].position=.24; ramp.color_ramp.elements[0].color=(*(v*.76 for v in color),1); ramp.color_ramp.elements[1].position=.80; ramp.color_ramp.elements[1].color=(*(min(v*1.16,1) for v in color),1)
    nt.links.new(noise.outputs["Fac"],ramp.inputs["Fac"]); nt.links.new(ramp.outputs["Color"],p.inputs["Base Color"])
    bump=nt.nodes.new("ShaderNodeBump"); bump.inputs["Strength"].default_value=.12; bump.inputs["Distance"].default_value=.045; nt.links.new(noise.outputs["Fac"],bump.inputs["Height"]); nt.links.new(bump.outputs["Normal"],p.inputs["Normal"])
    if glow: p.inputs["Emission Color"].default_value=(*color,1); p.inputs["Emission Strength"].default_value=glow
    return m

GRASS=material("M_World_Grass",(.17,.34,.15)); GRASS_LIGHT=material("M_World_GrassTips",(.37,.51,.21))
DIRT=material("M_World_Dirt",(.31,.21,.13)); DIRT_LIGHT=material("M_Dirt_Loose",(.48,.34,.20))
STONE=material("M_World_Limestone",(.48,.46,.38)); STONE_LIGHT=material("M_Stone_WornEdges",(.66,.60,.47)); STONE_DARK=material("M_Stone_Shadow",(.18,.23,.25))
ROCK=material("M_Rock",(.31,.34,.32)); WOOD=material("M_Wood_Oak",(.32,.19,.10)); WOOD_LIGHT=material("M_Wood_Edge",(.54,.35,.19))
IRON=material("M_Iron",(.17,.24,.28),.55,.36); BRASS=material("M_Brass",(.72,.52,.24),.68,.3)
LEAF=material("M_Leaves_Pine",(.08,.27,.16)); LEAF_LIGHT=material("M_Leaves_Light",(.22,.41,.20)); LEAF_DARK=material("M_Leaves_Shadow",(.035,.14,.085))
RUNE=material("M_Rune_Glow",(.05,.67,.91),.42,.24,1.65)
ASSET_NAMES=[]

def cube(name,loc,dims,mat,bevel=.025):
    bpy.ops.mesh.primitive_cube_add(size=1,location=loc); o=bpy.context.object; o.name=name
    o.dimensions=dims; bpy.ops.object.transform_apply(location=False,rotation=False,scale=True); o.data.materials.append(mat)
    if bevel:
        b=o.modifiers.new("Small edge wear","BEVEL"); b.width=bevel; b.segments=2; bpy.context.view_layer.objects.active=o; bpy.ops.object.modifier_apply(modifier=b.name)
        n=o.modifiers.new("Weighted normals","WEIGHTED_NORMAL"); bpy.ops.object.modifier_apply(modifier=n.name)
    return o

def cylinder(name,loc,radius,depth,mat,verts=10,bevel=.018,radius2=None):
    if radius2 is None: bpy.ops.mesh.primitive_cylinder_add(vertices=verts,radius=radius,depth=depth,location=loc)
    else: bpy.ops.mesh.primitive_cone_add(vertices=verts,radius1=radius,radius2=radius2,depth=depth,location=loc)
    o=bpy.context.object; o.name=name; o.data.materials.append(mat)
    if bevel:
        b=o.modifiers.new("Edge wear","BEVEL"); b.width=bevel; b.segments=2; bpy.context.view_layer.objects.active=o; bpy.ops.object.modifier_apply(modifier=b.name)
        n=o.modifiers.new("Weighted normals","WEIGHTED_NORMAL"); bpy.ops.object.modifier_apply(modifier=n.name)
    return o

def ico(name,loc,scale,mat,sub=1):
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=sub,radius=1,location=loc); o=bpy.context.object; o.name=name; o.scale=scale
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True); o.data.materials.append(mat); return o

def mesh(name,verts,faces,mat):
    d=bpy.data.meshes.new(name); d.from_pydata(verts,[],faces); d.materials.append(mat); d.update(); o=bpy.data.objects.new(name,d); bpy.context.collection.objects.link(o); return o

def join_asset(name,parts,grid="Free prop"):
    bpy.ops.object.select_all(action="DESELECT")
    for o in parts: o.select_set(True)
    bpy.context.view_layer.objects.active=parts[0]; bpy.ops.object.join(); o=bpy.context.object
    o.name=name; o.data.name=name+"_Mesh"; bpy.context.scene.cursor.location=(0,0,0)
    bpy.ops.object.origin_set(type="ORIGIN_CURSOR"); o.location=(0,0,0)
    o["WorldKitCategory"]=grid; o["PivotConvention"]="Bottom center at world origin; modular tile edges align to 4m grid where applicable"
    ASSET_NAMES.append(name); return o

def tile_base(mat,top=0,bottom=-.24): return cube("Tile base",(0,0,(top+bottom)/2),(4,4,top-bottom),mat,.035)

def ground_grass():
    p=[tile_base(GRASS)]
    for i in range(24):
        x=-1.8+(i*1.618%3.6); y=-1.8+(i*2.414%3.6); h=.10+(i%4)*.035
        p.append(mesh("Grass tuft",[(x-.04,y,-.002),(x,y,-.002),(x,y,h)],[(0,1,2)],GRASS_LIGHT))
        p.append(mesh("Grass tuft",[(x,y,-.002),(x+.05,y,-.002),(x+.01,y,h*.8)],[(0,1,2)],LEAF))
    for x,y,s in [(-1.4,-.8,.22),(.6,1.2,.16),(1.4,-1.3,.2)]: p.append(ico("Small ground stone",(x,y,.025),(s,s*.7,.06),STONE_LIGHT,1))
    return join_asset("SM_Terrain_Grass_4m",p,"Terrain / 4m grid")

def ground_dirt():
    p=[tile_base(DIRT)]
    for i in range(18):
        x=-1.7+(i*1.137%3.4); y=-1.7+(i*2.213%3.4); r=.12+(i%3)*.035
        p.append(ico("Dirt clod",(x,y,.015),(r,r*.65,.035),DIRT_LIGHT if i%3==0 else STONE_DARK,1))
    return join_asset("SM_Terrain_Dirt_4m",p,"Terrain / 4m grid")

def path_stone():
    p=[tile_base(GRASS)]
    # centered 2m lane runs along +Y and repeats seamlessly at tile edges
    for row in range(6):
        y=-1.66+row*.66
        for col in range(3):
            x=-.68+col*.68+(.16 if row%2 else 0)
            p.append(cube("Path paver",(x,y,.035),(.62,.58,.11),STONE_LIGHT if (row+col)%4==0 else STONE,.045))
    return join_asset("SM_Path_Stone_4m",p,"Terrain / 4m grid")

def path_dirt():
    p=[tile_base(GRASS)]
    p.append(cube("Dirt path",(0,0,.012),(2.0,4.0,.035),DIRT,.08))
    for i in range(28):
        x=-.82+(i*1.271%1.64); y=-1.85+(i*2.031%3.7); s=.09+(i%4)*.025
        p.append(ico("Path pebble",(x,y,.04),(s,s*.7,.035),DIRT_LIGHT if i%3 else STONE_DARK,1))
    return join_asset("SM_Path_Dirt_4m",p,"Terrain / 4m grid")

def cliff_edge():
    p=[tile_base(GRASS)]
    # terrain overhang along north (+Y) edge, rock drops 2.5m below tile
    for i in range(8):
        x=-1.75+i*.5; z=-1.05-(i%3)*.18; w=.52
        p.append(ico("Cliff face",(x,1.7,z),(w,.50,1.15+(i%2)*.28),ROCK if i%3 else STONE_DARK,1))
    for x in (-1.4,-.4,.6,1.5): p.append(ico("Cliff lip stone",(x,1.72,-.02),(.62,.48,.28),STONE_LIGHT,1))
    return join_asset("SM_Terrain_CliffEdge_4m",p,"Terrain / 4m grid")

def rock_cluster(size,name):
    p=[]
    specs={"Small": [(-.32,0,.27,(.48,.42,.43)),(.25,.05,.20,(.38,.34,.32)),(.02,.30,.17,(.29,.26,.24))],
           "Medium":[(-.58,0,.45,(.72,.56,.62)),(.25,.02,.30,(.57,.51,.45)),(.73,.04,.22,(.40,.39,.38)),(-.22,.33,.32,(.44,.38,.33))],
           "Large":[(-.75,0,.70,(.88,.70,.88)),(.25,0,.54,(.82,.67,.73)),(.88,.12,.35,(.57,.55,.52)),(-.12,.42,.52,(.62,.55,.50))]}[size]
    for i,(x,y,z,s) in enumerate(specs): p.append(ico("Faceted boulder",(x,y,z),s,ROCK if i%2 else STONE_DARK,2))
    return join_asset(name,p,"Nature / rocks")

def floor_stone(damaged=False):
    p=[tile_base(STONE_DARK if damaged else STONE)]
    for row in range(4):
        for col in range(4):
            if damaged and (row,col) in [(0,2),(3,0)]: continue
            x=-1.5+col; y=-1.5+row; mat=STONE_LIGHT if (row+col)%4==0 else STONE
            block=cube("Floor flagstone",(x,y,.035),(0.96,.96,.09),mat,.04)
            if damaged and (row,col)==(2,2): block.rotation_euler[2]=math.radians(7)
            p.append(block)
    if damaged: p.append(ico("Broken paving",(1.35,-1.26,.02),(.52,.30,.10),STONE_LIGHT,1))
    return join_asset("SM_Floor_Stone_Damaged_4m" if damaged else "SM_Floor_Stone_4m",p,"Architecture / 4m grid")

def wall_stone(style):
    p=[]
    def segment(y=0):
        pieces=[]
        for x in (-1.5,-.5,.5,1.5):
            for row in range(3):
                z=.5+row
                if style=="door" and abs(x)<1.0 and row<2: continue
                if style=="window" and abs(x)<1.0 and row==1: continue
                pieces.append(cube("Ashlar block",(x,y,z),(.98,.38,.96),STONE_LIGHT if (row+int(x*2))%5==0 else STONE,.035))
        pieces.append(cube("Wall footing",(0,y,.12),(4.18,.54,.24),STONE_DARK,.035))
        pieces.append(cube("Wall coping",(0,y,3.08),(4.16,.56,.20),STONE_LIGHT,.04))
        if style=="window":
            pieces.append(cube("Window sill",(0,y,1.00),(2.08,.70,.18),STONE_LIGHT,.03))
            pieces.append(cube("Window lintel",(0,y,2.08),(2.08,.60,.18),STONE_LIGHT,.03))
        return pieces
    if style=="corner":
        p+=segment(-1.8)
        for o in segment(0): o.rotation_euler[2]=math.pi/2; o.location.x+=1.8; p.append(o)
        return join_asset("SM_Wall_Corner_Stone_4m",p,"Architecture / grid corner")
    if style=="half":
        for x in (-1.5,-.5,.5,1.5): p.append(cube("Half wall ashlar",(x,0,.68),(.98,.42,1.30),STONE,.04))
        p += [cube("Half wall base",(0,0,.10),(4.12,.58,.20),STONE_DARK,.035),cube("Half wall coping",(0,0,1.38),(4.2,.70,.18),STONE_LIGHT,.04)]
        return join_asset("SM_Wall_Half_Stone_4x1p5",p,"Architecture / 4m grid")
    p+=segment(0)
    if style=="solid": name="SM_Wall_Stone_4x3"
    elif style=="door": name="SM_Wall_Doorway_Stone_4x3"
    else: name="SM_Wall_Window_Stone_4x3"
    return join_asset(name,p,"Architecture / 4m grid")

def pillar():
    p=[cube("Pillar base",(0,0,.14),(1.25,1.25,.28),STONE_DARK,.05)]
    for z in (.48,1.02,1.56,2.10,2.64): p.append(cube("Pillar shaft",(0,0,z),(.82,.82,.50),STONE,.035))
    p += [cube("Pillar neck",(0,0,2.91),(1.05,1.05,.20),STONE_LIGHT,.035),cube("Pillar capital",(0,0,3.12),(1.36,1.36,.22),STONE_LIGHT,.04)]
    return join_asset("SM_Pillar_Stone_3m",p,"Architecture / vertical")

def stairs():
    p=[]
    for i in range(8):
        depth=.5; h=.25*(i+1); y=-1.75+i*.5
        p.append(cube("Stone stair",(0,y,h/2),(4,depth,h),STONE_LIGHT if i%3==0 else STONE,.025))
    return join_asset("SM_Stairs_Stone_4x4",p,"Architecture / 4m grid")

def ramp():
    verts=[(-2,-2,0),(2,-2,0),(2,2,0),(-2,2,0),(-2,-2,0),(2,-2,0),(2,2,2),(-2,2,2)]
    faces=[(0,1,2,3),(4,7,6,5),(0,4,5,1),(1,5,6,2),(2,6,7,3),(3,7,4,0)]
    p=[mesh("Stone ramp",verts,faces,STONE)]
    for i in range(7):
        y=-1.65+i*.55; z=.05+i*.275
        p.append(cube("Ramp grip strip",(0,y,z),(3.82,.08,.05),STONE_LIGHT,.012))
    return join_asset("SM_Ramp_Stone_4x4",p,"Architecture / 4m grid")

def roof_half():
    # half-gable roof slopes from ridge at y=-2,z=3.4 down to eave y=2,z=1.5
    v=[(-2,-2,3.4),(2,-2,3.4),(2,2,1.5),(-2,2,1.5),(-2,-2,3.18),(2,-2,3.18),(2,2,1.28),(-2,2,1.28)]
    f=[(0,1,2,3),(4,7,6,5),(0,4,5,1),(1,5,6,2),(2,6,7,3),(3,7,4,0)]
    p=[mesh("Roof half",v,f,WOOD)]
    for x in (-1.75,-.9,0,.9,1.75):
        z=3.4-(x*0) # rafters run down the slope on y
        o=cube("Roof batten",(x,0,2.45),(.10,4.35,.13),WOOD_LIGHT,.02); o.rotation_euler[0]=math.radians(-25.5); p.append(o)
    for y in (-1.35,-.45,.45,1.35):
        z=3.4-(y+2)*.475
        p.append(cube("Roof tile band",(0,y,z),(4.05,.12,.10),STONE_DARK,.018))
    return join_asset("SM_Roof_GableHalf_4m",p,"Architecture / roof")

def roof_ridge():
    v=[(-2,0,0),(2,0,0),(2,-.5,.5),(-2,-.5,.5),(2,.5,.5),(-2,.5,.5)]
    f=[(0,1,2,3),(0,5,4,1),(1,4,2),(0,3,5),(3,2,4,5)]
    p=[mesh("Ridge cap",v,f,STONE_LIGHT)]
    for x in (-1.5,-.5,.5,1.5): p.append(cube("Ridge cap seam",(x,0,.43),(.08,1.0,.10),BRASS,.02))
    return join_asset("SM_Roof_Ridge_4m",p,"Architecture / roof")

def timber_wall():
    p=[cube("Timber wall backing",(0,0,1.5),(4,.25,3),WOOD,.035)]
    for x in (-1.85,1.85): p.append(cube("Timber upright",(x,-.20,1.5),(.22,.22,3.05),WOOD_LIGHT,.025))
    for z in (.25,1.45,2.85): p.append(cube("Timber crossbeam",(0,-.22,z),(4.1,.25,.22),WOOD_LIGHT,.03))
    for x in (-1.3,-.65,0,.65,1.3): p.append(cube("Infill plank",(x,-.145,1.55),(.12,.035,2.45),WOOD,.012))
    return join_asset("SM_Wall_Timber_4x3",p,"Architecture / 4m grid")

def door():
    p=[cube("Door slab",(0,0,1.45),(2,.20,2.9),WOOD,.04)]
    for x in (-1.10,1.10): p.append(cube("Door jamb",(x,0,1.5),(.20,.46,3.0),STONE_LIGHT,.035))
    p.append(cube("Door lintel",(0,0,3.05),(2.4,.46,.24),STONE_LIGHT,.035))
    for x in (-.65,-.2,.25,.70): p.append(cube("Door board",(x,-.12,1.45),(.055,.025,2.55),WOOD_LIGHT,.012))
    p.append(cube("Door brace",(0,-.18,.48),(1.9,.08,.14),IRON,.02))
    p.append(cylinder("Door handle",(.68,-.24,1.45),.07,.06,BRASS,8,.006))
    return join_asset("SM_Door_Wood_2x3",p,"Architecture / door")

def fence(corner=False):
    p=[]
    spans=[(0,0,4,False),(2,2,4,True)] if corner else [(0,0,4,False)]
    for cx,cy,length,rot in spans:
        a=math.pi/2 if rot else 0
        for t in (-length/2,length/2):
            x=cx+t*math.cos(a); y=cy+t*math.sin(a); p.append(cube("Fence post",(x,y,.76),(.24,.24,1.52),WOOD_LIGHT,.025))
            p.append(cube("Post cap",(x,y,1.56),(.34,.34,.12),STONE_LIGHT,.025))
        for z in (.45,1.15):
            o=cube("Fence rail",(cx,cy,z),(length,.16,.14),WOOD,.02); o.rotation_euler[2]=a; p.append(o)
        for i in range(1,8):
            t=-length/2+i*length/8; x=cx+t*math.cos(a); y=cy+t*math.sin(a)
            o=cube("Fence picket",(x,y,.78),(.10,.12,.75),WOOD_LIGHT,.015); o.rotation_euler[2]=a; p.append(o)
    return join_asset("SM_Fence_Corner_4m" if corner else "SM_Fence_Straight_4m",p,"Settlement / fence")

def bridge():
    p=[]
    for x in (-1.25,1.25): p.append(cube("Bridge stringer",(x,0,.42),(.24,4,.55),WOOD,.035))
    for i in range(9):
        y=-1.75+i*.44; p.append(cube("Bridge plank",(0,y,.72),(2.8,.40,.16),WOOD_LIGHT if i%3==0 else WOOD,.02))
    for x in (-1.46,1.46):
        for y in (-1.8,1.8): p.append(cube("Bridge footing",(x,y,.16),(.48,.48,.32),STONE_DARK,.035))
    return join_asset("SM_Bridge_Segment_4m",p,"Settlement / bridge")

def pine(height,name):
    p=[]; trunk=height*.22
    p.append(cylinder("Pine trunk",(0,0,trunk/2),.17*height/4,trunk,WOOD,8,.018,radius2=.12*height/4))
    tiers=3 if height>3 else 2
    for i in range(tiers):
        z=trunk*.70+i*(height-trunk*.7)/(tiers+0.2); r=height*(.25-.045*i); h=height*.42
        p.append(cylinder("Pine bough tier",(0,0,z+h*.32),r,h,LEAF if i%2 else LEAF_DARK,8,.02,radius2=.025))
        p.append(ico("Foliage color facets",(.12*r,-.08*r,z+h*.36),(r*.68,r*.65,h*.25),LEAF_LIGHT,1))
    return join_asset(name,p,"Nature / tree")

def broadleaf():
    p=[cylinder("Tree trunk",(0,0,1.45),.24,2.9,WOOD,9,.02,radius2=.14)]
    for loc,sc,ma in [((0,0,3.7),(1.1,.95,1.1),LEAF),((-.65,.05,3.15),(.78,.75,.78),LEAF_DARK),((.66,.02,3.25),(.78,.72,.82),LEAF_LIGHT),((.05,.3,4.35),(.72,.70,.74),LEAF_LIGHT)]: p.append(ico("Canopy cluster",loc,sc,ma,2))
    for x,y,z in [(-.52,0,2.55),(.52,0,2.7),(0,.12,2.85)]:
        o=cylinder("Tree branch",(x/2,y,z),.095,.92,WOOD,7,.01,radius2=.05); o.rotation_euler[1]=math.copysign(math.radians(45),x); p.append(o)
    return join_asset("SM_Tree_Deciduous_01",p,"Nature / tree")

def grass_clump():
    p=[]
    for i in range(12):
        a=math.tau*i/12; x=math.cos(a)*.25; y=math.sin(a)*.25; h=.45+(i%4)*.12
        p.append(mesh("Grass blade",[(x-.07,y,0),(x+.07,y,0),(x+.015,y,h)],[(0,1,2)],GRASS_LIGHT if i%3 else LEAF))
    p.append(ico("Grass base",(0,0,.08),(.35,.31,.12),GRASS,1))
    return join_asset("SM_Grass_Clump_01",p,"Nature / foliage")

def fallen_log():
    p=[cylinder("Fallen log",(0,0,.42),.39,2.8,WOOD,9,.025,radius2=.32)]
    for x in (-1.20,1.20): p.append(cylinder("Cut end",(x,0,.42),.32,.05,WOOD_LIGHT,9,.01))
    for x in (-.65,.4,1.0):
        o=cylinder("Broken branch",(x,.12,.68),.11,.62,WOOD_LIGHT,7,.012,radius2=.065); o.rotation_euler[1]=math.radians(55); p.append(o)
    return join_asset("SM_FallenLog_01",p,"Nature / forest")

def stump():
    p=[cylinder("Tree stump",(0,0,.42),.48,.84,WOOD,10,.025,radius2=.39),cylinder("Cut stump top",(0,0,.86),.39,.045,WOOD_LIGHT,10,.008)]
    for r in (.12,.23,.32): p.append(cylinder("Growth ring",(0,0,.887),r,.006,WOOD,16,.001))
    return join_asset("SM_Stump_01",p,"Nature / forest")

def rubble():
    p=[]
    for i in range(11):
        x=-1.0+(i*1.151%2.0); y=-.55+(i*1.81%1.1); z=.16+(i%3)*.18
        p.append(ico("Ruin rubble",(x,y,z),(.42+(i%2)*.14,.35,.27+(i%3)*.10),STONE_LIGHT if i%4==0 else STONE,1))
    return join_asset("SM_Ruin_RubbleCluster_01",p,"Nature / ruins")

def waystone():
    p=[cylinder("Waystone base",(0,0,.12),.75,.24,STONE_DARK,8,.035),cylinder("Gold inlay",(0,0,.29),.61,.10,BRASS,8,.015)]
    p.append(cylinder("Standing stone",(0,0,1.28),.56,1.95,STONE_LIGHT,6,.025,radius2=.22))
    p += [cube("Rune groove",(0,-.31,1.25),(.07,.025,.90),STONE_DARK,.01),cube("Rune light",(0,-.33,1.25),(.025,.02,.68),RUNE,.005)]
    return join_asset("SM_Waystone_Rune_01",p,"Settlement / landmark")

def lantern():
    p=[cylinder("Lantern post foot",(0,0,.16),.32,.32,STONE_DARK,8,.025),cylinder("Lantern post",(0,0,1.55),.12,2.5,WOOD,8,.015)]
    p += [cube("Lantern crossarm",(0,0,2.75),(1.05,.13,.13),WOOD_LIGHT,.02),cube("Lantern roof",(0,0,3.12),(.76,.62,.14),IRON,.035)]
    for x in (-.28,.28):
        for y in (-.22,.22): p.append(cube("Lantern frame",(x,y,2.89),(.055,.055,.42),IRON,.01))
    p.append(cube("Lantern glow",(0,0,2.88),(.42,.34,.30),RUNE,.035))
    return join_asset("SM_LanternPost_01",p,"Settlement / light")

def cairn():
    p=[]
    for i,(z,r) in enumerate(((.18,.56),(.50,.46),(.79,.35),(1.02,.23))):
        o=ico("Cairn stone",(0,0,z),(r,r*.85,r*.48),STONE_LIGHT if i%2 else STONE,1); o.rotation_euler[2]=i*.6; p.append(o)
    p.append(cylinder("Cairn rune cap",(0,0,1.31),.14,.12,BRASS,6,.015))
    return join_asset("SM_Cairn_01",p,"Settlement / landmark")

def signpost():
    p=[cube("Sign post",(0,0,1.30),(.16,.16,2.6),WOOD,.02),cube("Sign board",(0,-.12,2.05),(1.6,.22,.64),WOOD_LIGHT,.04)]
    for x in (-.50,-.25,0,.25,.50): p.append(cube("Sign engraving",(x,-.24,2.05),(.035,.018,.24),WOOD,.006))
    p.append(cube("Sign cap",(0,-.12,2.42),(1.78,.28,.12),STONE_LIGHT,.02))
    return join_asset("SM_Signpost_01",p,"Settlement / marker")

def bridge_rail(): return None

BUILDERS=[
    ground_grass,ground_dirt,path_stone,path_dirt,cliff_edge,
    lambda:rock_cluster("Small","SM_RockCluster_Small"),lambda:rock_cluster("Medium","SM_RockCluster_Medium"),lambda:rock_cluster("Large","SM_RockCluster_Large"),
    lambda:floor_stone(False),lambda:floor_stone(True),lambda:wall_stone("solid"),lambda:wall_stone("door"),lambda:wall_stone("window"),lambda:wall_stone("half"),lambda:wall_stone("corner"),
    pillar,stairs,ramp,roof_half,roof_ridge,timber_wall,door,lambda:fence(False),lambda:fence(True),bridge,
    lambda:pine(2.5,"SM_Tree_Pine_Small"),lambda:pine(4.2,"SM_Tree_Pine_Tall"),broadleaf,grass_clump,fallen_log,stump,rubble,waystone,lantern,cairn,signpost
]

# Build all sources centered on their intended grid pivot, then export one FBX per mesh.
bpy.ops.object.select_all(action="SELECT"); bpy.ops.object.delete(use_global=False)
assets=[]
for build in BUILDERS:
    asset=build(); assets.append(asset)
    bpy.ops.object.select_all(action="DESELECT"); asset.select_set(True); bpy.context.view_layer.objects.active=asset
    fbx=os.path.join(EXPORT,asset.name+".fbx")
    bpy.ops.export_scene.fbx(filepath=fbx,use_selection=True,object_types={"MESH"},apply_unit_scale=True,
        apply_scale_options="FBX_SCALE_ALL",axis_forward="-Z",axis_up="Y",use_mesh_modifiers=True,
        add_leaf_bones=False,bake_anim=False,mesh_smooth_type="FACE",path_mode="AUTO")
    print("Exported",asset.name)

# Stage a six-column catalog. Modular pieces remain individual meshes, selected by name in Blender.
for i,o in enumerate(assets):
    row=i//6; col=i%6; o.location=(col*5.0-12.5,-12.5+row*5.0,0)
    o["GridUnitMeters"]=4.0

# Gallery-only ground, camera and lighting are not included in per-object FBX exports.
bpy.ops.mesh.primitive_plane_add(size=1,location=(0,0,-.28)); floor=bpy.context.object; floor.name="Gallery floor (not exported)"
floor.dimensions=(200,200,1); bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
floor.data.materials.append(material("M_Catalog_Backdrop",(.025,.045,.048),0,.95))

def light(name,loc,power,color,size,target=(0,0,1)):
    d=bpy.data.lights.new(name,"AREA"); d.energy=power; d.color=color; d.shape="DISK"; d.size=size
    o=bpy.data.objects.new(name,d); bpy.context.collection.objects.link(o); o.location=loc; o.rotation_euler=(Vector(target)-o.location).to_track_quat("-Z","Y").to_euler()
light("Warm key",(-14,-18,25),5600,(1.0,.78,.55),14)
light("Cool fill",(14,0,26),4100,(.60,.78,1.0),18)
light("Teal rim",(0,20,18),4200,(.36,1.0,.84),15)

scene=bpy.context.scene; scene.name="SAOMMO | Modular World Builder Kit"; scene.render.engine="BLENDER_EEVEE"
scene.render.resolution_x=2200; scene.render.resolution_y=1500; scene.render.resolution_percentage=100
scene.render.image_settings.file_format="PNG"; scene.view_settings.view_transform="AgX"; scene.view_settings.look="AgX - Medium High Contrast"; scene.view_settings.exposure=.35; scene.render.filepath=os.path.join(RENDERS,"SAOMMO_world_builder_kit.png")
world=bpy.data.worlds.new("World Kit Night Studio") if not bpy.data.worlds else bpy.data.worlds[0]
scene.world=world; world.use_nodes=True; world.node_tree.nodes["Background"].inputs["Color"].default_value=(.055,.075,.095,1); world.node_tree.nodes["Background"].inputs["Strength"].default_value=.42
cd=bpy.data.cameras.new("World Kit Catalog Camera"); cam=bpy.data.objects.new("World Kit Catalog Camera",cd); bpy.context.collection.objects.link(cam)
cam.location=(0,-48,77); target=Vector((0,0,1.6)); cam.rotation_euler=(target-Vector(cam.location)).to_track_quat("-Z","Y").to_euler(); cd.type="ORTHO"; cd.ortho_scale=40; scene.camera=cam

font=bpy.data.fonts.load("C:/Windows/Fonts/segoeuib.ttf")
for i,o in enumerate(assets):
    row=i//6; col=i%6; x=col*5.0-12.5; y=-12.5+row*5.0-2.15
    c=bpy.data.curves.new("Catalog label | "+o.name,"FONT"); c.body=o.name.replace("SM_","").replace("_"," "); c.size=.24; c.font=font; c.align_x="CENTER"
    t=bpy.data.objects.new("Catalog label | "+o.name,c); bpy.context.collection.objects.link(t); t.location=(x,y,.025); t.rotation_euler[0]=math.radians(90)
    t.data.materials.append(material("M_CatalogLabel_"+str(i),(.94,.76,.45),0,.6,1.2))

bpy.context.view_layer.update()
title_curve=bpy.data.curves.new("Builder kit title","FONT"); title_curve.body="SAOMMO  /  MODULAR WORLD KIT"; title_curve.size=.72; title_curve.font=font
title=bpy.data.objects.new("Builder kit title",title_curve); bpy.context.collection.objects.link(title)
title.location=cam.matrix_world @ Vector((-19,12,-25)); title.rotation_euler=cam.rotation_euler.copy()
title.data.materials.append(material("M_BuilderKitTitle",(.95,.76,.43),0,.55,1.5))

for screen in bpy.data.screens:
    for area in screen.areas:
        if area.type=="VIEW_3D": area.spaces.active.region_3d.view_perspective="CAMERA"

blend=os.path.join(ROOT,"SAOMMO_World_Builder_Kit.blend")
bpy.ops.wm.save_as_mainfile(filepath=blend)
bpy.ops.render.render(write_still=True)
print("Built",len(assets),"world builder assets at",blend)
