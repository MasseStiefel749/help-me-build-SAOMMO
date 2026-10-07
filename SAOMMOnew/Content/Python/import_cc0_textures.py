"""Import unpacked ambientCG CC0 texture sets into /Game/Materials (Band 5 §9).

Run: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<this>"

Scans every source folder listed in JOBS (unpacked ambientCG *_1K-JPG zips on
D:\Assets\CC0\Textures, see the free-asset-pipeline skill) and imports the
standard PBR maps as T_<Set>_<Map> with the SAOMMO import settings recorded
in ASSET-LICENSES.csv:

    Color          sRGB on,  TC_DEFAULT      (albedo, gamma-encoded)
    NormalGL       sRGB off, TC_NORMALMAP    (OpenGL convention, matches UE)
    Roughness      sRGB off, TC_DEFAULT      (data map)
    AmbientOccl.   sRGB off, TC_DEFAULT      (data map)
    Metalness      sRGB off, TC_DEFAULT      (data map)

Backlog #41 (CC0-Vegetation, Band 5 ss8/ss9): the two vegetation sets
were downloaded UNPACKED from the Poly Haven files API, so their files
follow Poly Haven's own naming (<prefix>_<map>_1k.jpg) instead of the
ambientCG folder convention (<set>_1K-JPG_<Map>.jpg). They are listed in
VEG_JOBS and map onto the same T_<Set>_<Map> scheme (#41d):

    island_tree_02          -> T_IslandTree02_{Color,Normal,Roughness}
    island_tree_02_branches -> T_IslandTree02_Branches_*
    island_tree_02_leaves   -> T_IslandTree02_Leaves_*
    tree_stump_01           -> T_TreeStump01_{Color,Normal,Roughness}

The leaves variant additionally ships an opacity mask
(island_tree_02_leaves_alpha_1k.jpg). Neither job list knew alpha maps
before #41 - the "alpha" suffix in VEG_MAPS is that branch: sRGB off +
TC_DEFAULT like every other data map, destined for MP_OPACITY_MASK of the
masked foliage master M_CC0Foliage (build_vegetation_material.py). Every
source file is logged with its sha256, so TextureImportResult.txt doubles
as the hash record for the #41d commit.

Displacement and other maps are ignored (no displacement workflow in the
slice). Idempotent: assets that already exist are kept, but their sRGB flag
and compression settings are re-applied and saved, so a bad first import
self-heals. Writes <Project>/Saved/TextureImportResult.txt and prints
RESULT: ok=<bool>.
"""
import os
import traceback

# (source folder under D:\Assets\CC0\Textures, asset id used in T_ names)
# The three R8 sets (Ground037/Rock063/Planks009) were imported 2026-09-24
# from ad-hoc downloads whose folders are gone; they are already in
# /Game/Materials, so only sets with a kept source archive are listed here.
JOBS = [
    ("Metal038_1K-JPG", "Metal038"),
]

# Backlog #41d (CC0-Vegetation): unpacked Poly Haven sets. Every entry is
# (source folder under D:\Assets\CC0\Textures, set id used in T_ names,
# variants) and every variant is (file prefix, name suffix): the prefix
# builds the source file <prefix>_<map>_1k.jpg, the suffix builds the asset
# name T_<Set><suffix>_<Map>. "" is the base set (trunk + branches share the
# island_tree_02 diffuse/normal/roughness maps, see #41d).
VEG_JOBS = [
    ("island_tree_02", "IslandTree02", (
        ("island_tree_02", ""),
        ("island_tree_02_branches", "_Branches"),
        ("island_tree_02_leaves", "_Leaves"),
    )),
    ("tree_stump_01", "TreeStump01", (
        ("tree_stump_01", ""),
    )),
]

DEST_PATH = "/Game/Materials"

# ambientCG file suffix -> (T_ suffix, sRGB, compression settings)
MAPS = (
    ("Color", "Color", True, "TC_DEFAULT"),
    ("NormalGL", "Normal", False, "TC_NORMALMAP"),
    ("Roughness", "Roughness", False, "TC_DEFAULT"),
    ("AmbientOcclusion", "AO", False, "TC_DEFAULT"),
    ("Metalness", "Metalness", False, "TC_DEFAULT"),
)

# Poly Haven file suffix -> (T_ suffix, sRGB, compression settings), the
# VEG_JOBS counterpart of MAPS. "alpha" is the #41d opacity-mask branch: the
# mask is a plain data map (white = leaf, black = cut out), so sRGB stays
# off and TC_DEFAULT keeps the R channel MP_OPACITY_MASK reads.
VEG_MAPS = (
    ("diff", "Color", True, "TC_DEFAULT"),
    ("nor_gl", "Normal", False, "TC_NORMALMAP"),
    ("rough", "Roughness", False, "TC_DEFAULT"),
    ("alpha", "Alpha", False, "TC_DEFAULT"),
)

results = []


def log(msg):
    results.append(msg)
    print("[IMPORT] " + msg)


def sha256_hex(path):
    """sha256 of a source file, logged as the #41 hash record."""
    import hashlib
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for chunk in iter(lambda: handle.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


counts = {"imported": 0, "skipped": 0}


def ensure_texture(src, dest_name, srgb, compression, asset_tools):
    """One T_* asset: import when missing, then (re)apply sRGB/compression.

    Returns True when the asset is on disk with the wanted settings. A
    missing SOURCE file is allowed (not every pack ships every map: metals
    have no AO, only the leaves variant carries an alpha mask) - a missing
    asset after the import is not.
    """
    import unreal
    dest_path = "%s/%s" % (DEST_PATH, dest_name)
    if not os.path.isfile(src):
        log("map absent (allowed): %s" % os.path.basename(src))
        return True

    log("source sha256 %s = %s" % (os.path.basename(src), sha256_hex(src)))

    if not unreal.EditorAssetLibrary.does_asset_exist(dest_path):
        task = unreal.AssetImportTask()
        task.filename = os.path.abspath(src)
        task.destination_path = DEST_PATH
        task.destination_name = dest_name
        task.automated = True
        task.save = False
        task.replace_existing = False
        try:
            asset_tools.import_asset_tasks([task])
        except Exception:
            log("IMPORT-FAIL %s\n%s" % (dest_name, traceback.format_exc()))
            return False
        counts["imported"] += 1
        log("imported %s <- %s" % (dest_name, os.path.basename(src)))
    else:
        counts["skipped"] += 1
        log("reused %s" % dest_name)

    tex = unreal.EditorAssetLibrary.load_asset(dest_path)
    if tex is None:
        log("FAIL: %s missing after import" % dest_path)
        return False
    want = getattr(unreal.TextureCompressionSettings, compression)
    tex.set_editor_property("srgb", srgb)
    tex.set_editor_property("compression_settings", want)
    saved = unreal.EditorAssetLibrary.save_asset(dest_path)
    got_srgb = tex.get_editor_property("srgb")
    got_comp = tex.get_editor_property("compression_settings")
    ok_here = bool(saved) and got_srgb == srgb and got_comp == want
    log("settings %s: srgb=%s/%s comp=%s/%s saved=%s" % (
        dest_name, got_srgb, srgb, str(got_comp).split(".")[-1],
        compression, saved))
    return ok_here


def main():
    import unreal
    result_path = unreal.Paths.project_saved_dir() + "TextureImportResult.txt"
    src_root = "D:/Assets/CC0/Textures"
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    all_ok = True

    # ambientCG sets (unpacked *_1K-JPG zips, original job list).
    for folder, set_id in JOBS:
        src_dir = "%s/%s" % (src_root, folder)
        if not os.path.isdir(src_dir):
            log("SKIP set %s: source folder missing (%s)" % (set_id, src_dir))
            all_ok = False
            continue
        for file_suffix, map_name, srgb, compression in MAPS:
            src = "%s/%s_1K-JPG_%s.jpg" % (src_dir, set_id, file_suffix)
            dest_name = "T_%s_%s" % (set_id, map_name)
            all_ok = ensure_texture(src, dest_name, srgb, compression,
                                    asset_tools) and all_ok

    # Poly Haven sets (backlog #41d, unpacked, <prefix>_<map>_1k.jpg).
    for folder, set_id, variants in VEG_JOBS:
        src_dir = "%s/%s" % (src_root, folder)
        if not os.path.isdir(src_dir):
            log("SKIP vegetation set %s: source folder missing (%s)" % (
                set_id, src_dir))
            all_ok = False
            continue
        for prefix, name_suffix in variants:
            for map_suffix, map_name, srgb, compression in VEG_MAPS:
                src = "%s/%s_%s_1k.jpg" % (src_dir, prefix, map_suffix)
                dest_name = "T_%s%s_%s" % (set_id, name_suffix, map_name)
                all_ok = ensure_texture(src, dest_name, srgb, compression,
                                        asset_tools) and all_ok

    imported = counts["imported"]
    skipped = counts["skipped"]
    ok = all_ok and (imported + skipped) > 0
    with open(result_path, "w") as f:
        f.write("ok=%s imported=%d reused=%d\n%s\n" % (
            ok, imported, skipped, "\n".join(results)))
    log("result written: %s" % result_path)
    print("RESULT: ok=%s imported=%d reused=%d" % (ok, imported, skipped))
    return ok


if __name__ == "__main__":
    try:
        main()
    except Exception:
        print("[IMPORT] UNHANDLED\n%s" % traceback.format_exc())
