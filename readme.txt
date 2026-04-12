Transform Arena - Programming Assignment
Breda University of Applied Sciences - CMGT

A roguelike arena game inspired by Soul Knight. Transform between
three shapes, clear procedurally generated rooms, defeat bosses,
buy weapons from shops, and collect buffs across 3 floors.

Controls:
  WASD / Arrow keys  -  Move
  1                  -  Transform to Circle (or buy weapon 1 in shop)
  2                  -  Transform to Triangle (or buy weapon 2 in shop)
  3                  -  Transform to Square (or buy weapon 3 in shop)
  Mouse              -  Aim (Triangle form)
  Space              -  Attack / Use ability
  E                  -  Use equipped weapon special move
  ESC                -  Pause / Resume
  W/S on title       -  Select menu option
  Enter              -  Confirm selection
  R                  -  Restart (after game over)

Game Features:
  - 3 Floors with procedurally generated room layouts
  - Maps get bigger each floor (6/7/8 rooms)
  - Minimap with icons: B for boss, $ for shop
  - Room walls with tile patterns, door indicators
  - 3 unique bosses: Guardian, Phantom, Hive
  - Each boss requires different form strategy to defeat
  - Weapon shop: buy weapons with unique special moves
  - 5 weapons: Flame Ring, Frost Shard, Thunder Strike,
    Shadow Dash, Barrier Shield
  - Equip 1 weapon at a time, press E to activate
  - Coin system: earn coins from kills, spend in shops
  - Armor system: absorbs hits, regenerates over time
  - Choose from 3 buffs after beating each boss (max 3)
  - 6 different buffs: speed, damage, health, cooldowns
  - Save/load system with continue option
  - High score tracking across sessions
  - Floating damage numbers
  - EFFECTIVE! indicator with 1.5x bonus for matching forms
  - Cooldown indicator
  - Pause menu with ESC
  - Procedural sound effects (no external audio files)
  - Slow-motion death effect with zoom
  - Game over stats (kills, floor, time, coins, weapon)

Enemy Types:
  - Chaser (red circle) - fast, rushes player
  - Shooter (yellow diamond) - ranged, fires projectiles
  - Brute (purple square) - slow, tanky, charges when close
  - Dasher (orange triangle) - charges at the player
  - Shielder (blue pentagon) - frontal shield, slam to break
  - Boss variants: unique mechanics per floor

Shape Abilities:
  Circle  - Dash through enemies with i-frames
  Triangle - Ranged attack aimed at mouse cursor
  Square  - Ground pound slam with knockback

Weapons (buy from shop, press E to use):
  Flame Ring      - Fire burst around you, damages nearby enemies
  Frost Shard     - 3 piercing ice projectiles in a spread
  Thunder Strike  - Zap 3 nearest enemies with lightning
  Shadow Dash     - Teleport forward leaving a damage trail
  Barrier Shield  - Block all projectiles for 3 seconds

Sound Effects:
  All sounds are procedurally generated using SFML Audio.
  Sine waves, noise bursts, and frequency sweeps create
  hit, dash, shoot, slam, coin, portal, and death sounds
  without any external audio files.

Built with SFML 2.6.2 and Visual Studio 2022.
