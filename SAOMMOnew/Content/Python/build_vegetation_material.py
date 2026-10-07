"""Headless Vegetations-Materialien fuer CC0-Baum + Stumpf (Backlog #41d, Band 5 §9).

Run: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<this>"

M_CC0Surface (build_master_materials.py) ist opaque und kennt keinen
OpacityMask-Eingang, die Blaetter brauchen aber BLEND_Masked. Deshalb legt
dieses Skript einen zweiten Master an:

    /Game/Materials/M_CC0Foliage   Blend Mode MASKED, Shading Model
                                   Default Lit, TextureSampleParameter2D
                                   Color / Normal / Roughness / OpacityMask
                                   (OpacityMask = T_IslandTree02_Leaves_Alpha)

Trunk, Branches und Stumpf bleiben - wie in #41d vorgesehen - auf
M_CC0Surface, weil sie vollstaendig opaque sind. Vier Instanzen (Band 5 §9:
MI-Wiederverwendung statt Unique-Materialien):

    MI_IslandTree_Trunk     -> M_CC0Surface  T_IslandTree02_*
    MI_IslandTree_Branches  -> M_CC0Surface  T_IslandTree02_Branches_*
    MI_IslandTree_Leaves    -> M_CC0Foliage  T_IslandTree02_Leaves_* (+Alpha)
    MI_TreeStump            -> M_CC0Surface  T_TreeStump01_*

Anschliessend werden die Slots beider StaticMeshes belegt und gespeichert:

    SM_IslandTree_Grauwaldrand  Slots 0..2 - Zuordnung aus den
                                FBX-Materialnamen (leaves/branch/...),
                                Fallback = dokumentierte Reihenfolge
                                trunk/branches/leaves
    SM_TreeStump_Grauwaldrand   Slot 0 -> MI_TreeStump

AO-Hinweis: M_CC0Surface besitzt einen AO-Parameter, dessen Master-Default
T_Ground037_AO ist - unsere beiden Sets haben keine AO-Karte. Der Wert wird
auf eine weisse Engine-Textur gesetzt (AO = 1 = neutral); falls die nicht
fundbar ist, bleibt der Ground-Default stehen und wird als WARNUNG geloggt.

Idempotent: vorhandene Assets werden wiederverwendet und nur (re)verdrahtet,
save_asset(<pfad>) speichert explizit. Schreibt
<Saved>/VegetationMaterialBuildResult.txt und RESULT: ok=<bool>.
"""
import traceback

results = []

TEX_DIR = "/Game/Materials"
SURFACE_MASTER = "/Game/Materials/M_CC0Surface"
FOLIAGE_MASTER = "/Game/Materials/M_CC0Foliage"

# weisse Engine-Textur als neutraler AO-Ersatz (AO=1), mehrere Kandidaten
WHITE_CANDIDATES = (
    "/Engine/EngineResources/WhiteTexture",
    "/Engine/EngineMaterials/DefaultWhiteTexture",
    "/Engine/EngineTextures/WhiteTexture",
)

# Suffix -> (Parametername auf dem Master, MaterialProperty, Output-Pin)
# Reihenfolge wie build_metal_material.py (MP_BASE_COLOR etc. sind die
# einzigen in 5.8 akzeptierten Schreibweisen).
SURFACE_WIRING = (
    ("Color", "Color", "MP_BASE_COLOR", "RGB"),
    ("Normal", "Normal", "MP_NORMAL", "RGB"),
    ("Roughness", "Roughness", "MP_ROUGHNESS", "R"),
)
# Der Alpha-Zweig #41d: Maske in MP_OPACITY_MASK, Pin R (Eingang ist float).
FOLIAGE_WIRING = SURFACE_WIRING + (
    ("Alpha", "OpacityMask", "MP_OPACITY_MASK", "R"),
)

# MI-Name -> (Master-Pfad, T_ Praefix)  [T_<Praefix>_<Suffix>]
INSTANCES = (
    ("MI_IslandTree_Trunk", SURFACE_MASTER, "T_IslandTree02"),
    ("MI_IslandTree_Branches", SURFACE_MASTER, "T_IslandTree02_Branches"),
    ("MI_IslandTree_Leaves", FOLIAGE_MASTER, "T_IslandTree02_Leaves"),
    ("MI_TreeStump", SURFACE_MASTER, "T_TreeStump01"),
)

# Slotnamen der Baeume -> MI-Name (lowercase Substring-Regeln, erste Treffer
# gewinnt; "leaves"/"branch" muessen vor "island_tree_02" pruefen, weil der
# Branches-Name den Basissatz enthaelt).
TREE_RULES = (
    ("leaves", "MI_IslandTree_Leaves"),
    ("branch", "MI_IslandTree_Branches"),
    ("trunk", "MI_IslandTree_Trunk"),
    ("island_tree_02", "MI_IslandTree_Trunk"),
)
# Fallback, wenn die FBX-Materialnamen generisch sind: die in #41c
# dokumentierte Slotreihenfolge trunk/branches/leaves.
TREE_FALLBACK = (
    "MI_IslandTree_Trunk",
    "MI_IslandTree_Branches",
    "MI_IslandTree_Leaves",
)

MESH_SLOTS = (
    # (Mesh-Pfad, Regeln, Fallback-Reihenfolge)
    ("/Game/Props/SM_IslandTree_Grauwaldrand", TREE_RULES, TREE_FALLBACK),
    ("/Game/Props/SM_TreeStump_Grauwaldrand", (), ("MI_TreeStump",)),
)


def log(msg):
    results.append(msg)
    print("[VEGMAT] " + msg)


def try_set(obj, names, value):
    """Property mit einer von mehreren Schreibweisen setzen (5.8-Mangling)."""
    last = None
    for n in names:
        try:
            obj.set_editor_property(n, value)
            return "set_editor_property(%s)" % n
        except Exception as e:  # noqa: BLE001 - Probe-Schleife
            last = e
    for n in names:
        try:
            setattr(obj, n, value)
            return "setattr(%s)" % n
        except Exception as e:  # noqa: BLE001
            last = e
    log("try_set fehlgeschlagen fuer %s: %s" % (names, last))
    return None


def try_get(obj, names):
    for n in names:
        try:
            return obj.get_editor_property(n), "get_editor_property(%s)" % n
        except Exception:  # noqa: BLE001
            pass
    return None, None


def load(path):
    import unreal
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.EditorAssetLibrary.load_asset(path)
    return None


def wiring_for(master_path):
    return FOLIAGE_WIRING if master_path == FOLIAGE_MASTER else SURFACE_WIRING


# ---------------------------------------------------------------- Master ---
def ensure_foliage_master(asset_tools):
    """M_CC0Foliage anlegen/benutzen, MASKED erzwingen, Graph bauen."""
    import unreal
    master = load(FOLIAGE_MASTER)
    if master is None:
        master = asset_tools.create_asset(
            "M_CC0Foliage", TEX_DIR, unreal.Material, unreal.MaterialFactoryNew()
        )
        log("Master erstellt: %s -> %s" % (FOLIAGE_MASTER, master is not None))
    else:
        log("Master wiederverwendet: %s" % FOLIAGE_MASTER)
    if master is None:
        return None

    # Blend Mode jedes Mal erzwingen (self-healing wie die Settings im
    # Import-Skript) und zuruecklesen, damit der Beweis im Result steht.
    try:
        master.set_editor_property("blend_mode", unreal.BlendMode.BLEND_MASKED)
    except Exception:  # noqa: BLE001
        log("blend_mode setzen fehlgeschlagen\n%s" % traceback.format_exc())
    blend = master.get_editor_property("blend_mode")
    log("M_CC0Foliage blend_mode = %s (soll %s)" % (blend, "BLEND_MASKED"))
    try:
        shading = master.get_editor_property("shading_model")
        log("M_CC0Foliage shading_model = %s" % shading)
    except Exception:  # noqa: BLE001
        log("shading_model nicht lesbar (Default Lit ist Standby)")

    existing = unreal.MaterialEditingLibrary.get_material_expressions(master)
    if len(existing) >= len(FOLIAGE_WIRING):
        log("Master hat bereits %d Expressions, Graphbau uebersprungen"
            % len(existing))
    else:
        # Default-Texturen = Blatt-Satz, damit der Master allein sinnvoll
        # vorschaut; die MI verdrahtet ohnehin alles selbst.
        default_prefix = "T_IslandTree02_Leaves"
        y = -600
        for suffix, param, prop_name, pin in FOLIAGE_WIRING:
            tex = load("%s/%s_%s" % (TEX_DIR, default_prefix, suffix))
            expr = unreal.MaterialEditingLibrary.create_material_expression(
                master, unreal.MaterialExpressionTextureSampleParameter2D,
                -900, y)
            expr.set_editor_property("parameter_name", param)
            if tex is not None:
                expr.set_editor_property("texture", tex)
            wired = unreal.MaterialEditingLibrary.connect_material_property(
                expr, pin, getattr(unreal.MaterialProperty, prop_name))
            log("Graph %s -> %s (tex=%s, wired=%s)" % (
                param, prop_name, tex is not None, bool(wired)))
            y += 300
        log("Graph gebaut: %d Expressions" % len(
            unreal.MaterialEditingLibrary.get_material_expressions(master)))

    saved = unreal.EditorAssetLibrary.save_asset(FOLIAGE_MASTER)
    log("save %s -> %s" % (FOLIAGE_MASTER, saved))
    return master if saved else None


def make_instance(asset_tools, name, parent):
    """MI anlegen (oder wiederverbenutzen) und auf <parent> setzen."""
    import unreal
    path = "%s/%s" % (TEX_DIR, name)
    mi = load(path)
    if mi is not None:
        log("Instanz wiederverwendet: %s" % path)
    else:
        factory = unreal.MaterialInstanceConstantFactoryNew()
        how = try_set(factory, ("initial_parent", "InitialParent"), parent)
        mi = asset_tools.create_asset(
            name, TEX_DIR, unreal.MaterialInstanceConstant, factory)
        log("Instanz erstellt: %s -> %s (parent via %s)" % (
            path, mi is not None, how))
        if mi is not None and how is None:
            p, how_get = try_get(mi, ("parent", "Parent"))
            log("parent nach create: %s via %s" % (p == parent, how_get))
            if p is None:
                if hasattr(unreal.MaterialEditingLibrary,
                           "set_material_instance_parent"):
                    unreal.MaterialEditingLibrary.set_material_instance_parent(
                        mi, parent)
                    log("parent via MaterialEditingLibrary gesetzt")
                else:
                    log("parent setzen: %s" % try_set(
                        mi, ("parent", "Parent"), parent))
    if mi is None:
        return None
    # Sicherstellen, dass der Parent stimmt (falscher Parent aus einem
    # frueheren Lauf wuerde stille Fehlverdrahtung erzeugen).
    p, _ = try_get(mi, ("parent", "Parent"))
    if p is not None and getattr(p, "get_path_name", lambda: "")() != \
            parent.get_path_name():
        log("WARNUNG: parent war %s, setze neu" % getattr(
            p, "get_path_name", lambda: p)())
        if hasattr(unreal.MaterialEditingLibrary,
                   "set_material_instance_parent"):
            unreal.MaterialEditingLibrary.set_material_instance_parent(
                mi, parent)
        else:
            try_set(mi, ("parent", "Parent"), parent)
    return mi


def wire_instance(mi, name, prefix, wiring, white_tex, with_ao):
    """Texturen-Parameter der MI setzen; returns True wenn alles saß.

    with_ao gilt nur fuer Instanzen auf M_CC0Surface (dessen Master-AO-
    Default T_Ground037_AO ist, weil unsere Sets keine AO-Karte haben);
    M_CC0Foliage hat gar keinen AO-Eingang, dort wird nichts gesetzt.
    """
    import unreal
    ok = True
    for suffix, param, _prop, _pin in wiring:
        tex = load("%s/%s_%s" % (TEX_DIR, prefix, suffix))
        if tex is None:
            ok = False
            log("FAIL: Textur fehlt %s_%s (Parameter %s)" % (
                prefix, suffix, param))
            continue
        try:
            unreal.MaterialEditingLibrary\
                .set_material_instance_texture_parameter_value(mi, param, tex)
        except Exception:  # noqa: BLE001
            ok = False
            log("FAIL: Parameter %s/%s\n%s" % (
                name, param, traceback.format_exc()))
    # AO neutralisieren, wenn der Master ueberhaupt einen AO-Parameter hat
    # (M_CC0Surface ja, M_CC0Foliage nein).
    if with_ao:
        if white_tex is not None:
            try:
                unreal.MaterialEditingLibrary\
                    .set_material_instance_texture_parameter_value(
                        mi, "AO", white_tex)
                log("AO neutral: %s bei %s" % (
                    white_tex.get_path_name(), name))
            except Exception:  # noqa: BLE001
                log("WARNUNG: AO-Override fehlgeschlagen fuer %s\n%s" % (
                    name, traceback.format_exc()))
        else:
            log("WARNUNG: keine weisse Textur gefunden - %s behaelt den "
                "Master-AO-Default (T_Ground037_AO)" % name)
    return ok


# ----------------------------------------------------------------- Slots ---
def slot_state(mesh):
    """[(Slotname, Materialpfad|None)] aus static_materials (authoritativ)."""
    try:
        sms = list(mesh.get_editor_property("static_materials"))
    except Exception as exc:  # noqa: BLE001
        log("static_materials nicht lesbar: %s" % exc)
        return []
    out = []
    for sm in sms:
        try:
            name = str(sm.material_slot_name)
        except Exception:  # noqa: BLE001
            name = "?"
        try:
            mat = sm.material_interface
            out.append((name, mat.get_path_name() if mat else None))
        except Exception:  # noqa: BLE001
            out.append((name, "?"))
    return out


def assign_slot(mesh, slot, mi, label):
    """MI auf Slot <slot> setzen, Readback ueber static_materials."""
    import unreal
    # Route 1: UStaticMesh::SetMaterial (in 5.8 als set_material exposed).
    setter = getattr(mesh, "set_material", None)
    if not callable(setter):
        log("%s set_material nicht exposed - direkter Property-Zugriff" % label)
    else:
        try:
            setter(slot, mi)
            log("%s set_material(%d) ausgefuehrt" % (label, slot))
        except Exception as exc:  # noqa: BLE001
            log("%s set_material(%d) fehlgeschlagen: %s" % (label, slot, exc))
    state = slot_state(mesh)
    want = mi.get_path_name()
    if slot < len(state) and state[slot][1] == want:
        return True

    # Route 2: static_materials-Property direkt (Struktur zurueckschreiben).
    log("%s Route 2: static_materials[%d] setzen" % (label, slot))
    try:
        sms = list(mesh.get_editor_property("static_materials"))
        entry = sms[slot]
        try:
            entry.material_interface = mi
        except Exception:  # noqa: BLE001
            entry.set_editor_property("material_interface", mi)
        sms[slot] = entry
        mesh.set_editor_property("static_materials", sms)
    except Exception:  # noqa: BLE001
        log("%s Route 2 fehlgeschlagen\n%s" % (label, traceback.format_exc()))
    try:
        if hasattr(mesh, "post_edit_change"):
            mesh.post_edit_change()
    except Exception as exc:  # noqa: BLE001
        log("%s post_edit_change: %s" % (label, exc))
    state = slot_state(mesh)
    return slot < len(state) and state[slot][1] == want


def mi_for_slot(rules, fallback, index, name):
    """(MI-Name, Begruendung) fuer einen Slot - Name schlaegt Reihenfolge."""
    lowered = (name or "").lower()
    for key, mi_name in rules:
        if key in lowered:
            return mi_name, "Slotname '%s' enthaelt '%s'" % (name, key)
    if index < len(fallback):
        return fallback[index], "Fallback-Reihenfolge (Slot %d)" % index
    return None, "nicht zuordenbar (Slot %d, Name '%s')" % (index, name)


# ------------------------------------------------------------------ Main ---
def main():
    import unreal
    result_path = unreal.Paths.project_saved_dir() + \
        "VegetationMaterialBuildResult.txt"
    all_ok = True
    try:
        reg = unreal.AssetRegistryHelpers.get_asset_registry()
        reg.search_all_assets(True)
        log("Asset-Rescan erledigt")
    except Exception:  # noqa: BLE001
        log("Rescan uebersprungen\n%s" % traceback.format_exc())

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    surface = load(SURFACE_MASTER)
    if surface is None:
        log("FAIL: %s fehlt - build_master_materials.py zuerst laufen"
            % SURFACE_MASTER)
        with open(result_path, "w") as f:
            f.write("ok=False\n%s\n" % "\n".join(results))
        print("RESULT: ok=False")
        return False
    log("Master wiederverwendet: %s" % SURFACE_MASTER)

    foliage = ensure_foliage_master(asset_tools)
    all_ok = all_ok and foliage is not None

    white_tex = None
    for cand in WHITE_CANDIDATES:
        white_tex = load(cand)
        if white_tex is not None:
            log("weisse AO-Textur: %s" % cand)
            break
    if white_tex is None:
        log("WARNUNG: keine weisse Engine-Textur gefunden (%s)"
            % ", ".join(WHITE_CANDIDATES))

    # --- Instanzen -------------------------------------------------------
    instances = {}
    for name, parent_path, prefix in INSTANCES:
        parent = load(parent_path)
        if parent is None:
            all_ok = False
            log("FAIL: Master fehlt %s" % parent_path)
            continue
        mi = make_instance(asset_tools, name, parent)
        if mi is None:
            all_ok = False
            log("FAIL: keine Instanz %s" % name)
            continue
        instances[name] = mi
        wired = wire_instance(mi, name, prefix, wiring_for(parent_path),
                              white_tex, parent_path == SURFACE_MASTER)
        path = "%s/%s" % (TEX_DIR, name)
        saved = unreal.EditorAssetLibrary.save_asset(path)
        log("save %s -> %s (wired=%s)" % (path, saved, wired))
        all_ok = all_ok and wired and bool(saved)

    # --- Textur-Readback -------------------------------------------------
    getter = getattr(
        unreal.MaterialEditingLibrary,
        "get_material_instance_texture_parameter_value", None)
    if getter is None:
        all_ok = False
        log("FAIL: kein getter fuer Textur-Parameter")
    else:
        for name, parent_path, prefix in INSTANCES:
            mi = instances.get(name)
            if mi is None:
                continue
            for suffix, param, _prop, _pin in wiring_for(parent_path):
                want = load("%s/%s_%s" % (TEX_DIR, prefix, suffix))
                try:
                    got = getter(mi, param)
                except Exception:  # noqa: BLE001
                    got = None
                same = (want is not None and got is not None and
                        got.get_path_name() == want.get_path_name())
                all_ok = all_ok and same
                log("Readback %s.%s -> %s (soll %s)" % (
                    name, param,
                    got.get_path_name() if got else None,
                    want.get_path_name() if want else None))

    # --- AO-Readback (nur M_CC0Surface-Instanzen, Warnung ohne Gating) ----
    if getter is not None and white_tex is not None:
        for name, parent_path, _prefix in INSTANCES:
            if parent_path != SURFACE_MASTER or name not in instances:
                continue
            try:
                got_ao = getter(instances[name], "AO")
            except Exception:  # noqa: BLE001
                got_ao = None
            neutral = (got_ao is not None and
                       got_ao.get_path_name() == white_tex.get_path_name())
            log("AO-Readback %s -> %s (neutral=%s)" % (
                name,
                got_ao.get_path_name() if got_ao else None, neutral))
            if not neutral:
                log("WARNUNG: %s nutzt weiter den AO-Master-Default "
                    "(T_Ground037_AO)" % name)

    # --- Slots belegen ---------------------------------------------------
    for mesh_path, rules, fallback in MESH_SLOTS:
        mesh = load(mesh_path)
        if mesh is None:
            all_ok = False
            log("FAIL: Mesh fehlt %s" % mesh_path)
            continue
        before = slot_state(mesh)
        log("%s Slots vorher: %s" % (mesh_path, before))
        for index, (slot_name, old_mat) in enumerate(before):
            want_name, reason = mi_for_slot(rules, fallback, index,
                                            slot_name)
            mi = instances.get(want_name)
            if mi is None:
                all_ok = False
                log("FAIL: %s Slot %d -> %s fehlt (%s)" % (
                    mesh_path, index, want_name, reason))
                continue
            ok_here = assign_slot(mesh, index, mi, "%s[%d]" % (
                mesh_path.split("/")[-1], index))
            all_ok = all_ok and ok_here
            log("%s Slot %d (%s, vorher %s) -> %s via %s (ok=%s)" % (
                mesh_path, index, slot_name, old_mat, want_name, reason,
                ok_here))
        saved = unreal.EditorAssetLibrary.save_asset(mesh_path)
        log("save %s -> %s" % (mesh_path, saved))
        all_ok = all_ok and bool(saved)
        log("%s Slots nachher: %s" % (mesh_path, slot_state(mesh)))

    # --- Verifikation ----------------------------------------------------
    blend = foliage.get_editor_property("blend_mode") if foliage else None
    masked = blend == unreal.BlendMode.BLEND_MASKED
    all_ok = all_ok and masked
    n_expr = len(unreal.MaterialEditingLibrary.get_material_expressions(
        foliage)) if foliage else 0
    all_ok = all_ok and n_expr >= len(FOLIAGE_WIRING)

    slots_ok = True
    for mesh_path, _rules, _fallback in MESH_SLOTS:
        mesh = load(mesh_path)
        state = slot_state(mesh) if mesh else []
        if not state or any(p is None for _n, p in state):
            slots_ok = False
        for _n, p in state:
            if p and "/MI_" not in p:
                slots_ok = False
    all_ok = all_ok and slots_ok

    ok = all_ok
    log("verify: blend=%s expr=%d slots_ok=%s -> %s" % (
        str(blend), n_expr, slots_ok, ok))
    with open(result_path, "w") as f:
        f.write("ok=%s\n%s\n" % (ok, "\n".join(results)))
    log("Result geschrieben: %s" % result_path)
    print("RESULT: ok=%s" % ok)
    return ok


if __name__ == "__main__":
    try:
        main()
    except Exception:  # noqa: BLE001
        exc_text = traceback.format_exc()
        print("[VEGMAT] UNHANDLED\n%s" % exc_text)
        # Result trotz Abbruch schreiben, damit kein alter Stand luegt.
        try:
            import unreal
            with open(unreal.Paths.project_saved_dir() +
                      "VegetationMaterialBuildResult.txt", "w") as f:
                f.write("ok=False\n%s\nUNHANDLED:\n%s" % (
                    "\n".join(results), exc_text))
        except Exception:  # noqa: BLE001
            pass
