# Transform Arena

A top-down roguelike arena shooter built from scratch in **C++17** with **SFML 2.6** — no game engine.

The "Transform" theme is mechanical: switch between three forms mid-combat using the scroll wheel. Each form has unique stats, an exclusive ability, and a different damage matchup against enemies, so picking the right shape at the right moment is the core decision of every fight.

> Built as the programming intake assignment for the BUas CMGT Bachelor's programme (intake 2026-27).

---

## Gameplay

[![Watch on YouTube](https://img.shields.io/badge/Gameplay-YouTube-red?logo=youtube)](YOUR_YOUTUBE_LINK_HERE)

Click the badge above for a full gameplay video.

| Form | Color | Ability | Strong vs |
| --- | --- | --- | --- |
| **Circle** | Blue | Dash with i-frames | Chasers |
| **Triangle** | Orange | Mouse-aimed shot | Shooters |
| **Square** | Green | Ground-pound slam | Brutes |

---

## Features

- **3 procedural floors** of connected rooms, each larger than the last (5/6/7 grid)
- **3 unique bosses**, each requiring a different form to defeat
  - *The Guardian* — shielded; only Square's slam breaks the shield
  - *The Phantom* — teleports; Circle's dash deals bonus damage and stuns
  - *The Hive* — endless minions; Square's slam clears them
- **8 weapons** sold in shops, each with a unique special move
  (Flame Ring, Frost Shard, Thunder Strike, Shadow Dash, Barrier Shield, Vortex Pull, Mirror Clone, Chain Lightning)
- **Form matchup system** — 1.5x damage on effective forms, 0.5x when resisted (floor 2+)
- **Buff selection** — pick one of three random buffs after each boss; up to 3 stack
- **Save / load** with a Continue option from the title menu
- **Procedural sound effects** — sine waves and noise bursts synthesised at runtime, no audio files
- **Procedural map generation** with backtracking walker, branch rooms and forced shop placement
- **Procedural obstacle layouts** — eight hand-tuned patterns plus a random fallback
- 5 enemy types with distinct AI (Chaser, Shooter, Brute, Dasher, Shielder)
- Polish: screen shake, particle effects, slow-motion death with zoom, fullscreen with letterboxing, pause menu, in-game tutorial, high-score tracking, animated title screen

---

## Controls

| Action | Key |
| --- | --- |
| Move | `WASD` / Arrow keys |
| Switch form | Mouse scroll wheel |
| Aim (Triangle form) | Mouse |
| Use ability | `Space` |
| Use equipped weapon | `E` |
| Buy weapon in shop | `1` / `2` / `3` |
| Pause / Resume | `Esc` |
| Toggle fullscreen | `F11` |
| Restart after Game Over | `R` |

---

## Build

Requires **Visual Studio 2022** (Community is fine) with the *Desktop development with C++* workload installed. SFML is bundled in the repo, so no extra downloads are needed.

```powershell
git clone https://github.com/KevinRusev/assignment.git
cd assignment
start TransformGame.sln
```

In Visual Studio:
1. Set the configuration to **Release | x64** (or Debug | x64).
2. Hit **Build → Build Solution** (`Ctrl + Shift + B`), then **Debug → Start Without Debugging** (`Ctrl + F5`).

The build copies `openal32.dll` next to the executable automatically.

---

## Project structure

```
src/
├── main.cpp        Window, event loop, fullscreen toggle
├── Game.{h,cpp}    State machine, floor generation, HUD, item effects, save/load
├── Player.{h,cpp}  Movement, transform logic, abilities, multipliers
├── Enemy.{h,cpp}   Regular AI + 3 unique boss behaviours
├── Room.{h,cpp}    Room types, doors, obstacle layouts, shop UI
├── Projectile.h    Header-only struct with trail rendering
├── Particle.h      Particles, damage numbers, health pickups
├── Item.h          8 weapons with stat tables and vector icons
├── SaveData.h      Plain-text save with high-score tracking
└── Sound.h         Procedural audio generation (sine + noise)

SFML/               Bundled SFML 2.6 (headers + static libs + DLLs)
TransformGame.sln   Visual Studio 2022 solution
```

Roughly **3,500 lines of C++** across nine source files.

---

## Tech notes

- Statically linked SFML — defines `SFML_STATIC` and links `sfml-*-s[-d].lib` plus the OpenAL/FLAC/Vorbis/Ogg/Freetype dependencies that ship with SFML.
- All artwork is generated with `sf::CircleShape`, `sf::ConvexShape` and `sf::RectangleShape` at runtime; the only external asset is the system Arial font (with a graceful fallback if it isn't found).
- All sound effects are synthesised at startup into `sf::SoundBuffer` samples, so the game runs with zero audio files on disk.
- Rendering uses a fixed 800×600 letterboxed `sf::View` so the game scales cleanly to any window or fullscreen resolution without distortion.

---

## License

MIT — see [LICENSE](LICENSE).

The project bundles SFML 2.6, which is released under the zlib/libpng license. See `SFML/license.md`.

---

## Author

**Kevin Rusev** — [github.com/KevinRusev](https://github.com/KevinRusev)
