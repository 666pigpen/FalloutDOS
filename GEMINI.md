# FalloutDOS

A C++17 text-based adventure game set in a post-apocalyptic wasteland, inspired by the Fallout series.

## Project Overview

- **Core Loop:** Research -> Strategy -> Execution (Plan, Act, Validate).
- **Architecture:** 
  - `Game`: Central engine managing state, input, and rendering.
  - `Player`: Handles S.P.E.C.I.A.L. stats, inventory, and health.
  - `Room`: Defines the world nodes, exits, and local items.
  - `Item`: Basic entity for interactable objects.
  - `Command`: Simple POD for parsed user input.

## Project Status

- **Initial State:** Basic world with ~10 rooms (Vault 13 entrance to Riverboat Casino).
- **Features:** Navigation, inventory management, S.P.E.C.I.A.L. based checks (e.g., Strength for crates), basic combat/trading (via `talk`/`buy`), and a simple casino mini-game.
- **Build System:** CMake (though manual compilation with `g++ -std=c++17 ...` is used as a fallback).

## Known Issues / Technical Debt

- [x] **Repeatable Interactions:** Some "open" commands (like the crate in the shed) don't track if they've already been triggered, allowing for duplicate rewards.
- [x] **Parser Limitations:** The command parser is limited to simple `verb noun` structures and doesn't support complex objects or multiple targets.
- [x] **Static World:** World setup is hardcoded in `Game::setupWorld()`.

## Roadmap

- [x] Add Brotherhood of Steel base and questline.
- [x] Add Enclave outpost and questline.
- [x] Add Railroad hideout and questline.
- [x] Implement a system for faction reputation or conflicting choices.
- [x] Add an armor system with equip/unequip mechanics.
- [x] Add a Save/Load system using a JSON file.
- [x] Implement a Leveling/XP system with Perks.
- [x] Implement a Crafting system for combining items.
