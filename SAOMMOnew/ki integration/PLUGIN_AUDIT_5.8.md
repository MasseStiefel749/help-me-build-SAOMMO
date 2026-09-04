# Plugin Audit — UE 5.8 (Band 2 §12-13, Band 6 §22, Band 9 Ch.5)

**Date:** 2026-09-02
**File:** `SAOMMOnew.uproject` — `EngineAssociation 5.8`
**Total entries:** 598 plugin entries, **595 Enabled:true** — indicates "enable all" template pollution.

## Summary

The live `.uproject` enables almost every plugin. This violates Band 6 §22 (plugin discipline) and Band 2 technical priority. Many enabled plugins are unused and increase cook size, compile time, and VR overhead.

**Required per design (Band 9 Ch.5 + ADR-005/006/007):**

| Category | Required | Status in .uproject (explicit Enabled:true) |
|----------|----------|---------------------------------------------|
| **VR** | `OpenXR`, `OpenXRHandTracking`, `OpenXREyeTracker`, `OpenXRMsftHandInteraction` | ✅ explicit true (good) |
| **Gameplay** | `EnhancedInput`, `GameplayTags` | ⚠️ **NOT explicit** — relies on engine default (verify in Editor → Plugins, enable if disabled) |
| **AI** | `AIModule` (module dep), `StateTree`, `GameplayStateTree`, `AIModuleToolset`, `Environment Query` (merged into AIModule 5.8) | `StateTree/GameplayStateTree/AIModuleToolset` ✅ true |
| **Animation** | `ControlRig`, `FullBodyIK`, `IKRig`, `IKRetargeter`, `MotionWarping`, `MotionMatching`, `PoseSearch` | ⚠️ NOT explicit — defaults; enable explicitly |
| **Graphics** | `Niagara`, `PCG`, `Water`, `Landmass` | ⚠️ NOT explicit — defaults; enable explicitly |
| **Editor** | `ModelingToolsEditorMode` | ✅ true |
| **UI** | `CommonUI` | ✅ true |

Missing explicit entries are **not a bug** if engine defaults enable them, but for reproducibility they should be added explicitly (see patch below).

## Keep / Disable Recommendation

### KEEP (explicitly enable, add if missing)

```json
{ "Name": "OpenXR", "Enabled": true },
{ "Name": "OpenXRHandTracking", "Enabled": true },
{ "Name": "OpenXREyeTracker", "Enabled": true },
{ "Name": "OpenXRMsftHandInteraction", "Enabled": true },
{ "Name": "EnhancedInput", "Enabled": true },
{ "Name": "EnhancedInputSequence", "Enabled": true },
{ "Name": "GameplayTags", "Enabled": true },
{ "Name": "StateTree", "Enabled": true },
{ "Name": "GameplayStateTree", "Enabled": true },
{ "Name": "AIModuleToolset", "Enabled": true },
{ "Name": "ControlRig", "Enabled": true },
{ "Name": "FullBodyIK", "Enabled": true },
{ "Name": "IKRig", "Enabled": true },
{ "Name": "IKRetargeter", "Enabled": true },
{ "Name": "MotionWarping", "Enabled": true },
{ "Name": "MotionMatching", "Enabled": true },
{ "Name": "PoseSearch", "Enabled": true },
{ "Name": "Niagara", "Enabled": true },
{ "Name": "PCG", "Enabled": true },
{ "Name": "Water", "Enabled": true },
{ "Name": "Landmass", "Enabled": true },
{ "Name": "ModelingToolsEditorMode", "Enabled": true, "TargetAllowList": ["Editor"] },
{ "Name": "CommonUI", "Enabled": true }
```

Add `"Iris"` only when multiplayer needs it (Band 2 §13 — deferred).

### EVALUATE (keep if you use them, else disable)

- `VRM4U`, `Nwiro`, `SteamSAL`, `EasyLocalizationTool`, `PolygonflowContentBrowserPlugin`, `VibeUE` — marketplace; keep only if actively used. Otherwise disable (license/maintenance risk).
- `HPMotionController`, `OpenXRViveTracker`, `PICOController` — keep if testing those headsets.
- `MassAI/MassCrowd/ZoneGraph` — deferred until enemy ecology needs crowd (Band 4 §9). Disable for slice.
- `LearningAgents/LearningCore/MLAdapter/RigLogic` — deferred (no AI training yet). Disable.
- `ChaosCachingUSD/ChaosFlesh/ChaosModularVehicle` — disable unless needed.

### DISABLE (safe for vertical slice, re-enable later if needed)

All of these are Enabled:true today but not required for slice (examples — full list 586 extras):

- Apple*/Android*/IOS* platform media/AR (except your target platform)
- `AlembicHairImporter`, `AppleProResMedia`, `AvidDNxHD`, `BinkMedia`, `BlackmagicMedia`, etc. — media codecs not used
- `Datasmith*`, `Interchange*`, `Shotgrid`, `Avalanche*`, `Concert*` — pipeline
- `Analytics*`, `AutomatedPerfTesting`, `Gauntlet`, `FunctionalTestingEditor` — test infra
- `GameFeatures`, `ModularGameplay` — keep only if using Game Features

**Recommended action:** In Editor → Plugins, filter by category, disable unused. Or apply patch:

## Patch Option (minimal .uproject)

Backup `SAOMMOnew.uproject` → replace `Plugins` array with KEEP list above + add `Enabled: false` removals via Editor. UE regenerates missing entries as defaults-disabled on next launch. **Do not hand-edit to 595 entries** — use Editor UI so dependency graph stays valid.

If you want a one-click clean, create `SAOMMOnew.uproject.minimal` with only KEEP list and rename after backup.

## Verification

- [ ] After disabling: Editor restarts cleanly, `SAOMMOnew` module builds (`Build succeeded`)
- [ ] Packaged build size drops (check `Saved/Cooked`)
- [ ] No missing reference for `BP_SAOMMO*` / `IMC_SAOMMO`

## Commit suggestion (when clean)

```text
Clean SAOMMOnew.uproject plugins to Band 9 minimal set (Band 6 §22)
```
