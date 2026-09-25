# Minecraft_ES

A lightweight Minecraft-like game for Android, written in C++ with OpenGL ES 3.0.

## Features

- Infinite procedural world with Perlin noise terrain generation
- Chunk system (16x256x16) with dynamic loading/unloading
- Greedy-ish meshing for block rendering
- Multiple block types: Grass, Dirt, Stone, Bedrock, Wood, Leaves, Sand, Water
- First-person player controller with physics (gravity, collision, swimming)
- Mobile controls:
  - Virtual joystick (bottom-left) for movement
  - Touch/drag anywhere else to look around
  - Jump button (bottom-right)
  - Tap to place blocks
  - Long-press to break blocks
- Day/night cycle with sky color and ambient lighting
- Crosshair and target block highlight
- World saving/loading (internal storage)
- Procedurally generated texture atlas (no copyrighted assets)

## Building

This project builds automatically via GitHub Actions. On push to `main`, an APK will be produced as a build artifact.

### Local Build

Requirements:
- Android SDK (API 34)
- Android NDK (25.2.9519653)
- CMake 3.22.1+
- JDK 17

