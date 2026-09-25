"""Headless metal master material + practice-blade instance (backlog #20, Band 5 §9).

Run: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<this>"

Creates /Game/Materials/M_CC0Metal with four TextureSampleParameter2D nodes
(Color / Normal / Roughness / Metalness) wired to BaseColor, Normal,
Roughness and Metallic - the ambientCG metal packs ship a Metalness map and
no AmbientOcclusion, which is why the wiring differs from M_CC0Surface
(build_master_materials.py). One instance /Game/Materials/MI_PracticeBlade
(parent = M_CC0Metal) carries the scratched-steel set Metal038
(CC0, see ASSET-LICENSES.csv) and is assigned to PracticeBlade_01-03 by
dress_arena.py.

Same idempotent pattern as build_master_materials.py: existing assets are
reused and only (re)wired, property lookups try several name spellings and
log every attempt. Writes <Project>/Saved/MetalBuildResult.txt and prints
RESULT: ok=<bool>.
"""
import traceback

results = []

MASTER_PATH = "/Game/Materials/M_CC0Metal"
MI_PATH = "/Game/Materials/MI_PracticeBlade"
MI_NAME = "MI_PracticeBlade"
TEX_DIR = "/Game/Materials"
SET = "Metal038"
# suffix -> (texture-sample parameter name, material property, output pin)
# Python-binding names are UPPER_SNAKE with the correct spelling, i.e.
# MP_METALLIC (two L) - the C++ typo MP_METALIC does not exist there and
# mixed-case MP_BaseColor from SceneTypes.h is rejected as well (probed 2026-09-25).
WIRING = (
    ("Color", "Color", "MP_BASE_COLOR", "RGB"),
    ("Normal", "Normal", "MP_NORMAL", "RGB"),
    ("Roughness", "Roughness", "MP_ROUGHNESS", "R"),
    ("Metalness", "Metalness", "MP_METALLIC", "R"),
)


def log(msg):
    results.append(msg)
    print("[METAL] " + msg)


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
        except Exception as e:  # noqa: BLE001 - probe loop by design
            last = e
    log("try_set failed for %s: %s" % (names, last))
    return None


def try_get(obj, names):
    for n in names:
        try:
            return obj.get_editor_property(n), "get_editor_property(%s)" % n
        except Exception:  # noqa: BLE001 - probe loop by design
            pass
    return None, None


def main():
    import unreal
    result_path = unreal.Paths.project_saved_dir() + "MetalBuildResult.txt"

    try:
        reg = unreal.AssetRegistryHelpers.get_asset_registry()
        reg.search_all_assets(True)
        log("asset rescan done")
    except Exception:
        log("rescan skipped\n%s" % traceback.format_exc())

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    # --- Master material -------------------------------------------------
    if unreal.EditorAssetLibrary.does_asset_exist(MASTER_PATH):
        master = unreal.EditorAssetLibrary.load_asset(MASTER_PATH)
        log("master reused: %s" % MASTER_PATH)
    else:
        master = asset_tools.create_asset(
            "M_CC0Metal", TEX_DIR, unreal.Material, unreal.MaterialFactoryNew()
        )
        log("master created: %s -> %s" % (MASTER_PATH, master is not None))
    if master is None:
        log("FAIL: no master material")
        with open(result_path, "w") as f:
            f.write("ok=False\n%s\n" % "\n".join(results))
        print("RESULT: ok=False")
        return False

    graph_ok = True
    existing = unreal.MaterialEditingLibrary.get_material_expressions(master)
    if len(existing) >= len(WIRING):
        log("master already has %d expressions, skipping graph build"
            % len(existing))
    else:
        prop_by_suffix = {
            suffix: getattr(unreal.MaterialProperty, prop_name)
            for suffix, _param, prop_name, _pin in WIRING
        }
        y = -600
        for suffix, param, _prop_name, pin in WIRING:
            tex = unreal.EditorAssetLibrary.load_asset(
                "%s/T_%s_%s" % (TEX_DIR, SET, suffix)
            )
            expr = unreal.MaterialEditingLibrary.create_material_expression(
                master, unreal.MaterialExpressionTextureSampleParameter2D,
                -900, y
            )
            expr.set_editor_property("parameter_name", param)
            if tex is not None:
                expr.set_editor_property("texture", tex)
            wired_ok = unreal.MaterialEditingLibrary.connect_material_property(
                expr, pin, prop_by_suffix[suffix]
            )
            graph_ok = graph_ok and bool(wired_ok)
            log("expression %s -> %s (tex=%s, wired=%s)" % (
                param, prop_by_suffix[suffix], tex is not None, wired_ok))
            y += 300
        log("graph built: %d expressions" % len(
            unreal.MaterialEditingLibrary.get_material_expressions(master)))

    saved = unreal.EditorAssetLibrary.save_asset(MASTER_PATH)
    log("save master -> %s" % saved)

    # --- Instance --------------------------------------------------------
    all_ok = bool(saved)
    if unreal.EditorAssetLibrary.does_asset_exist(MI_PATH):
        mi = unreal.EditorAssetLibrary.load_asset(MI_PATH)
        log("instance reused: %s" % MI_PATH)
    else:
        factory = unreal.MaterialInstanceConstantFactoryNew()
        how = try_set(factory, ("initial_parent", "InitialParent"), master)
        log("factory parent set via: %s" % how)
        mi = asset_tools.create_asset(
            MI_NAME, TEX_DIR, unreal.MaterialInstanceConstant, factory
        )
        log("instance created: %s -> %s" % (MI_PATH, mi is not None))
        if mi is not None and how is None:
            parent, how_get = try_get(mi, ("parent", "Parent"))
            log("instance parent after create: %s via %s" % (parent, how_get))
            if parent is None:
                if hasattr(unreal.MaterialEditingLibrary,
                           "set_material_instance_parent"):
                    unreal.MaterialEditingLibrary.set_material_instance_parent(
                        mi, master)
                    log("parent set via MaterialEditingLibrary")
                else:
                    log("parent set via: %s" % try_set(
                        mi, ("parent", "Parent"), master))
    if mi is None:
        all_ok = False
        log("FAIL: no instance %s" % MI_NAME)
    else:
        wired = True
        for suffix, param, _prop_name, _pin in WIRING:
            tex = unreal.EditorAssetLibrary.load_asset(
                "%s/T_%s_%s" % (TEX_DIR, SET, suffix)
            )
            if tex is None:
                wired = False
                log("FAIL: texture missing T_%s_%s" % (SET, suffix))
                continue
            try:
                unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
                    mi, param, tex)
            except Exception:
                wired = False
                log("FAIL: texture param %s/%s\n%s" % (
                    MI_NAME, param, traceback.format_exc()))
        mi_saved = unreal.EditorAssetLibrary.save_asset(MI_PATH)
        log("save %s -> %s (wired=%s)" % (MI_NAME, mi_saved, wired))
        all_ok = all_ok and wired and bool(mi_saved)

    # --- Verification ----------------------------------------------------
    tex_ok = True
    getter = getattr(
        unreal.MaterialEditingLibrary,
        "get_material_instance_texture_parameter_value", None)
    if mi is None or getter is None:
        tex_ok = False
        log("FAIL verify: instance or getter unavailable")
    else:
        for suffix, param, _prop_name, _pin in WIRING:
            exp = unreal.EditorAssetLibrary.load_asset(
                "%s/T_%s_%s" % (TEX_DIR, SET, suffix))
            got = None
            if getter is not None:
                try:
                    got = getter(mi, param)
                except Exception:
                    log("getter exc %s\n%s" % (param, traceback.format_exc()))
            same = (got is not None and exp is not None
                    and got.get_path_name() == exp.get_path_name())
            if not same:
                tex_ok = False
                log("FAIL verify %s -> %s (want %s)" % (
                    param,
                    got.get_path_name() if got else None,
                    exp.get_path_name() if exp else None))
    log("verify textures read back: %s" % tex_ok)

    present = [
        p for p in (MASTER_PATH, MI_PATH)
        if unreal.EditorAssetLibrary.does_asset_exist(p)
    ]
    n_expr = len(unreal.MaterialEditingLibrary.get_material_expressions(master))
    ok = (all_ok and graph_ok and tex_ok
          and len(present) == 2 and n_expr >= len(WIRING))
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
        print("[METAL] UNHANDLED\n%s" % traceback.format_exc())
