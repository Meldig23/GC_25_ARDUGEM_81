# GC_25_ARDUGEM_81

# 🚀 Cosmic Race

**Cosmic Race** is a two-player space-themed arcade game built using the **Arduino Nano**, **TFT Display**, and **Rotary Encoders**. In this game, players control rockets that are racing to escape the Earth’s atmosphere and traverse through challenging space environments. Strategy, speed, and timing are key!

### Game Lore
Far in the future, humans discovered how to travel at the speed of light. Two brave pilots launched into space, racing to escape Earth’s atmosphere. But something strange happened… The faster they went, the slower time moved. They weren’t just flying through space — they were flying through time. Now, every move they make decides not just how far they go, but when they end up.

Will they reach their destination before time runs out… or be lost forever in the race through time and space?


## 🎮 Game Overview

The game starts with **two rockets** attempting to **escape Earth’s atmosphere**. The journey is divided into two main levels:

### 🌌 Game Levels

- **Level 1 – Satellite Escape**  
  Navigate your rocket and escape through an orbit filled with satellites to reach a set distance.

- **Level 2 – Asteroid Escape**  
  After satellites, you must dodge incoming asteroids and continue your race to the final destination.

---


## 📜 Main Menu

- **Help** – Explains how to play the game
- **Start / Resume**
- **Restart**
- **P1 Color** – Choose Rocket color for Player 1
- **P2 Color** – Choose Rocket color for Player 2

> Start / Resume / Restart require both players to press their buttons simultaneously.  
> Help can be accessed by either player, P1 Color and P2 Color accessed by respective players.
> During Game Holding both button for 2 seconds returns to the main menu.



## 🎯 Game Objective

1. **First to reach 100m within 120 seconds wins!**
2. Avoid obstacles — hitting one will **freeze your movement temporarily**, but your timer **keeps running**.
3. Collect **green dots (boosters)** to **add time** to your timer.
4. Use **attack** (press button) to **freeze opponent** for 3 seconds.
   - **Cooldown:** You can attack only once every **15 seconds**.

## 🕹️ Controls

- Players use **rotary encoders** to move:
  - **Clockwise:** Move Right
  - **Counter-Clockwise:** Move
- Attck Can be done by Pressing the button
 

## Game Mechanics

- **Distance:** Starts from 0m → reach **100m** to win.
- **Timer:** Each player has **120 seconds**.
- **Collision:** Hitting an obstacle **freezes your distance** temporarily for 5sec.
- **Booster (Green Dot):** Increases your **remaining time**.
- **Attack:** 
  - Pressing your button freezes opponent for **3 seconds**
  - **Cooldown:** 15 seconds between attacks



## 🏁 Game Win Conditions

- **If one player reaches 100m before time ends**, they win.
- **If both reach 100m**, the player with **more time remaining** wins.
- **If both run out of time** before reaching 100m, **both lose**.
- **If one runs out of time**, we wait for the other:
  - If second player reaches 100m, they win.
  - If not, **both lose**.


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



## 📐 Implementation

The game logic is implemented on the Arduino Nano. The TFT display renders the gameplay including the rockets, obstacles, abilities, timers, and menu interface. Rotary encoders handle movement and menu interactions. The main program flow includes:

- **Initialization of Display and Encoders**
- **Menu Interface Rendering**
- **Game State Handling (Start, Resume, Restart)**
- **Real-time Obstacle and Ability Spawning**
- **Collision Detection and Timer Management**
- **Win/Lose Logic**

## 🧾 Project Structure

```plaintext
Cosmic-Race/
├──  rocket-red.h                
├── rocket-blue.h
├── rocket-yellow.h
├── satellite.h
├── meteoroid.h
├── CosmicRace.ino         
└── README.md              
