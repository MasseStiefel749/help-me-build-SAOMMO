"""R19/#29: resolve one representative asset per naming prefix to its real
class, so the Band 5 section 14 expansion documents verified types instead of
guessed glosses (Band 6 section 19).

Writes Saved/NamingProbeResult.txt (durable evidence channel - python print()
never reaches stdout in a commandlet, see Agent memory R15).
"""

import unreal

PROBES = [
    ("MF", "/Game/Characters/Mannequins/Anims/Pistol/MF_Pistol_Idle_ADS"),
    ("MM", "/Game/Characters/Mannequins/Anims/Unarmed/MM_Attack_01"),
    ("BP", "/Game/Blueprints/BP_SAOMMOCharacter"),
    ("T", "/Game/Materials/T_Ground037_Color"),
    ("SM", "/Game/Props/SM_Barrel_Brunnfeld"),
    ("IA", "/Game/Input/IA_Attack"),
    ("MI", "/Game/Materials/MI_Planks009"),
    ("M", "/Game/Materials/M_CC0Surface"),
    ("UI", "/Game/Input/Touch/UI_Thumbstick"),
    ("IMC", "/Game/Input/IMC_SAOMMO"),
    ("UE", "/Game/Levels/UE_Placeholder"),
    ("BPI", "/Game/Input/Touch/BPI_TouchInterface"),
    ("ABP", "/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"),
    ("AM", "/Game/Variant_Combat/Anims/AM_ChargedAttack"),
    ("NS", "/Game/Variant_Combat/VFX/NS_Damage"),
    ("EnvQuery", "/Game/Variant_Combat/Blueprints/AI/EnvQuery_Evade"),
    ("CR", "/Game/Characters/Mannequins/Rigs/CR_Mannequin_Body"),
    ("AO", "/Game/Characters/Mannequins/Anims/Pistol/Aim/AO_Pistol"),
    ("ST", "/Game/Variant_Combat/Blueprints/AI/ST_CombatEnemy"),
    ("SKM", "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"),
    ("PA", "/Game/Characters/Mannequins/Rigs/PA_Mannequin"),
    ("SK", "/Game/Characters/Mannequins/Meshes/SK_Mannequin"),
    ("BS", "/Game/Characters/Mannequins/Anims/Unarmed/BS_Idle_Walk_Run"),
]

lines = []


def main():
    out = unreal.Paths.project_saved_dir() + "NamingProbeResult.txt"
    for prefix, path in PROBES:
        cls = "LOAD-FAIL"
        try:
            asset = unreal.EditorAssetLibrary.load_asset(path)
            if asset is None:
                cls = "NONE"
            else:
                cls = asset.get_class().get_name()
        except Exception as exc:  # keep going, one failure must not kill the probe
            cls = "EXC: %s" % exc
        lines.append("%s|%s|%s" % (prefix, path, cls))
    try:
        with open(out, "w", encoding="utf-8") as fh:
            fh.write("ok=True\n")
            fh.write("\n".join(lines) + "\n")
    except Exception as exc:
        lines.append("WRITE-FAIL: %s" % exc)


main()
