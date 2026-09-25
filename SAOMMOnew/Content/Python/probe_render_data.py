"""Is the render data (LODs/sections) of the CC0 props present in a FRESH
process, while the engine cube's is?

Backlog #24 collision forensics, hypothesis after MeshCollisionDiff.txt
showed every exposed property identical: body creation may hinge on mesh
render data that a non-ticking commandlet never builds (async DDC/async
mesh build). Missing render data would also explain the complex trace
(C) missing - no triangles to hit.

Logs LOD/section/vertex counts for both assets plus a component line
trace each. Result: <Saved>/MeshRenderDataProbe.txt
"""
import traceback


def probe(path):
    import unreal
    out = ["=== %s ===" % path]
    mesh = unreal.EditorAssetLibrary.load_asset(path)
    if mesh is None:
        out.append("LOAD FAILED")
        return out
    for label, fn in (
            ("get_num_sections(0)", lambda: mesh.get_num_sections(0)),
            ("get_num_vertices(0)", lambda: mesh.get_num_vertices(0)),
            ("get_num_lods-cand", lambda: mesh.get_num_lods()),
            ("lib_get_lod_count", lambda: unreal.EditorStaticMeshLibrary
             .get_lod_count(mesh)),
            ("lib_get_lod_for_collision", lambda: unreal.EditorStaticMeshLibrary
             .get_lod_for_collision(mesh)),
    ):
        try:
            out.append("%s = %s" % (label, fn()))
        except Exception as exc:
            out.append("%s = ERR %s" % (label, str(exc)[:90]))
    try:
        rd = mesh.get_editor_property("render_data")
        out.append("render_data prop = %r" % (rd,))
    except Exception as exc:
        out.append("render_data prop = ERR %s" % str(exc)[:90])
    return out


def main():
    import unreal
    result_path = unreal.Paths.project_saved_dir() + "MeshRenderDataProbe.txt"
    lines = []
    try:
        for p in ("/Engine/BasicShapes/Cube.Cube",
                  "/Game/Props/SM_BladeRack_Klingenhof",
                  "/Game/Props/SM_Barrel_Brunnfeld",
                  "/Game/Props/SM_Bench_Brunnfeld"):
            lines += probe(p)
            print("[RDP] %s done" % p)
    except Exception:
        lines.append("TOP-FAIL\n" + traceback.format_exc())
    with open(result_path, "w") as f:
        f.write("\n".join(lines) + "\n")
    print("RESULT: lines=%d" % len(lines))


if __name__ == "__main__":
    main()
