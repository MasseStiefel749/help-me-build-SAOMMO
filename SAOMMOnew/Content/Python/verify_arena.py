"""Headless inventory + verification for the L_StartingReach test arena (spec §8).

Run: UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<this>"

Loads the map, dumps every actor (label/class/location) and evaluates the
spec-arena checklist items that can be proven without a headset:

  [1] traverse geometry (path chain + platform + player start present)
  [2] enemy spawners match the zone/threat plan (spec §5)
  [3] landmarks present (well, dummies, gate arch, ruin monument)
  [4] ruin Artifact teaser (monument + accent light, no item pickup)
  [5] lighting rig complete (sun/skylight/atmosphere + ruin accent)
  [6] dressing/props carry the MI_ materials (Band 5 §9)

Items needing PIE/headset (real traversal, VR+desktop camera, feel) are
reported as OPEN - they belong to Simon's playtest (backlog #2/#8).
Writes <Project>/Saved/ArenaCheckResult.txt.
"""
import traceback

MAP_PATH = "/Game/Levels/StartingReach/L_StartingReach"
MI_DIR = "/Game/Materials"

results = []
checks = []


def log(msg):
    results.append(msg)
    print("[ARENA] " + msg)


def check(name, ok, detail=""):
    checks.append((name, bool(ok), detail))
    log("CHECK %s -> %s %s" % (name, "PASS" if ok else "FAIL", detail))


def main():
    import unreal
    result_path = unreal.Paths.project_saved_dir() + "ArenaCheckResult.txt"

    ok_load = unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH)
    log("load -> %s" % ok_load)
    if not ok_load:
        check("map loads", False)
    else:
        check("map loads", True)

        subsys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        actors = subsys.get_all_level_actors()
        labels = {}
        inventory = []
        for a in actors:
            try:
                label = a.get_actor_label()
            except Exception:
                label = str(a)
            loc = a.get_actor_location()
            cls = a.get_class().get_name()
            labels[label] = (cls, loc)
            inventory.append("%-34s %-24s (%.0f, %.0f, %.0f)" % (
                label, cls, loc.x, loc.y, loc.z))
        log("actor count = %d" % len(actors))
        results.extend(inventory)

        def has(*fragments):
            return [l for l in labels if all(f in l for f in fragments)]

        # [1] traverse geometry: spawn, path chain, platform.
        path_found = [l for l in labels if l.startswith("Path_")]
        spawn = has("Spawn_Brunnfeld")
        plat = has("Platform_Klingenhof")
        check("traverse geometry (paths/spawn/platform)",
              len(path_found) >= 5 and spawn and plat,
              "paths=%d spawn=%s platform=%s" % (
                  len(path_found), bool(spawn), bool(plat)))

        # [2] spawners per spec §5: Grauwaldrand 2-4, Ruine outer 1-2 -
        # AND inside their zone x-ranges (EnemySpawner spawns around its own
        # actor location, so an origin placement would pollute the safe town).
        spawners = has("Spawner")
        g = [l for l in spawners if "Grauwaldrand" in l]
        r = [l for l in spawners if "Ruine" in l or "Approach" in l]
        bad_zone = [l for l in g if not 12500.0 <= labels[l][1].x <= 16500.0]
        bad_zone += [l for l in r if not 16500.0 <= labels[l][1].x <= 21500.0]
        check("spawners vs threat plan (G 2-4, Ruine 1-2, zoned)",
              2 <= len(g) <= 4 and 1 <= len(r) <= 2 and not bad_zone,
              "Grauwaldrand=%d Ruine=%d bad_zone=%s all=%s" % (
                  len(g), len(r), bad_zone, spawners))

        # [3] landmarks (spec §3): well, dummies, gate arch, ruin monument.
        well = has("Well")
        dummies = [l for l in labels if "Dummy" in l]
        arch = has("Arch") or has("Gate")
        monument = [l for l in labels if "Monument" in l]
        check("landmarks (well/dummies/arch/monument)",
              bool(well) and len(dummies) >= 1 and bool(arch) and bool(monument),
              "well=%s dummies=%d arch=%s monument=%s" % (
                  bool(well), len(dummies), bool(arch), monument))

        # [4] artifact teaser: monument present, no obtainable Artifact item.
        artifacts = has("ArtifactPickup", "Pickup_Artifact")
        check("ruin teaser without grantable Artifact",
              bool(monument) and not artifacts,
              "monument=%s artifactPickups=%s" % (bool(monument), artifacts))

        # [5] lighting rig (spec §6).
        sun = has("Sun")
        sky = has("Sky")
        accent = has("Accent") or [l for l in labels if "RuineLight" in l
                                   or "Light_Ruine" in l]
        check("lighting rig (sun/sky/atmo + ruin accent)",
              bool(sun) and len(sky) >= 2 and bool(accent),
              "sun=%s sky=%s accent=%s" % (bool(sun), len(sky), accent))

        # [6] dressing carries MI_ materials (Band 5 §9).
        mi_names = ["MI_Ground037", "MI_Rock063", "MI_Planks009",
                    "MI_PracticeBlade"]
        mi_paths = ["%s/%s" % (MI_DIR, n) for n in mi_names]
        mi_objs = [unreal.EditorAssetLibrary.load_asset(p) for p in mi_paths]
        mats_ok = all(o is not None for o in mi_objs)
        dressed = []
        bare = []
        for label, (cls, loc) in labels.items():
            if cls != "StaticMeshActor":
                continue
            actor = next((a for a in actors
                          if a.get_actor_label() == label), None)
            if actor is None:
                continue
            comp = actor.get_component_by_class(unreal.StaticMeshComponent)
            if comp is None:
                continue
            mat = comp.get_material(0)
            mat_name = mat.get_name() if mat is not None else "None"
            if mat_name in mi_names:
                dressed.append(label)
            elif label.startswith(("Rock_", "RuineWall_", "Path_", "Platform_",
                                   "Ground", "Well", "Scaffold", "BladeRack",
                                   "BladeMonument", "PracticeBlade")):
                # PracticeBlade must carry MI_PracticeBlade since backlog #20
                # (M_CC0Metal closed the spec-§7 "no metal MI" gap).
                bare.append("%s:%s" % (label, mat_name))
        check("dressing/props use MI_ instances",
              mats_ok and not bare and len(dressed) >= 10,
              "dressed=%d bare=%s" % (len(dressed), bare))

    # Report: checklist items needing PIE/headset stay OPEN by definition.
    ok = all(c[1] for c in checks)
    lines = ["ok=%s" % ok, ""]
    lines += ["%s | %s | %s" % ("PASS" if c[1] else "FAIL", c[0], c[2])
              for c in checks]
    lines += ["", "OPEN (PIE/headset - Simon):", "- real traverse without cheats",
              "- zones match threat tier in play", "- landmarks visible from entry",
              "- lore contradictions (manual)", "- blockout playable VR + desktop"]
    lines += [""] + results
    with open(result_path, "w") as f:
        f.write("\n".join(lines) + "\n")
    log("checks=%d failed=%d -> %s" % (
        len(checks), sum(1 for c in checks if not c[1]), result_path))
    print("ARENA_RESULT: ok=%s" % ok)
    return ok


if __name__ == "__main__":
    try:
        main()
    except Exception:
        print("[ARENA] UNHANDLED\n%s" % traceback.format_exc())
