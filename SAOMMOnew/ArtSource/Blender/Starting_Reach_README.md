# Starting Reach — Blender-Szene

`SAOMMO_Starting_Reach.blend` enthält eine editierbare Blockout-Umsetzung der bestehenden Regionsbeschreibung. Die Szene bildet Brunnfeld, Klingenhof, Grauwaldrand und die Ruine des Ersten Griffs in einem verbundenen Areal ab.

## Dateien

- `SAOMMO_Starting_Reach.blend` — vollständige Szene mit Kamera, Licht, Beschriftungen und modularen Einzelobjekten.
- `renders/SAOMMO_Starting_Reach.png` — gerenderte Gesamtübersicht.
- `Exports/Starting_Reach/SM_StartingReach_Blockout.fbx` — Mesh-Export für Unreal, Einheiten in Zentimetern, Mesh-Objekte einzeln benannt.
- `create_starting_reach_world.py` — Generator für die Blender-Szene, Vorschau und FBX.

## Aufbau

- Brunnfeld liegt bei (0,0), ist 50×50 m groß und enthält den zentralen Brunnen, Wiederaufbaugerüste und niedrige Mauern.
- Klingenhof liegt bei (60,0), ist 30×30 m groß und enthält vier Übungspuppen. Die 8-m-Verbindung führt aus Brunnfeld zum Übungsplatz.
- Grauwaldrand liegt bei (60,80), ist 80×80 m groß und besitzt den Torbogen am südlichen Zugang, Wald, Felsen und Ruinen-Deckung.
- Die Ruine liegt bei (60,160), umfasst einen linearen Hallenblock mit drei Räumen und einer versiegelten Nische. Der Lichtkörper ist nur ein Teaser.

Die Gestaltung bleibt beim dokumentierten Vertical Slice. FBX ist ein statischer Blockout-Export: Unreal-Materialeinstellungen, Kollision, Navmesh, Gegner, Gameplay-Interaktionen und Laufzeit sind nicht enthalten oder verifiziert. Im Unreal Editor bei Bedarf mit Importoptionen für separate statische Meshes importieren.
