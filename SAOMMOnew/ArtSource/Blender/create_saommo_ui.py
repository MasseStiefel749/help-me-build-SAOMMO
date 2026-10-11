import bpy, math, os, sys
from mathutils import Vector

ROOT = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(ROOT, "renders")
os.makedirs(OUT, exist_ok=True)
W, H = 1920, 1080

def emission(name, color, strength=1.0):
    m = bpy.data.materials.new(name); m.diffuse_color = (*color, 1)
    m.use_nodes = True; n = m.node_tree.nodes; n.clear()
    out = n.new("ShaderNodeOutputMaterial"); e = n.new("ShaderNodeEmission")
    e.inputs["Color"].default_value = (*color, 1); e.inputs["Strength"].default_value = strength
    m.node_tree.links.new(e.outputs[0], out.inputs["Surface"]); return m

INK = emission("Ink | deep blue-black", (0.012,0.025,0.039))
PANEL = emission("Panel | midnight slate", (0.025,0.055,0.073))
PANEL2 = emission("Panel | raised slate", (0.04,0.082,0.098))
GOLD = emission("Accent | aged gold", (0.76,0.56,0.25), 1.1)
GLOW = emission("Accent | moonlit teal", (0.27,0.70,0.68), 1.2)
WHITE = emission("Text | warm ivory", (0.91,0.88,0.78), 1.15)
MUTED = emission("Text | muted silver", (0.46,0.56,0.57))
RED = emission("Vital | health red", (0.65,0.20,0.17))
GREEN = emission("Vital | stamina green", (0.28,0.60,0.40))
SHADOW = emission("Scenery | shadow", (0.018,0.04,0.052))
MIST = emission("Scenery | distant mist", (0.075,0.17,0.19))
STONE = emission("Scenery | weathered stone", (0.11,0.17,0.17))
FONT_TITLE = bpy.data.fonts.load("C:/Windows/Fonts/georgia.ttf")
FONT_BOLD = bpy.data.fonts.load("C:/Windows/Fonts/segoeuib.ttf")
FONT_BODY = bpy.data.fonts.load("C:/Windows/Fonts/segoeui.ttf")

def reset_scene(name):
    bpy.ops.object.select_all(action="SELECT"); bpy.ops.object.delete(use_global=False)
    for datablocks in (bpy.data.meshes, bpy.data.curves, bpy.data.cameras, bpy.data.lights):
        pass
    s = bpy.context.scene; s.name = name; s.render.engine = "BLENDER_EEVEE"
    s.render.resolution_x=W; s.render.resolution_y=H; s.render.resolution_percentage=100
    s.render.image_settings.file_format="PNG"; s.render.film_transparent=False
    s.view_settings.view_transform="Standard"; s.view_settings.look="Medium High Contrast"
    s.render.resolution_percentage=100
    camd=bpy.data.cameras.new(name+" Camera"); cam=bpy.data.objects.new(name+" Camera",camd); s.collection.objects.link(cam)
    cam.location=(0,0,30); cam.rotation_euler=(0,0,0); camd.type="ORTHO"; camd.ortho_scale=19.2; s.camera=cam
    s.world.color=(0.012,0.025,0.039)
    return s

def rect(name,x,y,w,h,mat,z=0, bevel=0):
    bpy.ops.mesh.primitive_plane_add(size=1, location=(x,y,z)); o=bpy.context.object; o.name=name
    o.dimensions=(w,h,1); bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    o.data.materials.append(mat)
    if bevel:
        sol=o.modifiers.new("Card edge", "SOLIDIFY"); sol.thickness=0.012
        b=o.modifiers.new("Soft card corners", "BEVEL"); b.width=bevel; b.segments=4
    return o

def poly(name, pts, mat, z=-1):
    mesh=bpy.data.meshes.new(name); mesh.from_pydata([(x,y,z) for x,y in pts],[],[list(range(len(pts)))]); mesh.materials.append(mat)
    o=bpy.data.objects.new(name,mesh); bpy.context.scene.collection.objects.link(o); return o

def line(name, pts, mat, width=.018, z=1):
    c=bpy.data.curves.new(name,"CURVE"); c.dimensions="3D"; c.bevel_depth=width; c.bevel_resolution=3
    sp=c.splines.new("POLY"); sp.points.add(len(pts)-1)
    for p,co in zip(sp.points,pts): p.co=(co[0],co[1],z,1)
    o=bpy.data.objects.new(name,c); bpy.context.scene.collection.objects.link(o); o.data.materials.append(mat); return o

def circle(name,x,y,r,mat,width=.018,z=2):
    pts=[(x+r*math.cos(i*math.tau/96), y+r*math.sin(i*math.tau/96)) for i in range(97)]
    return line(name,pts,mat,width,z)

def text(body,x,y,size,mat=WHITE,font=FONT_BODY,align="LEFT",z=3,name=None):
    c=bpy.data.curves.new(name or body[:20],"FONT"); c.body=body; c.size=size; c.font=font; c.align_x=align
    c.extrude=0; c.space_character=1.08; o=bpy.data.objects.new(name or body[:20],c); o.location=(x,y,z); bpy.context.scene.collection.objects.link(o); o.data.materials.append(mat); return o

def label(body,x,y,size=.16,mat=MUTED): return text(body.upper(),x,y,size,mat,FONT_BOLD)

def scene_backdrop():
    rect("Night canvas",0,0,19.2,10.8,INK,-8)
    # Moon, distant layered peaks, and ruined keep create an original fantasy landscape.
    circle("Moon halo",5.9,3.1,1.23,MIST,.024,-6)
    bpy.ops.mesh.primitive_circle_add(vertices=96, radius=1.02, fill_type="NGON", location=(5.9,3.1,-5.9)); bpy.context.object.name="Moon"; bpy.context.object.data.materials.append(emission("Moon | pale",(.50,.68,.64)))
    poly("Far ridge",[(-9.6,-1.0),(-7.1,1.4),(-5.3,.2),(-3.0,2.15),(-.8,.1),(1.3,1.0),(3.3,-.1),(5.4,1.45),(7.8,-.1),(9.6,.7),(9.6,-5.4),(-9.6,-5.4)],MIST,-5)
    poly("Near ridge",[(-9.6,-1.3),(-7.9,-.1),(-6.3,-.7),(-4.0,.05),(-1.7,-1.0),(.4,-.25),(2.3,-1.2),(4.2,-.45),(6.0,-1.35),(8.2,-.7),(9.6,-1.1),(9.6,-5.4),(-9.6,-5.4)],SHADOW,-4)
    # broken arch / tower silhouette
    poly("Broken keep",[(3.6,-.8),(3.6,1.0),(3.92,1.0),(3.92,2.05),(4.72,2.05),(4.72,1.12),(5.05,1.12),(5.05,-.8)],SHADOW,-3)
    poly("Keep battlement",[(3.42,.86),(3.42,1.23),(3.68,1.23),(3.68,1.08),(3.94,1.08),(3.94,1.23),(4.21,1.23),(4.21,1.08),(4.47,1.08),(4.47,1.22),(4.73,1.22),(4.73,.85)],SHADOW,-3)
    # subtle horizontal vignette across lower scene
    rect("Ground veil",0,-3.45,19.2,3.9,emission("Ground | blue-black",(.018,.037,.043)),-2)
    line("Horizon",[(-9.6,-1.42),(9.6,-1.42)],MIST,.012,-1)
    for x in [-8.4,-7.5,-1.8,0.5,2.1,7.4,8.2]:
        line("Grass glint",[(x,-2.7),(x+.16,-2.42),(x+.29,-2.8)],MIST,.012,-1)

def brand(x=-7.5,y=3.55):
    text("SAOMMO",x,y,.80,WHITE,FONT_TITLE)
    text("E C H O E S   O F   T H E   F A L L E N   R E A C H",x+.04,y-.36,.13,GOLD,FONT_BOLD)
    line("Brand underline",[(x,y-.54),(x+3.36,y-.54)],GOLD,.012,2)

def button(name,x,y,w=3.35,active=False):
    rect(name+" | card",x,y,w,.60,PANEL2 if active else PANEL,1,.035)
    rect(name+" | marker",x-w/2+.12,y,.035,.29,GOLD if active else MUTED,2)
    text(name,x-w/2+.30,y-.085,.235,WHITE if active else MUTED,FONT_BOLD,z=3)
    line(name+" | rule",[(x-w/2+.30,y-.205),(x+w/2-.16,y-.205)],GOLD if active else PANEL2,.008,2)

def save_render(scene, slug):
    for screen in bpy.data.screens:
        for area in screen.areas:
            if area.type == "VIEW_3D":
                area.spaces.active.region_3d.view_perspective = "CAMERA"
    scene.render.filepath=os.path.join(OUT,slug+".png")
    bpy.ops.render.render(write_still=True)

def make_start():
    s=reset_scene("01 | Start Menu"); scene_backdrop(); brand(-7.3,3.62)
    text("Ein vergessenes Reich. Ein neuer Anfang.",-7.25,2.22,.225,MUTED,FONT_BODY)
    button("Neues Spiel",-5.62,1.16,3.35,True); button("Fortsetzen",-5.62,.39,3.35); button("Einstellungen",-5.62,-.38,3.35); button("Verlassen",-5.62,-1.15,3.35)
    # central ceremonial sword emblem, deliberately graphic and non-branded
    poly("Sword | blade",[(.35,2.38),(.57,2.90),(.76,2.35),(.67,-.18),(.47,-.18)],GOLD,1)
    poly("Sword | fuller",[(.55,2.48),(.60,2.69),(.64,-.16),(.56,-.16)],WHITE,1.2)
    rect("Sword | guard",.56,-.30,1.18,.105,GOLD,1.5,.025); rect("Sword | grip",.56,-.77,.18,.86,STONE,1.4,.025)
    circle("Sword | pommel",.56,-1.28,.14,GOLD,.045,1.5)
    for i in range(5): line("Rune",[(.39+i*.085,1.7-i*.51),(.72-i*.025,1.66-i*.51)],GLOW,.013,1.1)
    text("RUINS REMEMBER",5.75,-3.0,.24,GOLD,FONT_TITLE,align="CENTER")
    text("v0.1  |  DESKTOP PROTOTYPE",-9.18,-5.08,.13,MUTED,FONT_BOLD)
    text("ENTER  AUSWÄHLEN     ESC  ZURÜCK",9.12,-5.08,.13,MUTED,FONT_BOLD,"RIGHT")
    save_render(s,"01_start_menu")
    return s

def make_hud():
    s=reset_scene("02 | Gameplay HUD"); scene_backdrop()
    # top-left character status
    rect("Status backing",-6.92,4.47,4.55,.95,INK,1,.08); circle("Portrait frame",-8.75,4.47,.36,GOLD,.026,3)
    circle("Portrait inner",-8.75,4.47,.27,MIST,.02,3)
    text("WANDERER",-8.20,4.68,.22,WHITE,FONT_BOLD); text("STUFE 04  ·  SCHWERTLEHRLING",-8.20,4.40,.13,MUTED,FONT_BODY)
    rect("HP track",-6.83,4.17,2.55,.075,PANEL2,3); rect("HP fill",-7.12,4.17,1.96,.052,RED,4)
    rect("Stamina track",-4.55,4.17,1.24,.075,PANEL2,3); rect("Stamina fill",-4.77,4.17,.78,.052,GREEN,4)
    # small quest panel
    rect("Quest panel",6.62,3.82,4.75,1.57,INK,1,.05); label("Aktuelle Aufgabe",4.55,4.37,.15,GOLD)
    text("Der Pfad zum Klingenhof",4.55,3.98,.21,WHITE,FONT_BOLD); text("Finde den alten Übungsplatz.",4.55,3.64,.16,MUTED,FONT_BODY)
    circle("Quest marker",8.48,3.82,.13,GOLD,.02,3)
    # center small reticle
    circle("Reticle",0,0,.075,WHITE,.012,4); line("Reticle H",[(-.14,0),(-.09,0)],WHITE,.012,4); line("Reticle H2",[(.09,0),(.14,0)],WHITE,.012,4)
    # bottom hotbar
    rect("Hotbar backing",0,-4.43,5.55,1.0,INK,1,.055)
    slots=[("1","Angriff"),("2","Block"),("3","Trank"),("4","Rolle"),("Q","Fähigkeit")]
    for i,(key,name) in enumerate(slots):
        x=-2.08+i*1.04; rect("Slot "+key,x,-4.40,.78,.70,PANEL2 if i==0 else PANEL,2,.035)
        text(key,x-.30,-4.17,.14,GOLD,FONT_BOLD); circle("Ability icon "+key,x,-4.44,.13,GLOW if i==0 else MUTED,.02,4)
        text(name,x,-4.90,.105,MUTED,FONT_BODY,"CENTER")
    text("E  Interagieren",-8.95,-4.57,.17,WHITE,FONT_BODY); text("I  Inventar",-8.95,-4.86,.15,MUTED,FONT_BODY)
    text("HUD-KONZEPT  ·  PROTOTYP",-9.18,-5.12,.13,MUTED,FONT_BOLD)
    save_render(s,"02_gameplay_hud"); return s

def make_inventory():
    s=reset_scene("03 | Inventory"); scene_backdrop(); rect("Modal scrim",0,0,19.2,10.8,emission("Scrim",(.006,.014,.021)),1)
    rect("Inventory panel",0,0,14.8,8.1,PANEL,2,.06)
    text("INVENTAR",-6.95,3.25,.54,WHITE,FONT_TITLE); text("AUSRÜSTUNG  ·  BEUTE  ·  MATERIALIEN",-6.92,2.86,.15,GOLD,FONT_BOLD)
    line("Header line",[(-6.95,2.62),(6.95,2.62)],GOLD,.012,4)
    # left equipment preview
    rect("Preview panel",-4.45,-.30,4.75,5.25,INK,3,.04); label("Ausrüstung",-6.42,1.80,.17,GOLD)
    # abstract mannequin / armor silhouette
    circle("Avatar head",-4.45,.95,.40,STONE,.01,4)
    poly("Avatar torso",[(-5.10,.46),(-4.8,.69),(-4.1,.69),(-3.8,.46),(-3.98,-.90),(-4.92,-.90)],STONE,4)
    poly("Avatar legs",[(-4.88,-.84),(-4.02,-.84),(-3.90,-2.20),(-4.33,-2.20),(-4.48,-1.43),(-4.65,-2.20),(-5.05,-2.20)],SHADOW,4)
    # armor sockets
    for name,x,y in [("Kopf",-5.93,1.02),("Brust",-2.96,.0),("Waffe",-5.93,-1.65),("Ring",-2.96,-1.65)]:
        rect("Equip "+name,x,y,.54,.54,PANEL2,5,.035); circle("Equip glyph "+name,x,y,.12,GOLD,.018,6); text(name,x,y-.42,.12,MUTED,FONT_BODY,"CENTER")
    text("WERT",-6.18,-2.46,.14,MUTED,FONT_BOLD); text("ANGRIFF  18     SCHUTZ  12",-6.18,-2.76,.16,WHITE,FONT_BODY)
    # right tabs and item grid
    label("Beute",-1.55,1.80,.17,GOLD); text("12 / 30",6.23,1.80,.15,MUTED,FONT_BODY,"RIGHT")
    for r in range(4):
        for c in range(5):
            x=-1.48+c*1.55; y=1.05-r*1.13; rect("Item slot",x,y,.95,.85,PANEL2 if (r,c)==(0,0) else INK,4,.035)
            if (r,c) in [(0,0),(0,1),(1,2),(2,0),(3,3)]:
                circle("Item icon",x,y,.16,GOLD if (r,c)==(0,0) else GLOW,.027,6)
                text("x"+str(2+c),x+.34,y-.28,.12,MUTED,FONT_BODY,"RIGHT")
    text("Übungsschwert",-1.95,-3.05,.17,WHITE,FONT_BOLD,z=5); text("Ein schlichtes Schwert aus dem Klingenhof.",-1.95,-3.34,.14,MUTED,FONT_BODY,z=5)
    rect("Equip button",4.95,-3.17,2.45,.52,GOLD,4,.035); text("AUSRÜSTEN",4.95,-3.23,.16,INK,FONT_BOLD,"CENTER",6)
    text("ESC  SCHLIESSEN",6.88,-3.62,.14,MUTED,FONT_BOLD,"RIGHT")
    save_render(s,"03_inventory"); return s

def make_character():
    s=reset_scene("04 | Character Creator"); scene_backdrop(); rect("Creator scrim",0,0,19.2,10.8,emission("Creator scrim",(.009,.021,.028)),1)
    text("DEIN PFAD BEGINNT",-8.05,4.12,.55,WHITE,FONT_TITLE); text("GESTALTE DEINEN WANDERER",-8.02,3.75,.15,GOLD,FONT_BOLD)
    line("Creator header",[(-8.02,3.52),(8.02,3.52)],GOLD,.013,4)
    # large character preview silhouette
    circle("Creator head",-2.0,1.92,.60,STONE,.01,4)
    poly("Creator shoulders",[(-3.02,1.2),(-2.65,1.52),(-1.35,1.52),(-.98,1.2),(-1.28,-1.1),(-2.72,-1.1)],STONE,4)
    poly("Creator legs",[(-2.69,-1.0),(-1.31,-1.0),(-1.18,-3.05),(-1.78,-3.05),(-2,-1.78),(-2.22,-3.05),(-2.83,-3.05)],SHADOW,4)
    line("Creator sword",[(-.8,1.1),(.55,-1.32)],GOLD,.065,5); line("Sword crossguard",[(-1.10,-1.0),(-.22,-1.48)],GOLD,.05,5)
    text("VORSCHAU",-2, -3.60,.14,MUTED,FONT_BOLD,"CENTER")
    # right-side design controls
    rect("Creation panel",4.20,-.05,6.5,6.6,PANEL,3,.04)
    label("Erscheinung",1.55,2.72,.16,GOLD)
    text("Name",1.58,2.20,.16,MUTED,FONT_BODY); rect("Name field",4.65,2.20,5.30,.48,INK,4,.025); text("Neuer Wanderer",2.15,2.13,.18,WHITE,FONT_BODY)
    for i,(title,opts,sel) in enumerate([("KÖRPER",["Schlank","Athletisch","Kräftig"],1),("HAAR",["Kurz","Lang","Gebunden"],0),("FARBE",["Aschgrau","Kastanie","Schwarz"],2)]):
        y=1.31-i*1.27; label(title,1.58,y,.14,MUTED)
        for j,opt in enumerate(opts):
            x=3.02+j*2.05; rect(title+opt,x,y,.48,.40,GOLD if j==sel else PANEL2,4,.025); text(opt,x+.34,y-.37,.13,WHITE if j==sel else MUTED,FONT_BODY,"CENTER")
    text("Die Auswahl ist später jederzeit änderbar.",1.58,-2.86,.14,MUTED,FONT_BODY)
    rect("Begin button",5.28,-3.48,3.15,.62,GOLD,4,.035); text("WELT BETRETEN",5.28,-3.55,.18,INK,FONT_BOLD,"CENTER",6)
    text("ZURÜCK",-8.0,-4.72,.14,MUTED,FONT_BOLD); text("CHARAKTERERSTELLUNG  ·  KONZEPT",8.0,-4.72,.13,MUTED,FONT_BOLD,"RIGHT")
    save_render(s,"04_character_creator"); return s

def make_death():
    s=reset_scene("05 | Revive Screen"); scene_backdrop(); rect("Death veil",0,0,19.2,10.8,emission("Death veil",(.008,.016,.023)),0)
    circle("Revive emblem outer",0,1.12,1.02,GOLD,.018,2); circle("Revive emblem inner",0,1.12,.77,MIST,.014,2)
    line("Emblem sword blade",[(0,1.67),(0,.47)],GOLD,.045,3); line("Emblem guard",[(-.37,.84),(.37,.84)],GOLD,.04,3)
    text("DU BIST GEFALLEN",0,-.33,.61,WHITE,FONT_TITLE,"CENTER")
    text("Der Kampf ist noch nicht vorbei.",0,-.79,.22,MUTED,FONT_BODY,"CENTER")
    rect("Revive button",0,-1.75,4.25,.68,GOLD,2,.04); text("WIEDERBELEBEN",0,-1.83,.21,INK,FONT_BOLD,"CENTER",4)
    text("Fortsetzung am letzten Rastplatz",0,-2.38,.16,MUTED,FONT_BODY,"CENTER")
    text("TOD / RESPAWN  ·  KONZEPT, REGELN NOCH OFFEN",0,-4.62,.14,GOLD,FONT_BOLD,"CENTER")
    save_render(s,"05_revive_screen"); return s

designs={"start":(make_start,"01_start_menu"), "hud":(make_hud,"02_gameplay_hud"), "inventory":(make_inventory,"03_inventory"), "character":(make_character,"04_character_creator"), "revive":(make_death,"05_revive_screen")}
args=sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
keys=args if args else list(designs.keys())
for key in keys:
    if key not in designs: raise ValueError("Unknown design: "+key)
    creator, slug=designs[key]
    creator()
    blend=os.path.join(ROOT,"SAOMMO_"+slug+".blend")
    bpy.ops.wm.save_as_mainfile(filepath=blend)
    print("Saved",blend)
