# SAOMMO – World Props

Stilisierte Low-Poly-Weltobjekte passend zum Ruinen-/Klingenhof-Prototyp. Alle Modelle wurden in Blender 5.2.1 gebaut, als einzelne Mesh-Objekte gespeichert und separat als FBX exportiert.

## Modelle

- `SM_RuinArch_Broken_01` – beschädigter Steinbogen mit Bruchstück
- `SM_Waystone_Checkpoint_01` – Runen-Wegstein/Checkpoint
- `SM_SwordMonument_01` – Schwertdenkmal auf Steinsockel
- `SM_RuinPillar_Broken_01` – gebrochene Säule
- `SM_AncientWell_01` – alter Brunnen mit Kurbel
- `SM_CampBrazier_01` – Kohlebecken mit stilisierter Flamme
- `SM_OldSupplyCrate_01` – Versorgungskiste
- `SM_KlingenhofBench_01` – Bank für den Klingenhof

## Dateien

- `SAOMMO_World_Props.blend` – editierbare Galerie mit allen acht Meshes
- `Exports/FBX/` – je ein FBX pro Modell, mit Material-Slots
- `renders/SAOMMO_world_props.png` – Galerie-Vorschau
- `create_saommo_world_props.py` – Blender-Erzeugung und FBX-Export

## Unreal-Import

Im Unreal Content Browser `Import` wählen und die FBX-Dateien einzeln aus `Exports/FBX/` importieren. Als Static Mesh importieren. Die FBX-Dateien enthalten Material-Slots, aber keine finalen Unreal-Materialien oder Texturen; dafür Material-Instanzen im Projekt anlegen. Die Modelle haben noch keine Unreal-Collision-Meshes und sind visuelle Prototypen; Collision bei Bedarf in Unreal generieren oder pro Asset ergänzen. Nach dem ersten Import Maßstab im Unreal-Viewport prüfen; die Meshes sind in Blender metrisch angelegt.

Alle Formen sind neu für SAOMMO gestaltet. Die Flamme ist statische Geometrie; sie ist noch kein Niagara-Effekt.
