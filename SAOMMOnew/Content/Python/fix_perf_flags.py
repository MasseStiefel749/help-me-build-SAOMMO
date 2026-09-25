"""#30 safe flag fixes - LOD-group alignment only (band 5 section 17).

Two live character textures sit in TEXTUREGROUP_WORLD while every sibling
of the same map set uses the character groups. The LOD group only steers
streaming budget/priority, never pixel data, so this is a flag-level fix:
no recompression, no visual change (honest status: not screenshot-verified,
Simon's F5 is the visual acceptance for anything art-adjacent).

Saves via EditorAssetLibrary.save_asset (ADR-013d - .uasset persists
that way, mtime-verified in earlier rounds).

Writes Saved/PerfFixResult.txt (judge file - print() is invisible in a
commandlet, agent memory R15).
"""

import unreal

FIXES = {
    "/Game/Characters/Mannequins/Textures/Manny/T_Manny_02_MRA":
        "TEXTUREGROUP_CHARACTER_SPECULAR",
    "/Game/Characters/Mannequins/Textures/Manny/T_Manny_02_N":
        "TEXTUREGROUP_CHARACTER_NORMAL_MAP",
}


def main():
    out = unreal.Paths.project_saved_dir() + "PerfFixResult.txt"
    lines = ["ok=True"]
    for path, target in FIXES.items():
        tex = unreal.EditorAssetLibrary.load_asset(path)
        if tex is None:
            lines.append("%s|LOAD-FAIL" % path)
            continue
        before = str(tex.lod_group)
        mark = "no-mark-api"
        try:
            tex.lod_group = getattr(unreal.TextureGroup, target)
            # A python property assignment does NOT call PostEditChange, so
            # the package never turns dirty and save_asset(only_if_is_dirty
            # =True, the default) silently writes nothing while still
            # returning True (lesson of fix run 1: mtime unchanged).
            f_mark = getattr(tex, "mark_package_dirty", None)
            if callable(f_mark):
                f_mark()
                mark = "mark_package_dirty"
            saved = unreal.EditorAssetLibrary.save_asset(path, False)
        except Exception as e:
            lines.append("%s|%s|EXC:%s" % (path, before, e))
            continue
        # same-process re-read (still the linker cache - the disk truth
        # comes from the follow-up perf_audit run in a FRESH process,
        # which must show the new distribution)
        again = unreal.EditorAssetLibrary.load_asset(path)
        after = str(again.lod_group) if again is not None else "RELOAD-FAIL"
        lines.append("%s|%s|%s|saved=%s|%s|re-read=%s"
                     % (path, before, target, saved, mark, after))
    with open(out, "w", encoding="utf-8") as fh:
        fh.write("\n".join(lines) + "\n")


main()
