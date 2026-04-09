Transform Arena - Programming Assignment
Breda University of Applied Sciences - CMGT

A roguelike arena game inspired by Soul Knight. Transform between
three shapes, clear procedurally generated rooms, defeat bosses,
buy items from shops, and collect buffs across 3 floors.

Controls:
  WASD / Arrow keys  -  Move
  1                  -  Transform to Circle (or buy item 1 in shop)
  2                  -  Transform to Triangle (or buy item 2 in shop)
  3                  -  Transform to Square (or buy item 3 in shop)
  Mouse              -  Aim (Triangle form)
  Space              -  Attack / Use ability
  W/S on title       -  Select menu option
  Enter              -  Confirm selection
  R                  -  Restart (after game over)
  ESC                -  Quit

Game Features:
  - 3 Floors with procedurally generated room layouts
  - Maps get bigger each floor (6/7/8 rooms)
  - Minimap with icons: B for boss, $ for shop
  - Room walls with tile patterns, door indicators
  - Boss fight at the end of each floor
  - Shop rooms with buyable items (health, stats)
  - Coin system: earn coins from kills, spend in shops
  - Armor system: absorbs hits, regenerates over time
  - Choose from 3 buffs after beating each boss (max 3)
  - 6 different buffs: speed, damage, health, cooldowns
  - Save/load system with continue option
  - High score tracking across sessions
  - Floating damage numbers
  - Cooldown indicator
  - Game over stats (kills, floor, time, coins)

Enemy Types:
  - Chaser (red circle) - fast, rushes player
  - Shooter (yellow diamond) - ranged, fires projectiles
  - Brute (purple square) - slow, tanky, charges when close
  - Boss variants: larger, 5x health, red glow

Shape Abilities:
  Circle  - Dash through enemies with i-frames
  Triangle - Ranged attack aimed at mouse cursor
  Square  - Ground pound slam with knockback

Built with SFML 2.6.2 and Visual Studio 2022.
