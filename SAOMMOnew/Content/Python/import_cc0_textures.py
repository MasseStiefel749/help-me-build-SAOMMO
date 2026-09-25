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

DEST_PATH = "/Game/Materials"

# ambientCG file suffix -> (T_ suffix, sRGB, compression settings)
MAPS = (
    ("Color", "Color", True, "TC_DEFAULT"),
    ("NormalGL", "Normal", False, "TC_NORMALMAP"),
    ("Roughness", "Roughness", False, "TC_DEFAULT"),
    ("AmbientOcclusion", "AO", False, "TC_DEFAULT"),
    ("Metalness", "Metalness", False, "TC_DEFAULT"),
)

results = []


def log(msg):
    results.append(msg)
    print("[IMPORT] " + msg)


def main():
    import unreal
    result_path = unreal.Paths.project_saved_dir() + "TextureImportResult.txt"
    src_root = "D:/Assets/CC0/Textures"
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    all_ok = True
    imported = skipped = 0

    for folder, set_id in JOBS:
        src_dir = "%s/%s" % (src_root, folder)
        if not os.path.isdir(src_dir):
            log("SKIP set %s: source folder missing (%s)" % (set_id, src_dir))
            all_ok = False
            continue
        for file_suffix, map_name, srgb, compression in MAPS:
            src = "%s/%s_1K-JPG_%s.jpg" % (src_dir, set_id, file_suffix)
            dest_name = "T_%s_%s" % (set_id, map_name)
            dest_path = "%s/%s" % (DEST_PATH, dest_name)
            if not os.path.isfile(src):
                # Not every pack ships every map (metals have no AO).
                log("map absent (allowed): %s" % os.path.basename(src))
                continue

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
                    all_ok = False
                    continue
                imported += 1
                log("imported %s <- %s" % (dest_name, os.path.basename(src)))
            else:
                skipped += 1
                log("reused %s" % dest_name)

            tex = unreal.EditorAssetLibrary.load_asset(dest_path)
            if tex is None:
                log("FAIL: %s missing after import" % dest_path)
                all_ok = False
                continue
            want = getattr(unreal.TextureCompressionSettings, compression)
            tex.set_editor_property("srgb", srgb)
            tex.set_editor_property("compression_settings", want)
            saved = unreal.EditorAssetLibrary.save_asset(dest_path)
            got_srgb = tex.get_editor_property("srgb")
            got_comp = tex.get_editor_property("compression_settings")
            ok_here = bool(saved) and got_srgb == srgb and got_comp == want
            all_ok = all_ok and ok_here
            log("settings %s: srgb=%s/%s comp=%s/%s saved=%s" % (
                dest_name, got_srgb, srgb, str(got_comp).split(".")[-1],
                compression, saved))

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
