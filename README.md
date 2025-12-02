<h1>Threepp Excavator</h1>

[![CI](https://github.com/TOlsnes/ThreeppExcavator/actions/workflows/ci.yml/badge.svg)](https://github.com/TOlsnes/ThreeppExcavator/actions/workflows/ci.yml)

Threepp Excavator is a 3D simulation demo featuring an articulated excavator with physics-based collision detection, particle effects, audio integration, and interactive gameplay elements.

<h2>Features</h2>

- **Articulated Excavator**: Full kinematic chain with tracks, turret, boom, stick, and bucket
- **Physics-Based Collision**: Convex hull collision detection using mesh geometry
- **Particle System**: Dynamic dust/debris effects during movement
- **Track Marks**: Persistent decals showing vehicle path history
- **Audio System**: Engine sounds (startup, idle, hydraulics, steam, coin collection)
- **Coin Collection**: 15 randomly placed animated coins with independent bobbing
- **Dig/Dump Gameplay**: Scoop dirt from pile and deposit in designated dump zone
- **ImGui UI**: Real-time coin counter and adjustable master volume
- **Castle Environment**: Walls, doors, and perimeter rails with collision
- **Camera Controls**: Mouse-drag orbit camera

<h2>Controls</h2>

<ul>
  <li><strong>W/S</strong>: Forward/Reverse movement</li>
  <li><strong>A/D</strong>: Turn left/right (differential steering)</li>
  <li><strong>Q/E</strong>: Rotate turret left/right</li>
  <li><strong>R/F</strong>: Boom up/down</li>
  <li><strong>T/G</strong>: Stick extend/retract</li>
  <li><strong>Y/H</strong>: Bucket curl/uncurl</li>
  <li><strong>P</strong>: Reset game state (respawn coins, clear progress)</li>
  <li><strong>V</strong>: Toggle collision debug visualization</li>
  <li><strong>Mouse Drag</strong>: Orbit camera around excavator</li>
</ul>

<h2>Build Requirements</h2>

### All Platforms
- CMake 3.16+
- C++20 compatible compiler:
  - MSVC 2019+ (Windows)
  - GCC 10+ (Linux)
  - Clang 12+ (macOS)

### Linux Dependencies
```bash
sudo apt-get install -y \
  libxrandr-dev libxinerama-dev libxcursor-dev \
  libxi-dev libgl1-mesa-dev libasound2-dev
```

### macOS
```bash
xcode-select --install
```

<h2>Building</h2>

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release -j

# Run
./build/Release/main      # Linux/macOS
.\build\Release\main.exe  # Windows
```

<h2>Testing</h2>

Unit tests use [Catch2](https://github.com/catchorg/Catch2):

```bash
# Build and run tests
cmake --build build --config Debug
cd build
ctest -C Debug --output-on-failure
```

**Test Coverage:**
- CollisionWorld: Ground checks, collider management, movement resolution
- ParticleSystem: Lifecycle, spawning, fading, cleanup
- Coin/CoinManager: State management, collection radius, reset

<h2>Continuous Integration</h2>

GitHub Actions CI validates builds and tests on:
- Ubuntu (GCC)
- Windows (MSVC)
- macOS (Clang)

Both Debug and Release configurations are tested.

<h2>Architecture</h2>

```
blocks/
├── include/           # Public headers
│   ├── AudioManager.hpp
│   ├── Coin.hpp, CoinManager.hpp
│   ├── CollisionWorld.hpp
│   ├── DigZone.hpp, DumpZone.hpp
│   ├── Excavator.hpp
│   ├── ObjectSpawner.hpp
│   ├── ParticleSystem.hpp
│   ├── Renderer.hpp
│   ├── Settings.hpp       # Global tuning parameters (inline)
│   ├── TrackMarkManager.hpp
│   └── World.hpp
├── src/
│   ├── Logic/         # Game logic & physics
│   └── Visualization/ # Rendering & effects
├── tests/             # Catch2 unit tests
├── models/            # OBJ meshes & WAV audio
└── .github/workflows/ # CI configuration
```

**Key Systems:**
- **CollisionWorld**: Static singleton managing convex hull colliders for all static geometry
- **Excavator**: Hierarchical scene graph with pivot nodes for each joint; per-frame collision resolution
- **Settings**: Header-only namespace with inline globals for runtime configuration
- **ParticleSystem**: Short-lived alpha-fading quads spawned at track contact points
- **TrackMarkManager**: Deferred decal placement with distance-based spawning and timed fadeout

<h2>UML Class Diagram</h2>

<img src="UML diagram.png" alt="UML Diagram" width="600">

Settings provides centralized configuration. InputManager is used by main for key/mouse events.

<h2>Dependencies (Auto-Fetched)</h2>

- [threepp](https://github.com/markaren/threepp) - 3D graphics library (Three.js port)
- [GLFW](https://github.com/glfw/glfw) - Windowing & input (via threepp)
- [Dear ImGui](https://github.com/ocornut/imgui) - Immediate mode UI
- [Catch2](https://github.com/catchorg/Catch2) - Unit testing framework

All dependencies are fetched automatically via CMake `FetchContent`.

<h2>Reflection</h2>

Throughout this project i felt like it kinda turned away from the creative fun project i was doing into a chore which i do think negatively affected my work. I had a lot of fun working on the interactions inbetween the different parts of the excavator and figure out how to get it working propperly while closer to the end it became more of a oh add another thing to satisfy the requirements over and over. I really enjoyed working out the physical problems like how to get the tracks to move. Thats the main reason why i wanted to make a excavator instead of a car like everyone else. I could make the animation a lot smoother by adding more models that are closer together as right now theres just 3 different animations. I think toward the end i ended up looking up way more and thinking less when i came upon problems, that kinda snowballed. 

<h2>Improvements</h2>

There is a LOT to improve on here. First of all i think as i got lazier with my coding it also got less and less optimized and way more code. Other than that the error handeling is probably obysmal. There was quite many errors i let copilot fix without fully understanding which came back to bite me in the butt. Finally i should probably add the unlockable gate (whcich i forgot) aswell as better audio integration including things like collision, scooping and dumping of materials. If it were to be played as an actual mini-game some explanatory UI should also be added, for example pop ups/guides telling you how the controlls work, i think it could function pretty well as a tutorial for a bigger game.

**Recent Improvements:**
- Added Catch2 unit tests for core systems (CollisionWorld, ParticleSystem, Coin/CoinManager)
- Implemented CI/CD via GitHub Actions (cross-platform validation)
- Fixed per-coin animation state (coins now animate independently after collection)
- Resolved runtime crashes (leftTrackPivot_ initialization)
- Cleaned unused/duplicate API methods across multiple subsystems
- Corrected include paths for threepp folder structure
- Added inline semantics to Settings globals to prevent linker errors

<h2>Sources</h2>
1. Use of goto in Collision world (i read that its bad to use goto but not if its inside of a nested loop): https://stackoverflow.com/questions/3517726/what-is-wrong-with-using-goto

2. Collision detection in Collision world (especially the part that pushes the excavator away if its too close (288-314): https://gamedev.net/forums/topic/715956-aabb-collision/

3. Convex hull collision (this one helped me visualize what everything actually ment when it came to collision) https://stackoverflow.com/questions/56912376/3d-collision-detection-convex-hull-vs-convex-hull-need-position-and-normal

4. The tests are mainly using Github copilot and this: https://github.com/catchorg/Catch2

5. Other files like .gitignore and the ci workflows are also using github copilot
