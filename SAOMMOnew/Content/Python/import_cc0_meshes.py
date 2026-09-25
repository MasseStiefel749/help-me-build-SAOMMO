"""Import kept Poly Haven CC0 FBX props into /Game/Props (Band 5 §8/§18).

Run: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<this>"

Backlog #24 (CC0-Prop-Pass): the slice props were still Cube/Cylinder
blockout proxies. JOBS maps the kept source files (D:\\Assets\\CC0\\Models,
downloaded + MD5-verified against the Poly Haven files API 2026-09-25,
license CC0 verified on polyhaven.com/license, rows in
ASSET-LICENSES.csv) to SM_* static meshes:

    katana_stand_01/katana_stand_01_1k.fbx             -> SM_BladeRack_Klingenhof
    Barrel_01/Barrel_01_1k.fbx                         -> SM_Barrel_Brunnfeld
    painted_wooden_bench/painted_wooden_bench_1k.fbx   -> SM_Bench_Brunnfeld

Import settings: mesh ONLY - no materials, no textures (Band 5 §9 /
backlog #24: props are dressed with the existing MI_* instances in
dress_arena.py, "kein Material-Neubau"). Every optional 5.8 import
property is set through a dual-name probe that logs acceptance, because
the FbxImportUI/FbxStaticMeshImportData surface is not guaranteed across
engine versions. Idempotent: replace_existing re-imports, so a bad first
import self-heals.

Evidence per mesh (temp StaticMeshActor probe, never saved):
  * world bounds extent -> real-world cm scale sanity check
  * material slot count -> dressing must dress every slot
  * body_setup present? -> collision state for the vault
Plus one-time introspection of collision helper availability.

Writes <Project>/Saved/MeshImportResult.txt and prints RESULT: ok=<bool>.
"""
import os
import traceback

# (source file relative to SRC_ROOT, SM_ asset name)
JOBS = [
    ("katana_stand_01/katana_stand_01_1k.fbx", "SM_BladeRack_Klingenhof"),
    ("Barrel_01/Barrel_01_1k.fbx", "SM_Barrel_Brunnfeld"),
    ("painted_wooden_bench/painted_wooden_bench_1k.fbx", "SM_Bench_Brunnfeld"),
]

SRC_ROOT = "D:/Assets/CC0/Models"
DEST_PATH = "/Game/Props"

lines = []


def log(msg):
    lines.append(msg)
    print("[MESH] " + msg)


def set_any(obj, names, value):
    """Try several property spellings; log what 5.8 actually accepts."""
    last = None
    for name in names:
        try:
            obj.set_editor_property(name, value)
            got = obj.get_editor_property(name)
            log("option %s -> %s (read-back %s)" % (name, value, got))
            return got == value
        except Exception as exc:
            last = exc
    log("option %s not accepted: %s" % (names, last))
    return False


def main():
    import unreal

    result_path = unreal.Paths.project_saved_dir() + "MeshImportResult.txt"
    success = False
    try:
        tools = unreal.AssetToolsHelpers.get_asset_tools()
        subsys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        sm_cls = unreal.load_class(None, "/Script/Engine.StaticMeshActor")
        if sm_cls is None:
            raise RuntimeError("StaticMeshActor class missing")

        imported = []
        for rel, name in JOBS:
            src = SRC_ROOT + "/" + rel
            if not os.path.isfile(src):
                log("SOURCE MISSING: " + src)
                continue

            task = unreal.AssetImportTask()
            task.filename = src
            task.destination_path = DEST_PATH
            task.destination_name = name
            set_any(task, ["automated"], True)
            set_any(task, ["replace_existing"], True)
            set_any(task, ["save"], True)

            opts = unreal.FbxImportUI()
            set_any(opts, ["import_mesh", "bImportMesh"], True)
            set_any(opts, ["import_as_skeletal", "bImportAsSkeletal"], False)
            # Band 5 §9 / backlog #24: MI_* only, no imported materials.
            set_any(opts, ["import_materials", "bImportMaterials"], False)
            set_any(opts, ["import_textures", "bImportTextures"], False)
            set_any(opts, ["import_animations", "bImportAnimations"], False)
            try:
                smd = opts.get_editor_property("static_mesh_import_data")
                if smd is None:
                    log("static_mesh_import_data is None on fresh FbxImportUI")
                else:
                    set_any(smd, ["combine_meshes", "bCombineMeshes"], True)
                    set_any(smd, ["generate_lightmap_uvs", "bGenerateLightmapUVs"],
                            True)
                    set_any(smd, ["auto_generate_collision", "bAutoGenerateCollision"],
                            True)
            except Exception as exc:
                log("static_mesh_import_data unavailable: %s" % exc)
            task.options = opts

            try:
                tools.import_asset_tasks([task])
            except Exception:
                log("IMPORT-THROW %s\n%s" % (name, traceback.format_exc()))
                continue

            asset_path = "%s/%s" % (DEST_PATH, name)
            if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
                try:
                    paths = task.get_editor_property("imported_object_paths")
                except Exception:
                    paths = "?"
                log("IMPORT-MISSING %s imported=%s" % (asset_path, paths))
                continue
            imported.append(asset_path)
            log("imported %s" % asset_path)

            mesh = unreal.EditorAssetLibrary.load_asset(asset_path)
            if mesh is None:
                log("LOAD-FAIL " + asset_path)
                continue

            # Bounds/slot/collision probe on a temp actor (world not saved).
            actor = None
            try:
                actor = subsys.spawn_actor_from_class(
                    sm_cls, unreal.Vector(0.0, 0.0, 0.0),
                    unreal.Rotator(0.0, 0.0, 0.0))
                actor.set_actor_label("__tmp_meshprobe")
                comp = actor.get_component_by_class(unreal.StaticMeshComponent)
                comp.set_static_mesh(mesh)
                origin, extent = actor.get_actor_bounds(False, False)
                log("%s extent=(%.1f, %.1f, %.1f) cm slots=%d origin=(%.1f, %.1f, %.1f)" % (
                    name, extent.x, extent.y, extent.z,
                    comp.get_num_materials(),
                    origin.x, origin.y, origin.z))
            except Exception:
                log("PROBE-FAIL %s\n%s" % (name, traceback.format_exc()))
            finally:
                if actor is not None:
                    try:
                        subsys.destroy_actor(actor)
                    except Exception as exc:
                        log("probe cleanup failed: %s" % exc)

            try:
                bs = mesh.get_editor_property("body_setup")
                log("%s body_setup=%s" % (name, "yes" if bs else "NONE"))
                agg = bs.get_editor_property("agg_geom")
                try:
                    boxes = agg.get_editor_property("box_elems")
                    nbox = len(boxes) if boxes is not None else -1
                except Exception:
                    nbox = -1
                log("%s agg box_elems=%d" % (name, nbox))
            except Exception as exc:
                log("%s body_setup/agg probe failed: %s" % (name, exc))

            # Collision for blockout props: FBX auto_generate_collision left
            # AggGeom empty and EditorStaticMeshLibrary.add_simple_collisions
            # silently returned -1 without adding anything in this commandlet
            # (probed 2026-09-25), so use the render mesh as collision - the
            # standard blockout setting. CTF_USE_DEFAULT would leave the
            # walk-through question open.
            flag = None
            for vname in ("CTF_USE_COMPLEX_AS_SIMPLE", "CTC_USE_COMPLEX_AS_SIMPLE",
                          "CTF_COMPLEX_AS_SIMPLE"):
                if hasattr(unreal.CollisionTraceFlag, vname):
                    flag = getattr(unreal.CollisionTraceFlag, vname)
                    break
            if flag is None:
                log("%s CollisionTraceFlag members: %s" % (
                    name, [x for x in dir(unreal.CollisionTraceFlag)
                           if not x.startswith("_")]))
            else:
                try:
                    set_any(mesh, ["collision_trace_flag"], flag)
                    saved = unreal.EditorAssetLibrary.save_asset(asset_path)
                    log("%s collision flag saved=%s read-back=%s" % (
                        name, saved, mesh.get_editor_property("collision_trace_flag")))
                except Exception as exc:
                    log("%s collision flag set failed: %s" % (name, exc))
            try:
                log("%s collision_complexity=%s" % (
                    name, unreal.EditorStaticMeshLibrary.get_collision_complexity(mesh)))
            except Exception as exc:
                log("%s complexity probe failed: %s" % (name, exc))

        # One-time API-surface evidence for collision follow-up (5.8).
        if hasattr(unreal, "EditorStaticMeshLibrary"):
            helpers = [x for x in dir(unreal.EditorStaticMeshLibrary)
                       if "coll" in x.lower() or "convex" in x.lower()]
            log("EditorStaticMeshLibrary collision helpers: %s" % helpers)
        else:
            log("EditorStaticMeshLibrary not exposed in 5.8 python")

        success = len(imported) == len(JOBS)
        log("imported=%d/%d" % (len(imported), len(JOBS)))

    except Exception:
        log("TOP-FAIL\n" + traceback.format_exc())

    with open(result_path, "w") as f:
        f.write("success=%s\n" % success)
        f.write("\n".join(lines) + "\n")
    print("[MESH] done success=%s" % success)
    print("RESULT: ok=%s" % success)
    return success


if __name__ == "__main__":
    main()
