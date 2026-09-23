# SAOMMO — ADR-015: Orientation on Echoes of Aincrad

**Date:** 2026-09-04
**Status:** Active (direction, not scope creep — slice first, Band 1 stands)
**Sources:** Bandai Namco official pages + Steam listing (PS5/Xbox/PC,
UE-based, 10.07.2026) + CGMagazine PC review (7/10, UEVR note).

## 1. What Echoes is (facts)

Single-player Action JRPG: avatar creation (after a Beta prologue),
equipment/weapons/stats builds (speed/int/endurance), special skills,
AI partner with tactics + synergy attacks, real-time melee combat,
quests/treasures/dungeons/bosses across an expanding map, gear upgrades
and weapon crafting from world recipes. Reviewer verdict: decent story,
"fine, nothing spectacular" combat, but **repetitive loop, oversized
empty maps, backtracking**, single-player wearing an MMO costume.

## 2. Head-to-head (us today vs Echoes)

| Pillar | Echoes (shipped) | We (slice) | Read |
|---|---|---|---|
| VR | UEVR injection only | VR-first native (Full VR + desktop + Budget VR later) | **Our edge — keep** |
| Multiplayer | none (pretends) | deferred MMO, real trajectory | **Our edge — keep** |
| Sword combat | button skills, fine | physical swing + velocity arming | **Our edge — deepen** |
| Avatar/identity | full creator | none (fixed Manny/Quinn) | **Adopt (slim)** |
| Builds/stats | STR-like paths | XP/levels only | **Adopt (slim)** |
| Sword Skills | unlockable specials | none | **Adopt (one skill)** |
| Partner | Ito + tactics + combo attacks | none | **Adopt (one companion)** |
| Gear/crafting | upgrades + recipe crafting | loot with no sink (Material type unused!) | **Adopt (upgrade first)** |
| Quests/treasure | present but filler | none | **Adopt (3 hand-made quests)** |
| Bosses | dungeon bosses, gates | none | **Adopt (one arena boss)** |
| World density | vast + empty (reviewer's main complaint) | small slice by design | **Already aligned — weaponize** |
| Story | licensed SAO plot | original Reconstruction lore | **Keep original — never copy names/plot/art** |

## 3. Orientation decisions

### ADOPT (backlog order, each playable before the next — Band 1 rule)

1. **Gear upgrade** (cheapest, uses existing loot): spend materials +
   herbs at a Klingenhof anvil to raise sword DMG +1. Gives Material
   items their first sink and the HUD/equipment screen its first progression.
2. **One Sword Skill**: charged heavy (hold LMB 0.6s → 3x damage sweep
   with distinct swing + message). Proves specials without a skill tree.
3. **Stats build (slim)**: on level-up pick +1 Power (+DMG) or +1 Swiftness
   (+move speed) or +1 Endurance (+max HP). Three stats, real builds,
   equipment screen shows them.
4. **One companion**: a single Blade-Tradition NPC that follows (leash +
   teleport catch-up), attacks the player's target, dies/respawns at
   checkpoints. No tactics UI yet — positioning + focus target only.
5. **Avatar (slim)**: mesh/material variant picker at spawn (Manny/Quinn
   + tint), persisted in the save. Full creator deferred.
6. **Three hand-made quests**: Klingenhof practice (kill 3 with sword),
   herb gathering (collect 5), Ruine teaser (reach the niche). Quest state
   in the save; HUD objective line. Hand-made, never procedural filler.
7. **One arena boss**: oversized enemy in the Ruine approach with HP bar
   (world-space widget), two attacks (slam + charge telegraphs), Artifact
   teaser drop (lore-locked, no power granted in slice).

### EXPLOIT (their weaknesses are our positioning)

- **Density over size**: keep regions small and hand-filled; never scale
  the map to hide missing gameplay (their exact failure).
- **Real multiplayer trajectory** stays the endgame (their costume vs
  our architecture).
- **Native VR** stays the headline (their UEVR is a hack).

### AVOID (hard constraints)

- No SAO names, plot beats (Beta/PKers/Ito/mirror), art, music, or
  terminology. Our Reconstruction lore + German region names stay.
- No season-pass/DLC-shaped planning; open-source single slice first.
- No copying their UI layout pixel-wise (reference vibe only, cf. ADR-014).

## 4. Non-goals confirmed by this comparison

MMO networking, full character creator, skill trees, crafting recipes
beyond upgrade, factions UI, day/night AI — all stay deferred as before.

## Revert

Direction only; no code touched. Drop this ADR to cancel.
