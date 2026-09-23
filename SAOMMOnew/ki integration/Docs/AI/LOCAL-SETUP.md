# LOCAL-SETUP — SAOMMO Local AI on this machine

Hardware: RTX 4070 12GB (single GPU; RTX 3060 12GB planned) + Ryzen 7 7700X + 32GB RAM (64GB recommended).
Engine: UE 5.8 live. Repo: `help-me-build-SAOMMO`, branch `going-to-make-an-project`.

## Models (Ollama, on E: — C: stays free)

- `qwen3-coder:30b` (~18GB) — chat/edit/apply (Cline + Continue "SAOMMO Main Developer")
- `qwen2.5-coder:1.5b` (~1GB) — Continue tab-autocomplete + fast questions
- Server: `ollama serve` with `$env:OLLAMA_MODELS="E:\OllamaModels\models"`.
  Desktop App (tray) ignores that path — Quit it, use `E:\SAOMMO\Tools\SAOMMO-AI\Start-ModeA/B.ps1`.
  Autostart at login: `SAOMMO-Ollama-Serve.cmd` in the Windows Startup folder (replaces the removed Ollama.lnk).

## If Cline says "model not found"

1. Run `E:\SAOMMO\Tools\SAOMMO-AI\Fix-Ollama-Server.ps1` (kills duplicate servers, restarts with E: path).
2. Check `ollama list` shows both models.
3. Quit the Ollama Desktop App in the tray if it reappeared, then Retry in Cline.

## Modes

- MODE A (Unreal open): 4070 stays with the editor; only the 1.5B model. Never force 30B next to Nanite/Lumen/VR preview.
- MODE B (Unreal closed): 30B with 16K context (32K after RAM/3060 upgrade).

## VS Code

Continue config: `%USERPROFILE%\.continue\config.yaml` (already wired to both models).
Cline: Provider Ollama, `http://localhost:11434`, `qwen3-coder:30b`, Compact Prompt ON.
This repo adds: `AGENTS.md`, `.clinerules`, `.clineignore`, `.vscode/settings.json` (this folder).

## Git safety (Band 6 §13/14)

Working tree is clean at setup time. Before agent edits: commit checkpoint.
24 local commits are ahead of origin — push when ready, never force-push.
