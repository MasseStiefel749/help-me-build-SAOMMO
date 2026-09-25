# Player-String-Inventory — DE/EN (localization preparation)

**Status:** Inventory — the DE column contains **proposals**, not approved canon
**Authority:** Band 7 §11 (bilingual, canonical meaning is the source of truth), Band 6 §19 (no invented facts), Band 7 §12 (no invented lore)
**Date:** 2026-09-25 (Dauerbetrieb R25, backlog #32)
**Purpose:** single source for the future `NSLOCTEXT`/`FText` migration of every player-facing string in the SAOMMO game code.

---

## 0. Rules applied (Band 7 §11)

* German = primary author language; English = working language for shared documentation; both versions must convey **identical meaning**; when in doubt the **canonical meaning** is the source of truth.
* `EN (code)` below = what the game shows today (verified by grep, 2026-09-25).
* `DE (proposal)` = proposed German rendering — **UI strings only, no lore content**, still requires author approval before it lands in code.
* Scope: SAOMMO game code HUD (ADR-010 kill-reward HUD, ADR-016 creator, Band 3 §11 death screen).
* Out of scope: `Variant_*` / `Legacy_*` template UIs (template code, deletion candidates #28), editor-only `UMETA(DisplayName)` (never player-visible), `UE_LOG` strings (developer log = English by convention), save-slot names (`CharacterSave`/`PlayerSave` = technical identifiers).

---

## 1. Code-HUD strings (all player-visible literals in `Source/SAOMMOnew`)

Count: **33 table rows — 26 EN-only translatable strings, 7 language-neutral or pure-format rows**. **Zero German strings exist in the code today** (grep for umlauts: no hits).

### A. Death screen — `UI/DeathScreenWidget.cpp` (Band 3 §11)

| Key | Location | EN (code) | DE (proposal) | Status |
| --- | --- | --- | --- | --- |
| `death.title` | L29 | `YOU DIED` | `DU BIST GESTORBEN` | code-EN only |
| `death.respawn` | L34, L65 (set twice) | `PRESS ANY KEY TO RESPAWN` | `BELIEBIGE TASTE DRÜCKEN ZUM WIEDERAUFSTARTEN` | code-EN only |

### B. Character creator — `UI/SAOCharacterCreatorWidget.cpp` (ADR-016)

| Key | Location | EN (code) | DE (proposal) | Status |
| --- | --- | --- | --- | --- |
| `creator.title` | L490 | `CHARACTER CREATOR` | `CHARAKTER-ERSTELLER` | code-EN only |
| `creator.name_hint` | L512 (hint) | `Hero` | `Held` | code-EN only; same default as `SAOCharacterData.h:87` |
| `creator.name` | L513 | `Name` | `Name` | identical in both languages |
| `creator.body` | L516 | `Body` | `Körper` | code-EN only |
| `creator.skin` | L518 | `Skin color` | `Hautfarbe` | code-EN only |
| `creator.hair` | L522 | `Hair color` | `Haarfarbe` | code-EN only |
| `creator.eye` | L526 | `Eye color` | `Augenfarbe` | code-EN only |
| `creator.confirm` | L533 | `CONFIRM` | `BESTÄTIGEN` | code-EN only |
| `creator.cancel` | L534 | `CANCEL` | `ABBRECHEN` | code-EN only |
| `creator.body.manny` | L45 | `Manny (Male)` | `Manny (männlich)` | proper name kept; parenthetical translated |
| `creator.body.quinn` | L46 | `Quinn (Female)` | `Quinn (weiblich)` | proper name kept; parenthetical translated |

### C. Player HUD — `UI/PlayerHudWidget.cpp` (ADR-010)

| Key | Location | EN (code) | DE (proposal) | Status |
| --- | --- | --- | --- | --- |
| `hud.crosshair` | L41 | `+` | — | language-neutral, no key needed |
| `hud.equipment_title` | L67 | `EQUIPMENT  (E / I to close)` | `AUSRÜSTUNG  (E / I schließt)` | code-EN only |
| `hud.no_character` | L166 | `(no character)` | `(kein Charakter)` | code-EN only |
| `hud.body_levels` | L180 (format) | `Lv %d   XP %.0f / %.0f` | unchanged (abbreviation, neutral) | numeric format |
| `hud.body_hp` | L181 (format) | `HP %.0f / %.0f` | unchanged (`HP` neutral) | numeric format |
| `hud.weapon_sword` | L185 (format) | `Weapon: Sword  (DMG %.0f)` | `Waffe: Schwert  (SCHADEN %.0f)` | code-EN only |
| `hud.weapon_none` | L189 | `Weapon: -` | `Waffe: -` | code-EN only |
| `hud.inventory_title` | L192 | `--- Inventory ---` | `--- Inventar ---` | code-EN only |
| `hud.item_row` | L204 (format) | `%s  x%d` | unchanged — name comes from `Item.DisplayName` (FText) | numeric format |
| `hud.inventory_empty` | L209 | `(empty — kill enemies, grab pickups)` | `(leer — besiege Gegner, sammle Beute)` | code-EN only |
| `hud.control_hints` | L212 | `WASD move · Mouse look · LMB attack · E interact · V camera · Shift sprint · I inventory` | `WASD Bewegen · Maus Umsehen · LMB Angreifen · E Interagieren · V Kamera · Shift Sprinten · I Inventar` | code-EN only |
| `hud.hp_label` | L225 (format) | `HP %.0f / %.0f` | unchanged (`HP` neutral) | numeric format |
| `hud.level_label` | L229 (format) | `Lv %d  XP %.0f` | unchanged (abbreviation, neutral) | numeric format |
| `hud.focus` | L233 (format) | `[E] %s` | unchanged — focus name comes from the actor | key hint neutral |

### D. On-screen messages (`GEngine->AddOnScreenDebugMessage`, player-visible)

| Key | Location | EN (code) | DE (proposal) | Status |
| --- | --- | --- | --- | --- |
| `msg.hit` | `Combat/CombatComponent.cpp:209` | `Hit %s (%.0f)` | `Treffer %s (%.0f)` | code-EN only; candidate to fold into `FloatingCombatText` (ADR-010) during migration |
| `msg.interact_focus` | `Interaction/InteractionComponent.cpp:100` | `Interact: %s` | `Interagieren: %s` | code-EN only |
| `msg.interact_none` | `Interaction/InteractionComponent.cpp:105` | `Interact: nothing in reach` | `Nichts in Reichweite` | code-EN only |
| `msg.picked_up` | `World/ItemPickup.cpp:97` (format) | `Picked up: %s x%d` | `Aufgesammelt: %s x%d` | code-EN only |
| `msg.checkpoint` | `World/Checkpoint.cpp:68` | `Checkpoint reached` | `Checkpunkt erreicht` | code-EN only; **glossary gap**: `terminology.md` has no entry for checkpoint — add after approval |

### E. Item names

| Key | Location | EN (code) | DE (proposal) | Status |
| --- | --- | --- | --- | --- |
| `item.health_herb` | `World/ItemPickup.cpp:29` | `Health Herb` | `Heilkraut` | already `FText`, but set via `FText::FromString` (no localization namespace) → switch to `NSLOCTEXT` at migration |

### F. Language-neutral (documented, no key)

* `World/FloatingCombatText.cpp:22` — `%.0f` damage numbers (digits only).
* Crosshair `+` (see table C).
* Save slots, object names, log strings — technical identifiers (rule 0).

---

## 2. Existing DE/EN document pairs — meaning check (Band 7 §11)

### `Docs/World/lore/lore-anchor.{de,en}.md`

Compared paragraph by paragraph on 2026-09-25: **all three sections convey identical meaning** (civilization/fall paragraph, reconstruction/artifacts paragraph, the closing address, plus the three usage bullets). Two observations, documented, **nothing changed** (Band 7 §10: never silently change existing lore):

1. DE `geübten Schwerkämpfern` ↔ EN `skilled swordsmen` — EN narrows the term to swordsmen. Consistent with the blade-culture canon, so no action; recorded because §11 demands identical meaning.
2. DE addresses the player as `Du`/`Deinen` (capitalized, epistolary style) ↔ EN `you` (neutral). Author style in the canonical DE text — kept as is.

Authority note: both files call themselves "canonical (in their language)". Per Band 7 §11 the **meaning** is canonical and German is the primary author language — neither file sits above the other alone. Recorded here so future agents do not "resolve" this by picking a side.

### `Docs/World/terminology.md`

Already bilingual where it matters: Core Terms (6/6 EN+DE) and Faction Short Names (4/4 EN+DE) ✓. **Gap:** Place Types (5 rows) and Threat Tiers (5 rows) are EN-only. These are AI/documentation terms (working language EN is §11-legal); DE candidates for a future approval pass:

| Term (EN) | DE candidate (proposal) |
| --------- | ----------------------- |
| Safe zone | Sichere Zone |
| Training grounds | Trainingsgelände |
| Wilderness border | Wildnisgrenze |
| Ruin / dungeon | Ruine / Dungeon |
| Safe / Low / Medium / High (threat) | Sicher / Gering / Mittel / Hoch |

### EN-only documents (no action needed, §11-conform)

`open-questions.md`, `test-arena-spec.md`, `regions/01-starting-reach.md`, `factions/initial-factions.md`, `Docs/AI/*`, `Content/World/StartingReach/README.md` — shared documentation = English working language by definition. The lore anchor pair is the only narrative text that must exist bilingually.

---

## 3. Migration path (preparation only — no code changed in this row)

1. Replace every literal in section 1 with `NSLOCTEXT("SAOMMO", "<key>", "...")` (or `LOCTEXT` per module); `FText::FromString(TEXT(...))` is **not** localizable — including `Item.DisplayName`.
2. Ship cultures: decision for the author (which cultures beyond `en`/`de` are supported) — before any `.po`/translation work.
3. German strings are up to ~40 % longer (`death.respawn`, `hud.control_hints`) → visual F5 check needed after migration (Simon).
4. Acceptance proposal for the migration row: grep over `Source/` shows zero player-visible `TEXT()` literals outside this inventory's neutral list; every key resolves in both cultures.
5. This row (#32) delivers the inventory only; the C++ migration is its own future row with Build.bat proof (ADR-017b).

---

## 4. Re-run recipe

```text
grep over Source/SAOMMOnew for: FText::FromString(TEXT(" · AddOnScreenDebugMessage · SetText( · MakeLabel( · MakeButton(
compare against this table — new player-visible strings must get a row here first (Band 7 §11).
```
