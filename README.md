# Transform Arena

A top-down, **Souls-inspired roguelike arena** built from scratch in **C++17** with **SFML 2.6** — **no game engine**.

![Library](https://img.shields.io/badge/Library-SFML_2.6-8CC445?style=flat-square&logo=sfml&logoColor=white)
![Language](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D6?style=flat-square&logo=windows&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-blue?style=flat-square)
![Status](https://img.shields.io/badge/Status-Shipped-brightgreen?style=flat-square)

## Overview

A personal project that pairs the **“Transform”** intake theme with combat that rewards reading enemy types and **switching forms at the right moment**. You clear procedurally stitched floors, spend coins on weapons, and learn three bosses that each punish the wrong shape — the same loop-driven mindset I use in my Unreal melee prototype, scaled down to 2D vector art and a single codebase you can read end-to-end.

> Intake piece for **BUas · Creative Media and Game Technologies (CMGT)** — Bachelor’s programme, **2026–27** intake.  
> Same author as [**Medieval Combat Game**](https://github.com/KevinRusev/MedivalCombatGame) (UE5 / C++) — different stack, same focus on **feel**, **clarity**, and **systems you can extend**.

## Highlights

**Player — three forms, one scroll wheel**

- **Circle** · fast · dash through enemies with i-frames  
- **Triangle** · ranged · mouse-aimed shots  
- **Square** · tank · ground-pound slam with knockback and shield-breaking utility  
- Form **matchups**: effective hits (**1.5×**), resisted on later floors (**0.5×**) — forces real-time decisions, not idle DPS  
- Buffs stack (up to **3**) after each boss; weapons add a second layer of **E**-activated specials

**Enemies & bosses**

- Five regular archetypes (chaser, shooter, brute, dasher, shielder) with distinct movement and pressure  
- **The Guardian** — frontal shield; **Square**’s slam shatters it  
- **The Phantom** — teleports; **Circle**’s dash deals bonus damage and stuns  
- **The Hive** — minion economy + ring patterns; **Square** clears crowds, **Triangle** picks the core  

**World & progression**

- Procedural **floor graph** (backtracking walker + branch rooms + guaranteed shop)  
- Room **obstacle sets** — hand-authored layouts + random fallback, doors stay passable  
- **Shops**: **8** weapons with unique effects (vortex, barrier, chain lightning, clone, etc.)  
- **Save / load** with **Continue** on the title screen; high-score fields persist across runs  

**Audio & presentation**

- **Procedural SFX** — sine sweeps and noise bursts into `sf::SoundBuffer`; **no WAV/MP3 assets**  
- Screen shake, particles, slow-motion death / zoom, letterboxed **800×600** view for any window size  
- Pause, tutorial overlay, fullscreen **F11**

## Gameplay clip

[![Watch on YouTube](https://img.shields.io/badge/Watch-Gameplay-red?style=for-the-badge&logo=youtube)](YOUR_YOUTUBE_LINK_HERE)

| Form | Colour | Ability | Strong vs |
| --- | --- | --- | --- |
| **Circle** | Blue | Dash (i-frames) | Chasers |
| **Triangle** | Orange | Mouse-aim shot | Shooters |
| **Square** | Green | Ground-pound | Brutes |

## Tech

- **C++17** · SFML **2.6** (graphics, window, audio) — **static** link (`SFML_STATIC`) + bundled `.lib` / `.dll`  
- **Visual Studio 2022** · x64 **Debug** and **Release**  
- Procedural **vector** rendering (`sf::CircleShape`, `sf::ConvexShape`, `sf::RectangleShape`) — no texture pipeline  
- Plain-text **savegame** next to the executable · zlib-licensed SFML headers in `SFML/` (see `SFML/license.md`)

Roughly **3.5k lines** across seven translation units + headers.

## Project layout

```text
TransformArena/
├── src/
│   ├── main.cpp           Entry: window, events, F11 fullscreen
│   ├── Game.{h,cpp}       Loop, maps, HUD, items, save/load, buffs, portals
│   ├── Player.{h,cpp}     Movement, forms, abilities, multipliers
│   ├── Enemy.{h,cpp}      Grunts + Guardian / Phantom / Hive
│   ├── Room.{h,cpp}       Tiles, doors, shops, obstacle layouts
│   ├── Projectile.h       Bullets + trails
│   ├── Particle.h         Particles, floaty damage text, pickups
│   ├── Item.h             Eight shop weapons & icon drawing
│   ├── SaveData.h         Key=value persistence + highs
│   └── Sound.h            Procedural buffers
├── SFML/                  Headers, static libs, runtime DLLs (vendored)
├── TransformGame.sln
├── TransformGame.vcxproj
├── readme.txt             Controls & features (assignment hand-in)
├── LICENSE                MIT (game code; SFML stays under its own license)
└── README.md              This file
```

## Build

```powershell
git clone https://github.com/KevinRusev/TransformArena.git
cd TransformArena
start TransformGame.sln
```

1. Configuration **Release | x64** (or **Debug | x64**).  
2. **Build → Build Solution** (`Ctrl + Shift + B`).  
3. **Debug → Start Without Debugging** (`Ctrl + F5`).

The post-build step copies **openal32.dll** into the output folder.

## Requirements

- **Windows 10 / 11**  
- **Visual Studio 2022** with **Desktop development with C++**  
- **No extra downloads** — SFML is in the repo

## Controls

| Action | Input |
| --- | --- |
| Move | `WASD` / arrows |
| Transform | Mouse wheel |
| Aim (Triangle) | Mouse |
| Ability | `Space` |
| Weapon special | `E` |
| Shop buy | `1` / `2` / `3` |
| Pause | `Esc` |
| Fullscreen | `F11` |
| Restart (game over) | `R` |

## License

**Game source** — **MIT**. See [`LICENSE`](LICENSE).

**SFML** ships under the **zlib/libpng** license — full text in [`SFML/license.md`](SFML/license.md).

---

### Kevin Rusev

Computer programming student · **Bulgaria** · gameplay in **UE5 (C++)** and products on the web.

**Languages**  
![C++](https://img.shields.io/badge/-C%2B%2B-00599C?logo=cplusplus&logoColor=white&style=flat-square)
![Python](https://img.shields.io/badge/-Python-3776AB?logo=python&logoColor=white&style=flat-square)
![JavaScript](https://img.shields.io/badge/-JavaScript-F7DF1E?logo=javascript&logoColor=black&style=flat-square)
![TypeScript](https://img.shields.io/badge/-TypeScript-3178C6?logo=typescript&logoColor=white&style=flat-square)

**Stack**  
![Unreal Engine](https://img.shields.io/badge/-Unreal_Engine_5-313131?logo=unrealengine&style=flat-square)
![SFML](https://img.shields.io/badge/-SFML-8CC445?logo=sfml&logoColor=white&style=flat-square)
![React](https://img.shields.io/badge/-React-61DAFB?logo=react&logoColor=black&style=flat-square)

**Links**  
[![GitHub](https://img.shields.io/badge/GitHub-KevinRusev-181717?logo=github&style=flat-square)](https://github.com/KevinRusev)
[![Profile README](https://img.shields.io/badge/Profile-README-181717?logo=github&style=flat-square)](https://github.com/KevinRusev/KevinRusev)
[![Medieval combat (UE5)](https://img.shields.io/badge/Portfolio-Medieval_Combat-313131?logo=unrealengine&style=flat-square)](https://github.com/KevinRusev/MedivalCombatGame)
[![Email](https://img.shields.io/badge/Email-kevinrusev1%40gmail.com-red?style=flat-square&logo=gmail)](mailto:kevinrusev1@gmail.com)

**Also see** · [**Medieval Combat Game**](https://github.com/KevinRusev/MedivalCombatGame) — third-person Souls-style combat in Unreal Engine 5 (**C++ only** for combat, AI, and core systems).
