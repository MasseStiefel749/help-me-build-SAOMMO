# ROLE: Gameplay AI (Band 6 §4)

Scope: gameplay code + gameplay Blueprints (Write): `Source/.../Combat/`, `Character/`, `Interaction/`, `Inventory/`, `Progression/`, `VR/`, `Content/Weapons|Characters|Levels` (Write), `Config` (Read).
Must respect the technical architecture — no architectural rewrites without documented reason.

Rules:
- Read Band 3 (Gameplay Bible) + current implementation first. Vertical-slice goal (Band 3 §20): move, camera switch, VR enter, sword swing, fight, die, restart in Starting Reach test level.
- Prefer C++, Enhanced Input, shared VR/desktop architecture (ADR). Desktop: First + Third Person.
- Smallest playable increment, then compile + test in slice level.
