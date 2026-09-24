"""Headless master material + instances for the three CC0 texture sets (Band 5 §9).

Run: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<this>"

Creates /Game/Materials/M_CC0Surface with four TextureSampleParameter2D nodes
(Color / Normal / Roughness / AO; UV0 is used implicitly by the sample nodes)
and one MI_<set> instance per imported ambientCG CC0 set (Ground037,
Rock063, Planks009), each wired to its own T_<set>_* textures. Band 5 §9:
organized, reusable materials instead of unique per-asset materials.

Idempotent: existing assets are reused and only (re)wired. Every property
lookup is attempted under several name spellings (UE python name mangling
differs between engine versions) and the attempts are logged, so one run
diagnoses the whole wiring. Writes <Project>/Saved/MaterialBuildResult.txt.
"""
import traceback

results = []

MASTER_PATH = "/Game/Materials/M_CC0Surface"
TEX_DIR = "/Game/Materials"
SETS = ("Ground037", "Rock063", "Planks009")
# suffix -> texture-sample parameter name on the master material
WIRING = (
    ("Color", "Color"),
    ("Normal", "Normal"),
    ("Roughness", "Roughness"),
    ("AO", "AO"),
)


def log(msg):
    results.append(msg)
    print("[MATLIB] " + msg)


def try_set(obj, names, value):
    """Set a property trying several spellings; returns the one that worked."""
    last = None
    for n in names:
        try:
            obj.set_editor_property(n, value)
            return "set_editor_property(%s)" % n
        except Exception as e:  # noqa: BLE001 - probe loop by design
            last = e
    for n in names:
        try:
            setattr(obj, n, value)
            return "setattr(%s)" % n
        except Exception as e:  # noqa: BLE001
            last = e
    log("try_set failed for %s: %s" % (names, last))
    return None


def try_get(obj, names):
    for n in names:
        try:
            value = obj.get_editor_property(n)
            return value, "get_editor_property(%s)" % n
        except Exception:
            pass
    return None, None


def main():
    import unreal
    result_path = unreal.Paths.project_saved_dir() + "MaterialBuildResult.txt"

    try:
        reg = unreal.AssetRegistryHelpers.get_asset_registry()
        reg.search_all_assets(True)
        log("asset rescan done")
    except Exception:
        log("rescan skipped\n%s" % traceback.format_exc())

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    # --- Master material -------------------------------------------------
    master = None
    if unreal.EditorAssetLibrary.does_asset_exist(MASTER_PATH):
        master = unreal.EditorAssetLibrary.load_asset(MASTER_PATH)
        log("master reused: %s" % MASTER_PATH)
    else:
        master = asset_tools.create_asset(
            "M_CC0Surface", TEX_DIR, unreal.Material, unreal.MaterialFactoryNew()
        )
        log("master created: %s -> %s" % (MASTER_PATH, master is not None))
    if master is None:
        log("FAIL: no master material")
        return False

    existing = unreal.MaterialEditingLibrary.get_material_expressions(master)
    graph_ok = True
    if len(existing) >= len(WIRING):
        log("master already has %d expressions, skipping graph build" % len(existing))
    else:
        # Default textures on the master give a sane preview; every instance
        # overrides all four parameters anyway.
        default_set = SETS[0]
        prop_by_suffix = {
            "Color": unreal.MaterialProperty.MP_BASE_COLOR,
            "Normal": unreal.MaterialProperty.MP_NORMAL,
            "Roughness": unreal.MaterialProperty.MP_ROUGHNESS,
            "AO": unreal.MaterialProperty.MP_AMBIENT_OCCLUSION,
        }
        output_by_suffix = {
            "Color": "RGB",
            "Normal": "RGB",
            "Roughness": "R",
            "AO": "R",
        }
        y = -600
        for suffix, param in WIRING:
            tex = unreal.EditorAssetLibrary.load_asset(
                "%s/T_%s_%s" % (TEX_DIR, default_set, suffix)
            )
            expr = unreal.MaterialEditingLibrary.create_material_expression(
                master, unreal.MaterialExpressionTextureSampleParameter2D, -900, y
            )
            expr.set_editor_property("parameter_name", param)
            if tex is not None:
                expr.set_editor_property("texture", tex)
            wired_ok = unreal.MaterialEditingLibrary.connect_material_property(
                expr, output_by_suffix[suffix], prop_by_suffix[suffix]
            )
            graph_ok = graph_ok and bool(wired_ok)
            log("expression %s -> %s (tex=%s, wired=%s)" % (
                param, prop_by_suffix[suffix], tex is not None, wired_ok))
            y += 300
        log("graph built: %d expressions" % len(
            unreal.MaterialEditingLibrary.get_material_expressions(master)))

    saved = unreal.EditorAssetLibrary.save_asset(MASTER_PATH)
    log("save master -> %s" % saved)

    # --- Instances -------------------------------------------------------
    all_ok = bool(saved)
    for set_name in SETS:
        mi_name = "MI_" + set_name
        mi_path = "%s/%s" % (TEX_DIR, mi_name)
        mi = None
        if unreal.EditorAssetLibrary.does_asset_exist(mi_path):
            mi = unreal.EditorAssetLibrary.load_asset(mi_path)
            log("instance reused: %s" % mi_path)
        else:
            factory = unreal.MaterialInstanceConstantFactoryNew()
            log("factory parent-candidates: %s" % [n for n in dir(factory)
                                                   if "parent" in n.lower()])
            how = try_set(factory, ("initial_parent", "InitialParent"), master)
            log("factory parent set via: %s" % how)
            mi = asset_tools.create_asset(
                mi_name, TEX_DIR, unreal.MaterialInstanceConstant, factory
            )
            log("instance created: %s -> %s" % (mi_path, mi is not None))
            if mi is not None:
                parent, how_get = try_get(mi, ("parent", "Parent"))
                log("instance parent after create: %s via %s" % (
                    parent == master or parent == str(master) or
                    (parent is not None and getattr(parent, "get_path_name",
                                                    lambda: "")() ==
                     master.get_path_name()), how_get))
                if how is None and parent is None:
                    if hasattr(unreal.MaterialEditingLibrary,
                               "set_material_instance_parent"):
                        unreal.MaterialEditingLibrary.set_material_instance_parent(
                            mi, master)
                        log("parent set via MaterialEditingLibrary")
                    else:
                        how2 = try_set(mi, ("parent", "Parent"), master)
                        log("parent set on instance via: %s" % how2)
        if mi is None:
            all_ok = False
            log("FAIL: no instance for %s" % set_name)
            continue
        wired = True
        for suffix, param in WIRING:
            tex = unreal.EditorAssetLibrary.load_asset(
                "%s/T_%s_%s" % (TEX_DIR, set_name, suffix)
            )
            if tex is None:
                wired = False
                log("FAIL: texture missing T_%s_%s" % (set_name, suffix))
                continue
            try:
                unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
                    mi, param, tex
                )
            except Exception:
                wired = False
                log("FAIL: texture param %s/%s\n%s" % (
                    mi_name, param, traceback.format_exc()))
        mi_saved = unreal.EditorAssetLibrary.save_asset(mi_path)
        log("save %s -> %s (wired=%s)" % (mi_name, mi_saved, wired))
        all_ok = all_ok and wired and bool(mi_saved)

    # --- Verification ----------------------------------------------------
    tex_ok = True
    for set_name in SETS:
        mi = unreal.EditorAssetLibrary.load_asset("%s/MI_%s" % (TEX_DIR, set_name))
        if mi is None:
            tex_ok = False
            log("FAIL verify: MI_%s missing" % set_name)
            continue
        for suffix, param in WIRING:
            exp = unreal.EditorAssetLibrary.load_asset(
                "%s/T_%s_%s" % (TEX_DIR, set_name, suffix))
            got = None
            getter = getattr(
                unreal.MaterialEditingLibrary,
                "get_material_instance_texture_parameter_value", None)
            if getter is not None:
                try:
                    got = getter(mi, param)
                except Exception:
                    log("getter exc %s/%s\n%s" % (set_name, param,
                                                  traceback.format_exc()))
            same = (got is not None and exp is not None
                    and got.get_path_name() == exp.get_path_name())
            if not same:
                tex_ok = False
                log("FAIL verify %s.%s -> %s (want %s)" % (
                    set_name, param,
                    got.get_path_name() if got else None,
                    exp.get_path_name() if exp else None))
    log("verify textures read back: %s" % tex_ok)

    present = [
        p for p in [MASTER_PATH] + ["%s/MI_%s" % (TEX_DIR, s) for s in SETS]
        if unreal.EditorAssetLibrary.does_asset_exist(p)
    ]
    n_expr = len(unreal.MaterialEditingLibrary.get_material_expressions(
        unreal.EditorAssetLibrary.load_asset(MASTER_PATH)))
    ok = (all_ok and graph_ok and tex_ok
          and len(present) == 1 + len(SETS) and n_expr >= len(WIRING))
    log("verify: assets=%s expr=%d -> %s" % (len(present), n_expr, ok))

    with open(result_path, "w") as f:
        f.write("ok=%s\n%s\n" % (ok, "\n".join(results)))
    log("result written: %s" % result_path)
    print("RESULT: ok=%s" % ok)
    return ok


if __name__ == "__main__":
    try:
        main()
    except Exception:
        print("[MATLIB] UNHANDLED\n%s" % traceback.format_exc())
