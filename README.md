# GC_25_ARDUGEM_81

# 🚀 Cosmic Race

**Cosmic Race** is a two-player space-themed arcade game built using the **Arduino Nano**, **TFT Display**, and **Rotary Encoders**. In this game, players control rockets that are racing to escape the Earth’s atmosphere and traverse through challenging space environments. Strategy, speed, and timing are key!

---

## 🎮 Game Overview

The game starts with **two rockets** attempting to **escape Earth’s atmosphere**. The journey is divided into two main levels:

### 🌌 Game Levels

- **Level 1 – Satellite Escape**  
  Navigate your rocket and escape through an orbit filled with satellites to reach a set distance.

- **Level 2 – Asteroid Escape**  
  After satellites, you must dodge incoming asteroids and continue your race to the final destination.

---

## 🧠 Game Mechanics

- Players must **reach the destination within a time limit**.
- **Abilities** can be collected to modify the game:
  - **Ability 1:** Increases your time limit.
  - **Ability 2:** Decreases the opponent’s time limit.
- **Obstacles** will appear during the journey:
  - Colliding with them **freezes** your movement for a short duration.
- **Each player has an individual time display** on their side of the screen.
- The player who **reaches the goal first** (within the time limit) **wins**.

---

## 🏁 Game End Conditions

- ✅ **Win:** A player reaches the destination before their timer runs out.
- 🏆 **Win:** If both players reach the destination, the one who arrives first wins.
- ❌ **Lose:** If both players fail to reach in time, both lose the game.

---

## 🕹️ Controls

- Players use **rotary encoders** to move:
  - **Clockwise:** Move Right
  - **Counter-Clockwise:** Move Left

- **Main Menu Controls:**
  - **Help** – View instructions.
  - **Start/Resume** – Both players must press their buttons **simultaneously**.
  - **Restart** – Also requires both players to press together.
  - **P1 Color** – Player 1 chooses their rocket color.
  - **P2 Color** – Player 2 chooses their rocket color.

- **Returning to Main Menu:**
  - Any player can press any button from the help section to reach back to Main Menu.

---

## 📟 Hardware Components

| Component            | Quantity |
|---------------------|----------|
| Arduino Nano         | 1        |
| TFT Display (SPI)    | 1        |
| Rotary Encoders      | 2        |
| Rotary Encoder Hats  | 2        |
| Breadboard           | 1        |
| Push Buttons         | 2        |
| Connecting Wires     | As needed |

---

## 📐 Implementation

The game logic is implemented on the Arduino Nano. The TFT display renders the gameplay including the rockets, obstacles, abilities, timers, and menu interface. Rotary encoders handle movement and menu interactions. The main program flow includes:

- **Initialization of Display and Encoders**
- **Menu Interface Rendering**
- **Game State Handling (Start, Resume, Restart)**
- **Real-time Obstacle and Ability Spawning**
- **Collision Detection and Timer Management**
- **Win/Lose Logic**

---

## 🧾 Project Structure

```plaintext
Cosmic-Race/
├── images/                 # Game assets and rocket icons (optional)
├── CosmicRace.ino         # Main Arduino sketch
├── README.md              # This file
├── libraries/             # Required libraries (TFT, encoder, etc.)
└── docs/                  # Any additional documentation or help files
