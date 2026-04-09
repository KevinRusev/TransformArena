Transform Arena - Programming Assignment
Breda University of Applied Sciences - CMGT

A roguelike arena game where you transform between three shapes,
each with unique abilities. Clear rooms, defeat bosses, collect
buffs and fight your way through 3 floors.

Controls:
  WASD / Arrow keys  -  Move
  1                  -  Transform to Circle (fast, agile)
  2                  -  Transform to Triangle (ranged, mouse aim)
  3                  -  Transform to Square (slow, tanky, area damage)
  Mouse              -  Aim (Triangle form)
  Space              -  Attack / Use ability
  W/S on title       -  Select menu option
  Enter              -  Confirm selection
  R                  -  Restart (after game over)
  ESC                -  Quit

Game Features:
  - 3 Floors with procedurally generated room layouts
  - Minimap showing explored rooms
  - Boss fight at the end of each floor
  - Choose from 3 buffs after beating each boss (max 3 equipped)
  - 6 different buffs: speed, damage, health, cooldowns, and more
  - Save/load system with continue option
  - High score tracking across sessions
  - Floating damage numbers
  - Cooldown indicator
  - Game over stats (kills, floor, time)

Enemy Types:
  - Chaser (red circle) - fast, chases player
  - Shooter (yellow diamond) - ranged, fires projectiles
  - Brute (purple square) - slow, tanky, charges when close
  - Boss variants are larger with more health

Shape Abilities:
  Circle  - Dash through enemies with i-frames
  Triangle - Ranged attack aimed at mouse cursor
  Square  - Ground pound slam with knockback

Built with SFML 2.6.2 and Visual Studio 2022.
