Transform Arena - Programming Assignment
Breda University of Applied Sciences - CMGT

A roguelike arena game inspired by Soul Knight. Transform between
three shapes, clear procedurally generated rooms, defeat bosses,
buy weapons from shops, and collect buffs across 3 floors.

Controls:
  WASD / Arrow keys  -  Move
  Mouse scroll       -  Switch form (Circle / Triangle / Square)
  Mouse              -  Aim (Triangle form)
  Space              -  Attack / Use ability
  E                  -  Use equipped weapon special move
  1 / 2 / 3          -  Buy weapon in shop
  ESC                -  Pause / Resume
  F11                -  Toggle fullscreen
  W/S on title       -  Select menu option
  Enter              -  Confirm selection
  R                  -  Restart (after game over)

Game Features:
  - 3 Floors with procedurally generated room layouts
  - Maps get bigger each floor (6/7/8 rooms)
  - Minimap with icons: B for boss, $ for shop
  - Room walls with tile patterns, door indicators
  - Random obstacles/cover blocks in rooms
  - 3 unique bosses: Guardian, Phantom, Hive
  - Each boss requires different form strategy to defeat
  - Form matchup system: effective (1.5x) and resisted (0.5x)
  - Form resistance on floors 2+ forces switching forms
  - Weapon shop: buy weapons with unique special moves
  - 8 weapons with unique effects and visuals
  - Equip 1 weapon at a time, press E to activate
  - Coin system: earn coins from kills, spend in shops
  - Armor system: absorbs hits, regenerates over time
  - Choose from 3 buffs after beating each boss (max 3)
  - Save/load system with continue option
  - High score tracking across sessions
  - Floating damage numbers with trail effects
  - Projectile trails and glow effects
  - Damage flash (red screen tint when hit)
  - EFFECTIVE! and RESISTED! indicators
  - Cooldown indicator
  - Animated title screen with floating shapes
  - In-game tutorial accessible from main menu
  - Victory screen with visual celebration
  - Pause menu with ESC
  - Fullscreen support with F11 (letterboxed)
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
  Flame Ring        - Fire burst around you, damages nearby enemies
  Frost Shard       - 3 piercing ice projectiles in a spread
  Thunder Strike    - Zap 3 nearest enemies with lightning
  Shadow Dash       - Teleport forward leaving a damage trail
  Barrier Shield    - Block all projectiles for 3 seconds
  Vortex Pull       - Pull all enemies toward you
  Mirror Clone      - Spawn a decoy that draws enemy fire
  Chain Lightning   - Bolt chains between all enemies

Sound Effects:
  All sounds are procedurally generated using SFML Audio.
  Sine waves, noise bursts, and frequency sweeps create
  hit, dash, shoot, slam, coin, portal, and death sounds
  without any external audio files.

Built with SFML 2.6.2 and Visual Studio 2022.

--------------------------------------------------------------------------------
HOW TO BUILD (submission reviewers)
--------------------------------------------------------------------------------
1. Extract this zip to a new empty folder (path without spaces recommended).
2. Open TransformGame.sln in Visual Studio 2022 (install "Desktop development
   with C++").
3. Set configuration to Release | x64 (or Debug | x64) and build the solution.
4. Run with Ctrl+F5. The executable is under bin\Release\ or bin\Debug\.
   openal32.dll is copied to the output folder automatically by the project.

SFML is included in the SFML\ folder (headers, .lib, .dll) — no separate download.

--------------------------------------------------------------------------------
ATTRIBUTION — external libraries, tutorials, resources
--------------------------------------------------------------------------------
- SFML 2.6 — Simple and Fast Multimedia Library. https://www.sfml-dev.org/
  Licensed under the zlib/libpng license (see SFML\license.md). Vendored in
  SFML\ for a portable, out-of-the-box build.

- C++ standard library reference: https://en.cppreference.com/

- Official SFML tutorials and documentation:
  https://www.sfml-dev.org/tutorials/2.5/ (and 2.6 API where applicable)

- Prep / C++ refresher linked in the BUas assignment brief:
  https://www.3dgep.com/cpp-fast-track-1-getting-started

- Gameplay feel: loosely inspired by Soul Knight (no code or assets copied).

All graphics are drawn with SFML primitives at runtime. All sound is
synthesised in code (Sound.h); there are no external image or audio files.
