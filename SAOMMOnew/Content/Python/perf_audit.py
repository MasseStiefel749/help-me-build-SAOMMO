"""#30 asset performance audit - band 5 section 17 (headless subset).

Measurable headless: texture size/compression/LOD-group/sRGB (+ estimated
memory ceiling), material expression counts, MI parameter counts, uasset
file sizes. NOT measurable headless (documented as such in the report):
draw calls, VFX cost, animation cost, VR rendering cost - those need a
live session.

Every metric uses an hasattr/try cascade and records n/a or API-MISSING
instead of guessing (band 6 section 19: no invented facts; R18 showed
several editor libraries return -1 inside commandlets).

Writes Saved/PerfAuditResult.txt - python print() never reaches stdout in
a commandlet (agent memory R15), so the result file is the judge.
"""

import os
import unreal

# candidate size APIs on UTexture2D, tried in order
TEX_SIZE_PAIRS = [
    ("blueprint_get_size_x", "blueprint_get_size_y"),
    ("get_width", "get_height"),
    ("get_size_x", "get_size_y"),
]

# candidate LOD-count APIs on static meshes (expected -1 per R18 dead
# ends - recorded anyway so the report can state that with evidence)
MESH_LOD_CANDIDATES = ["get_num_lods", "get_lod_count"]

SKIP_PREFIX = ("__External",)

CONTENT_DIR = unreal.Paths.project_content_dir()


def package_of(p):
    """'/Game/A/B.B' -> '/Game/A/B' (object path to package path)."""
    return p.split(".", 1)[0] if p.count(".") >= 1 and "/Game/" in p else p


def file_kb(p):
    """Resolve a /Game package path to its on-disk size in KB."""
    rel = package_of(p)[len("/Game/"):]
    for ext in (".uasset", ".umap"):
        fp = os.path.join(CONTENT_DIR, rel + ext)
        if os.path.exists(fp):
            return os.path.getsize(fp) / 1024.0
    return -1


def main():
    out = unreal.Paths.project_saved_dir() + "PerfAuditResult.txt"
    lines = []
    tex_api = {}

    all_paths = unreal.EditorAssetLibrary.list_assets(
        "/Game", recursive=True, include_folder=False
    )

    textures, materials, instances, meshes, others = [], [], [], [], []
    for p in all_paths:
        if any(p.replace("/Game/", "").startswith(s) for s in SKIP_PREFIX):
            continue
        try:
            data = unreal.EditorAssetLibrary.find_asset_data(p)
            cls = str(data.asset_class_path.asset_name)
        except Exception:
            cls = "UNKNOWN"
        if cls == "Texture2D":
            textures.append(p)
        elif cls == "Material":
            materials.append(p)
        elif cls == "MaterialInstanceConstant":
            instances.append(p)
        elif cls in ("StaticMesh", "SkeletalMesh"):
            meshes.append(p)
        else:
            others.append((p, cls))

    lines.append("ok=True")
    lines.append("== INVENTORY ==")
    lines.append("textures=%d materials=%d instances=%d meshes=%d other=%d"
                 % (len(textures), len(materials), len(instances),
                    len(meshes), len(others)))

    # ---------------- textures ----------------
    lines.append("== TEXTURES ==")
    lines.append("path|WxH|compression|lod_group|srgb|ceiling_MB|uasset_KB|size_api")
    total_mem = 0.0
    lod_groups = {}
    srgb_false = []
    big = []
    load_fails = []
    for p in sorted(textures):
        name = package_of(p)[len("/Game/"):]
        kb = file_kb(p)
        try:
            t = unreal.EditorAssetLibrary.load_asset(p)
        except Exception:
            t = None
        if t is None:
            load_fails.append(name)
            lines.append("%s|LOAD-FAIL||||||%.0f|" % (name, kb))
            continue
        w = h = None
        size_api = "none"
        for mx, my in TEX_SIZE_PAIRS:
            fx, fy = getattr(t, mx, None), getattr(t, my, None)
            if callable(fx) and callable(fy):
                try:
                    w, h = int(fx()), int(fy())
                    size_api = mx
                    break
                except Exception:
                    pass
        if w is None:
            size_api = "API-MISSING"
        tex_api[size_api] = tex_api.get(size_api, 0) + 1
        try:
            comp = str(t.compression_settings)
        except Exception:
            comp = "n/a"
        try:
            lg = str(t.lod_group)
        except Exception:
            lg = "n/a"
        lod_groups[lg] = lod_groups.get(lg, 0) + 1
        try:
            srgb = bool(t.srgb)
        except Exception:
            srgb = None
        if srgb is False:
            srgb_false.append(name)
        est = -1.0
        if w and h:
            # raw RGBA32 upper bound of the base mip (BC-compressed assets
            # land below this - a ceiling, not a claim)
            est = w * h * 4.0 / (1024.0 * 1024.0)
            total_mem += est
            if w >= 4096 or h >= 4096:
                big.append("%s %dx%d" % (name, w, h))
        lines.append("%s|%s|%s|%s|%s|%.1f|%.0f|%s"
                     % (name,
                        ("%dx%d" % (w, h)) if w else "n/a",
                        comp, lg,
                        {True: "true", False: "false", None: "n/a"}[srgb],
                        est, kb, size_api))
    lines.append("texture_base_mip_ceiling_MB=%.1f" % total_mem)
    lines.append("texture_size_api=%s" % tex_api)
    lines.append("lod_group_distribution=%s" % lod_groups)
    lines.append("srgb_false=%s" % (srgb_false or "[]"))
    lines.append("ge4k=%s" % (big or "[]"))
    lines.append("texture_load_fails=%s" % (load_fails or "[]"))

    # ---------------- materials ----------------
    lines.append("== MATERIALS ==")
    lines.append("path|expressions|uasset_KB|api")
    expr_counts = []
    f_expr = getattr(unreal.MaterialEditingLibrary, "get_material_expressions", None)
    for p in sorted(materials):
        name = package_of(p)[len("/Game/"):]
        kb = file_kb(p)
        cnt, how = "n/a", "n/a"
        try:
            m = unreal.EditorAssetLibrary.load_asset(p)
        except Exception:
            m = None
        if m is not None:
            if callable(f_expr):
                try:
                    cnt = len(f_expr(m))
                    how = "MaterialEditingLibrary.get_material_expressions"
                except Exception as e:
                    cnt = "EXC:%s" % e
                    how = "raised"
            else:
                how = "API-MISSING"
        if isinstance(cnt, int):
            expr_counts.append(cnt)
        lines.append("%s|%s|%.0f|%s" % (name, cnt, kb, how))
    if expr_counts:
        lines.append("material_expressions min=%d max=%d avg=%.1f"
                     % (min(expr_counts), max(expr_counts),
                        sum(expr_counts) / float(len(expr_counts))))

    # ---------------- material instances ----------------
    lines.append("== MATERIAL_INSTANCES ==")
    lines.append("path|tex_params|scalar_params|vector_params|uasset_KB")
    mi_counts = []
    for p in sorted(instances):
        name = package_of(p)[len("/Game/"):]
        kb = file_kb(p)
        try:
            mi = unreal.EditorAssetLibrary.load_asset(p)
        except Exception:
            mi = None
        vals = []
        for prop in ("texture_parameter_values", "scalar_parameter_values",
                     "vector_parameter_values"):
            v = getattr(mi, prop, None) if mi is not None else None
            try:
                n = len(v)
                vals.append(n)
            except Exception:
                vals.append(-1)
        mi_counts.append(sum(v for v in vals if isinstance(v, int) and v > 0))
        lines.append("%s|%s|%s|%s|%.0f"
                     % (name, vals[0], vals[1], vals[2], kb))
    if mi_counts:
        lines.append("mi_param_total min=%d max=%d"
                     % (min(mi_counts), max(mi_counts)))

    # ---------------- meshes ----------------
    lines.append("== MESHES ==")
    lines.append("path|class|uasset_KB|lod_probe|tri_lod0|collision")
    mesh_api = {}
    for p in sorted(meshes):
        name = package_of(p)[len("/Game/"):]
        kb = file_kb(p)
        try:
            data = unreal.EditorAssetLibrary.find_asset_data(p)
            cls = str(data.asset_class_path.asset_name)
        except Exception:
            cls = "?"
        lodinfo = "n/a"
        tri = "n/a"
        coll = "n/a"
        if cls == "StaticMesh":
            try:
                m = unreal.EditorAssetLibrary.load_asset(p)
            except Exception:
                m = None
            if m is not None:
                done = False
                for cand in ("get_num_lods", "get_lod_count",
                             "get_triangle_count", "get_section_count"):
                    if done:
                        break
                    for f, owner in (
                            (getattr(unreal.EditorStaticMeshLibrary,
                                     cand, None), "lib"),
                            (getattr(m, cand, None), "inst")):
                        if not callable(f):
                            continue
                        for args in ((m,), ()):
                            try:
                                v = f(*args)
                                lodinfo = "%s[%s%s]->%s" % (
                                    cand, owner,
                                    "" if args else "/0arg", v)
                                done = True
                                break
                            except Exception as e:
                                lodinfo = "%s[%s%s] EXC:%s: %s" % (
                                    cand, owner, "" if args else "/0arg",
                                    type(e).__name__, str(e)[:80])
                        if done:
                            break
                if lodinfo == "n/a":
                    lodinfo = "API-MISSING"
                # triangle count, probe-proven signature: instance method
                # with the LOD index as the only argument
                tri = "n/a"
                ftri = getattr(m, "get_num_triangles", None)
                if callable(ftri):
                    for args in ((0,), ()):
                        try:
                            tri = str(ftri(*args))
                            break
                        except Exception:
                            pass
                # collision flag (row 30 asks for it; R18 proved FKAggregateGeom
                # is unexposed for writes - probe what is READABLE and say so)
                bs = getattr(m, "body_setup", "PROP-NOT-EXPOSED")
                if isinstance(bs, str):
                    coll = "body_setup prop not exposed"
                elif bs is None:
                    coll = "body_setup=None"
                else:
                    ctf = getattr(bs, "collision_trace_flag", None)
                    if ctf is None:
                        ctf = "ctf-not-exposed"
                    coll = "body_setup=yes %s" % ctf
        key = lodinfo.split("->")[0].split(" ")[0]
        mesh_api[key] = mesh_api.get(key, 0) + 1
        lines.append("%s|%s|%.0f|%s|%s|%s" % (name, cls, kb, lodinfo, tri, coll))
    lines.append("mesh_lod_api_distribution=%s" % mesh_api)

    # ---------------- others ----------------
    lines.append("== OTHER (class distribution, sizes in textures section only) ==")
    cls_dist = {}
    total_other_kb = 0.0
    for p, cls in sorted(others):
        cls_dist[cls] = cls_dist.get(cls, 0) + 1
        total_other_kb += max(file_kb(p), 0.0)
    lines.append("class_distribution=%s" % cls_dist)
    lines.append("other_total_KB=%.0f" % total_other_kb)

    # ---------------- enum reference (for future flag fixes) ----------
    lines.append("== REFERENCE ==")
    try:
        groups = sorted(m for m in dir(unreal.TextureGroup)
                        if m.startswith("TEXTUREGROUP_"))
        lines.append("texture_groups=%s" % groups)
    except Exception as e:
        lines.append("texture_groups=EXC:%s" % e)
    lines.append("editor_static_mesh_library_build_lods=%s"
                 % hasattr(unreal.EditorStaticMeshLibrary, "build_lods"))

    with open(out, "w", encoding="utf-8") as fh:
        fh.write("\n".join(lines) + "\n")


main()
