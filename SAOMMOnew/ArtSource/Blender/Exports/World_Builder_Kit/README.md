# SAOMMO Modular World Builder Kit

Ein modulares Low-Poly-Bauset für den SAOMMO-Weltprototyp. Die Formen teilen eine gedämpfte Stein-, Holz- und Waldpalette. Das Rastermodul beträgt 4 Meter (Blender metrisch; Unreal-FBX-Import prüfen). Modulare Flächen und Wände lassen sich in Unreal zu Wegen, Ruinen, Höfen und kleinen Siedlungen kombinieren.

## Dateien

- `SAOMMO_World_Builder_Kit.blend` – vollständige Galerie mit allen 36 editierbaren Einzelmeshes
- `Exports/World_Builder_Kit/FBX/` – ein FBX je Modell, bereit für Unreal Static Mesh Import
- `renders/SAOMMO_world_builder_kit.png` – Übersicht
- `create_saommo_builder_kit.py` – reproduzierbares Erzeugungsskript

## Inhalt (36 Modelle)

**Gelände und Wege:** `SM_Terrain_Grass_4m`, `SM_Terrain_Dirt_4m`, `SM_Terrain_CliffEdge_4m`, `SM_Path_Stone_4m`, `SM_Path_Dirt_4m`.

**Fels und Ruinen:** `SM_RockCluster_Small`, `SM_RockCluster_Medium`, `SM_RockCluster_Large`, `SM_Ruin_RubbleCluster_01`, `SM_Waystone_Rune_01`.

**Steinarchitektur:** `SM_Floor_Stone_4m`, `SM_Floor_Stone_Damaged_4m`, `SM_Wall_Stone_4x3`, `SM_Wall_Half_Stone_4x1p5`, `SM_Wall_Corner_Stone_4m`, `SM_Wall_Doorway_Stone_4x3`, `SM_Wall_Window_Stone_4x3`, `SM_Pillar_Stone_3m`, `SM_Stairs_Stone_4x4`, `SM_Ramp_Stone_4x4`, `SM_Roof_GableHalf_4m`, `SM_Roof_Ridge_4m`.

**Holzbau und Siedlung:** `SM_Wall_Timber_4x3`, `SM_Door_Wood_2x3`, `SM_Fence_Straight_4m`, `SM_Fence_Corner_4m`, `SM_Bridge_Segment_4m`, `SM_LanternPost_01`, `SM_Signpost_01`, `SM_Cairn_01`.

**Natur:** `SM_Tree_Pine_Small`, `SM_Tree_Pine_Tall`, `SM_Tree_Deciduous_01`, `SM_Grass_Clump_01`, `SM_FallenLog_01`, `SM_Stump_01`.

## Unreal Import

Im Content Browser einen Ordner wie `/Game/World/BuilderKit` anlegen und die FBX-Dateien aus `Exports/World_Builder_Kit/FBX/` importieren. `Import as Static Mesh` verwenden. Für den ersten Test Maßstab eines 4-m-Bodenteils im Unreal-Viewport prüfen und anschließend Snap to Grid auf 400 cm setzen. Die Ursprünge liegen bei Terrain/Architektur am Boden in der Mitte; der 4-m-Rasterbezug ist bei den Modulen im Namen markiert.

Die FBX-Dateien bringen Material-Slots mit, aber keine finalen Unreal-Materialien oder Texturen. Alle Meshes sind visuelle Prototypen ohne Unreal-Collision; Kollision bei Bedarf beim Import generieren. Das Set baut eine solide Blockout-/Prototypwelt, ersetzt aber noch keine finalen Landschaftsmaterialien, Terrain-Landscape, Vegetationssysteme, LODs oder Performance-Optimierung.
