# NotMario
NotMario is a 2D platformer game inspired by the classic Mario series. Built from the ground up in C++, the game utilizes the Simple and Fast Multimedia Library (SFML) for rendering and user interface. The game engine is self-implemented using the Entity-Component-System (ECS) architecture to provide a modular and efficient design.

## Features
Custom Game Engine: Built using the Entity-Component-System (ECS) architecture, ensuring a lightweight engine with high performance.\
SFML-Based UI: Uses SFML for rendering graphics, handling input, and managing window operations.\
Classic Gameplay: Jump, run, shoot bullets! and collect coins through levels inspired by the original Mario games.\
Object-Oriented Design: Emphasizes clean and maintainable code with a focus on reusability and modularity.\
Garbage Collection: Includes a self-implemented garbage collection system using RAII with smart pointers for efficient memory management.

## Dependencies
SFML: Simple and Fast Multimedia Library for graphics, audio and window management.\
C++17 or higher.

## Architecture Overview
The game is designed using the Entity-Component-System (ECS) architecture, which decouples game data (components) from logic (systems).

Entity: Represents game objects like Mario, enemies, or blocks.\
Component: Holds data for an entity (eg., position, gravity, lifetime ,velocity, sprite).\
System: Contains logic that operates on entities with specific components (eg., rendering system, physics system).

## Preview
<img width="599" alt="working" src="https://github.com/AkshaySodhi/NotMario/assets/95957791/42ad9750-500b-48df-abfa-74778c33565a">
<img width="599" alt="working" src="https://github.com/AkshaySodhi/NotMario/assets/95957791/80381e3d-4eac-4eae-87d5-1f6573acc7c2">

