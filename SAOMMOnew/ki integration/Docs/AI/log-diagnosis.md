# Log-Diagnose-Pass (#40) — Stand 2026-10-07

Quelle: Backlog-Zeile #40, Dauerbetrieb-Runde. Verifiziert nach Band 6 §20
(Verification Before Completion). Kein Code geaendert, kein Build noetig
(ADR-017b: Doku/Script-Kommentar).

## 1 Zweck

Bestandsaufnahme aller Log-Klassen des Projekts mit Herkunft, Severity und
Handlungsbedarf. Bindend ist die Whitelist-Regel in
`Scripts/log-whitelist.txt`: nur Engine-Rauschen mit belegter Herkunft und
bekanntem Backlog-Ticket darf dort stehen; gefixte Fehlerklassen nie
reintun (Regel der Nacht-Archaeologie).

## 2 Log-Landschaft

| Log | Pfad | Wer schreibt |
|---|---|---|
| Hauptlog | `SAOMMOnew/Saved/Logs/SAOMMOnew.log` | Editor-Boots, MCP, Headless-Commandlets |
| Automation-Lauf | `%LOCALAPPDATA%\Temp\opencode\automation\<Stamp>-<Tag>.log` | `Scripts/RunAutomation.ps1` (`-abslog=`, Zeile 49) |
| stdout/Stderr | derselbe Pfad, Endung `.log.out` / `.log.err` | RedirectStandardOutput/Err (Zeile 51-52) |
| Gesamturteil | `<Stamp>-urteil.txt` | Stufen 0-4 (Zeile 182) |

Tags: `stufe0-build`, `stufe1-list`, `stufe2-runtests`, `stufe3-boot`.

## 3 Erfassungsumfang Stufe 4 (RunAutomation.ps1 Zeilen 126-169)

Gescannt werden nur **zwei** Logs:
* `stufe3-boot.log` ab Zeile 1,
* `stufe2-runtests.log` ab der Zeile `Cmd: Automation RunTests` (Scope-Slicing,
  Kommentar Zeile 138-142).

Match ist der Substring `: Error`; Whitelist-Abgleich ist ein case-sensitiver
Substring ueber die ganze Zeile (`$E.Line.Contains($W)`).

NICHT gescannt: `stufe0/1`, alle `.out`/`.err`, der Hauptlog
`SAOMMOnew.log`, alle `: Warning`-Zeilen und Tracebacks. Zwei Fehlerklassen
konnten dadurch lange unsichtbar bleiben (Abschnitt 5).

## 4 Bestandsaufnahme (Referenzlauf 20261006-234914, GRUEN; Hauptlog-Stand 2026-10-07)

| Klasse | Vorkommen | Severity | Herkunft | Status |
|---|---|---|---|---|
| `LogPython: Error` Traceback `PythonTestRunner` fehlt | 5 je Boot | Error | Engine `ToolsetRegistry/init_unreal.py`, Headless-Quirk | whitelisted (#22, R16: AIModuleToolset ueber .uproject auf false) |
| `UnifiedErrorTest` LogTemp-Zeilen + 17x `Condition failed` | je Boot | Error | Engine-Selftest der Unified-Error-API | LogTemp whitelisted; 17x liegen VOR dem Scope-Slice und werden nie gescannt |
| `LogHMD` XR-Extensions nicht verfuegbar / OpenXR-Runtime | 6 je Log | Warning | Headless ohne HMD | Engine-Rauschen, kein Ticket noetig |
| `LogEditorDataStorageUI` widget factory purpose 0 | 14 je Boot | Warning | UE-5.8 Editor | Engine-Rauschen |
| `LogModelContextProtocol: Error: Call to unknown method "resources/templates/list"` | 4x Hauptlog | Error | Client `mcp-remote` (globale OpenCode-Config `~/.config/opencode/opencode.json`, Port 8000) gegen Engine-Plugin, das nur `resources/list` + `resources/read` implementiert (`ModelContextProtocolServer.cpp:571-575` weist unbekannte Methoden bewusst ab) | KEIN Projektcode (Repo-Grep 0 Treffer); unschaedlich; sichtbar nur, weil der Hauptlog nicht gescannt wird |
| `LogSpawn/LogAutomationController: Warning: UWorld::DestroyActor: World has no context!` (Enemy_0, Enemy_1, ItemPickup_0) | 4x stufe2, davon 2x im Scanbereich | Warning | Projekt-Testklassen (SAOMMOnew-Enemies/ItemPickup) | ECHTER BEFUND: Aufruf aus/logisch ausserhalb des Weltkontextes — neue Backlog-Zeile #42 |
| `LogCrowdFollowing: Unable to find RecastNavMesh instance` | 3x Hauptlog | Warning | CrowdManager ohne NavMesh in Temp-Welten | siehe Abschnitt 5 |
| `LogHttp` Timeout google.com/generate_204, datarouter-POST | je 1x | Warning | Netzwerk/Telemetrie der Testumgebung | Rauschen |
| `LogLayoutService` UnrealEd_Layout v1.5/v1.6 | 1x | Warning | Editor-Layout-Quirk | Rauschen |
| `LogModelContextProtocol` Licensed-Technology-Hinweis | 1x | Warning | beabsichtigte Meldung (Regel 5, MCP bewusst aktiv, 5e89f5d) | Absicht |

Stufe-4-Urteil des Referenzlaufs: `Logs=2, Error-Zeilen(nach Scope)=8,
Whitelist=2, Verstoesse=0` — alle 8 Treffer sind die dokumentierten
`LogPython`-Zeilen.

## 5 Whitelist-Status und Doku-Korrektur

Aktiv (2 Eintraege, inhaltlich korrekt):
* `LogPython: Error`
* `UnifiedErrorTest`

Korrektur in `Scripts/log-whitelist.txt`: Die Notiz
`Unable to find RecastNavMesh -> gefixt f905d99 (#17)` war zu pauschal.
Der Commit-Beleg (f905d99) weist 0x Vorkommen im **-game-Smoke** aus
(vorher 16x pro Boot), die Severity-Stufe ist dort nicht ausgewiesen; als
**Warning** taucht die CrowdFollowing-Zeile weiterhin 3x im Hauptlog auf.
Damit bleibt die Regel erhalten: regressionswirksam ist nur die
Error-Variante (die Stufe 4 sieht eh nur Errors) — die Warning-Variante ist
als Rest dokumentiert, nicht als behoben.

## 6 Offene Punkte (als Backlog fortgefuehrt, hier nur dokumentiert)

1. Stufe-4-Scope: Warnungen, `.out`-Dateien und der Hauptlog werden nicht
   gezaehlt — Fehlerklassen wie `resources/templates/list` bleiben dadurch
   unsichtbar (Erweiterung als eigene Zeile, muss GRUEN-Laeufe nicht brechen).
2. `DestroyActor: World has no context!` bei Enemy/ItemPickup im Testlauf
   (Backlog #42).
3. MCP-Client-Rauschen `resources/templates/list`: Abhilfe nur clientseitig
   (OpenCode-Config) oder engineitig — ausserhalb des Projektscopes.

## 7 Verifikation (Band 6 §20)

* kompiliert: nicht zutreffend (kein Code)
* Tests: nicht neu ausgefuehrt (kein Code; Baseline unveraendert found=12)
* keine Regressionen: Aenderung nur an Doku + Kommentarzeilen
* Dateien im Scope: genau 2 (diese Datei, `Scripts/log-whitelist.txt` Kommentar)
* Doku aktuell: dieses Dokument IST die Doku
* Git: ein Commit, Push, Message nennt Verifikationsstand
