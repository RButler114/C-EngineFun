# Everharvest Voyager V — Arcade Engine + Game

A C++17 side‑scrolling arcade framework and sample game built with SDL2. It uses a clean Entity–Component–System (ECS), state-driven screens, and config‑driven content (levels, combat, sprites). Designed to be easy to run, understand, and extend.

## Highlights

- Fast 60 FPS loop, sprite rendering with animation, simple audio (music + SFX)
- ECS with essential Components (Transform, Velocity, Collision, Audio) and Systems (Movement, Collision, Audio)
- Game States: Menu, Playing, Combat, Pause, Game Over, Customization
- Config‑driven gameplay via INI files: gameplay.ini, characters.ini, and per‑level configs
- Turn‑based “FF‑style” combat triggered on collisions, pausing the arcade loop
- Victory/Lose rules: survive‑timer, defeat boss, defeat‑all, reach end distance (with optional boss+end requirement)
- HUD with timer, score, level, distance progress bar; optional debug overlay

---

## 1) Quick Start

### Prerequisites

- C++17 compiler, CMake ≥ 3.16
- SDL2, SDL2_image, SDL2_mixer installed (e.g., macOS/Homebrew: `brew install sdl2 sdl2_image sdl2_mixer`)

### Build & Run

- One‑liner: `./run_game.sh` (creates build, compiles, runs)
- Manual:
  - mkdir build && cd build
  - cmake ..
  - make -j4
  - ./bin/GameEngine

### Keyboard (during play)

- Move: WASD
- Menu: ESC
- Level shortcuts: 1 (level1), 2, 3, B (boss), 0 (Base/reset overrides)
- Misc: R (reload configs), G (spawn test enemy)

---

## 2) Project Layout

- assets/config
  - gameplay.ini — global gameplay, HUD, colors, audio, [win] defaults
  - characters.ini — player/enemy sprites, stats, templates
  - levels/levelN.ini — per‑level overrides (rules, time, end_distance, next_level, placements)
- include/, src/
  - Engine/: core loop, rendering, input, audio, config loader
  - ECS/: components and systems
  - Game/: states (Playing, Combat, GameOver, etc.) and game logic
- docs/: focused deep‑dives (keybindings, config system)
- test/: unit/integration tests + `test/run_tests.sh`

---

## 3) How Gameplay Works (at a glance)

- PlayingState handles the arcade loop (movement, scrolling, collisions) and draws the HUD
- Colliding with enemies triggers CombatState (turn‑based) and pauses the arcade loop
- On victory/defeat, GameOverState shows outcome and lets you continue, retry, or go to menu
- Level progression is config‑driven via [win] in the active level file

### HUD & Debug

- Top‑left: SCORE and LEVEL label (e.g., level1)
- Top‑right: TIME; bottom‑right bar: distance to end (DIST: current/end)
- Optional debug overlay below HUD: `debug_overlay=true` to show time, distance, required rules, boss status

---

## 4) Configuring Win/Lose Rules

Use either gameplay.ini (global defaults) or per‑level overrides at assets/config/levels/levelN.ini.

### [win] keys (most common)

- win_on_timer=true|false
- survive_time_seconds=45.0
- win_on_boss_defeat=true|false
- win_on_defeat_all_enemies=true|false
- end_distance=1600.0           # reaching this from player start can win the level
- require_boss_and_end=true|false # when boss victory is enabled, require both boss defeat AND reaching end
- next_level=level2
- debug_overlay=true|false       # one‑line tester overlay below HUD

### Notes

- Per‑level [win] overrides global defaults
- Immediate victory on reaching end_distance if allowed by the boss/require_boss_and_end settings
- On timeout, we deterministically decide WIN/LOSE based on whether required conditions have been met

### Level 1 defaults (ensures an achievable first experience)

- assets/config/levels/level1.ini sets: `end_distance=1600`, `win_on_timer=false`, `next_level=level2`
- The game now defaults to loading level1 on first start (so HUD shows LEVEL: level1)

---

## 5) Common Tasks for Contributors

### Add a new level

1) Create `assets/config/levels/levelX.ini`
2) Add a [win] block (e.g., next_level, end_distance) and optionally explicit enemy placements with [enemy.N]
3) Run, press the matching hotkey (e.g., 2, 3, B), or call `LoadLevelConfig("levelX")` in code

### Add a new enemy or change sprites

1) Define a template in `assets/config/characters.ini` (sprite path, size, stats)
2) Reference it from levels via placements or spawn logic

### Tweak HUD or rules

- Edit `assets/config/gameplay.ini` [visual], [audio], [game_rules], and [win]
- For temporary tester info, set `[win] debug_overlay=true`

### Run tests

- `./test/run_tests.sh`
- Or build-and-run from `build/` if you’ve enabled test targets in CMake

### Code style & PRs

- Modern C++17, small focused PRs, include a short description of what/why
- Prefer config‑driven behavior over hard‑coding; keep HUD readable and non‑overlapping

---

## 6) Troubleshooting

- Black window or crash: ensure SDL2, SDL2_image, SDL2_mixer are installed and discoverable by CMake
- Player sprite not visible: check `[player] sprite_path` in `characters.ini` (engine falls back to simple shapes)
- LEVEL shows “Base”: means no level override is active; press 1 for level1 or see default‑load code in PlayingState::OnEnter
- Can’t finish by distance: confirm `[win] end_distance` is set and `win_on_boss_defeat/require_boss_and_end` aren’t blocking

---

## 7) License

MIT
