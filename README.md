# Transform Arena

A roguelike arena game built with C++ and SFML for the CMGT Programming assignment at Breda University.

## Theme: Transform
Switch between Circle, Triangle and Square forms mid-combat. Each form has unique stats and abilities. Matching the right form to an enemy type deals bonus damage.

## Features
- 3 floors of procedurally generated rooms with boss fights
- 3 unique bosses (Guardian, Phantom, Hive) each requiring different strategies
- 5 equippable weapons with special abilities
- Coin economy and weapon shops
- Buff selection system after each boss
- Save/load with continue support
- Procedural sound effects generated at runtime
- Pause menu, slow-motion death effect, damage numbers

## How to Build
1. Open `TransformGame.sln` in Visual Studio 2022
2. Set configuration to Release x64
3. Build and run (Ctrl+F5)

Requires SFML 2.6.2 (included in `lib/` and `include/` folders).
