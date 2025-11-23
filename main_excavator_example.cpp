#include <threepp/threepp.hpp>
#include <iostream>
#include <numbers>
#include <fstream>
#include <array>
#include <tuple>
#include <cmath>

#include "World.hpp"
#include "Renderer.hpp"
#include "Excavator.hpp"
#include "ObjectSpawner.hpp"
#include "ParticleSystem.hpp"
#include "CollisionWorld.hpp"
#include "DigZone.hpp"
#include "DumpZone.hpp"
#include "AudioManager.hpp"
#include "CoinManager.hpp"
#include "TrackMarkManager.hpp"
// Load external models
#include <threepp/loaders/OBJLoader.hpp>
#include <threepp/audio/Audio.hpp>
#include <threepp/objects/Text.hpp>
#include <filesystem>
#include "threepp/extras/imgui/ImguiContext.hpp"

using namespace threepp;

constexpr float PI = 3.1415927f;

int main() {
    // File log to diagnose startup
    std::ofstream logFile("run.log", std::ios::app);
    logFile << "[start] program begin" << std::endl;
    // Make the window explicit (title + size) to avoid hidden/zero-size issues
    Canvas::Parameters params;
    params.title("Blocks Excavator").size(1280, 720).vsync(true).resizable(true);
    Canvas canvas(params);
    logFile << "[init] canvas created" << std::endl;
    Renderer renderer(canvas);
    renderer.setClearColor(Color(0.5f, 0.7f, 1.0f)); // Sky blue

    World world;

    // Add lighting so track models aren't black (mainly so we can see the tracks move)
    auto ambientLight = AmbientLight::create(Color::white, 0.6f);
    world.scene().add(ambientLight);
    
    auto directionalLight = DirectionalLight::create(Color::white, 0.8f);
    directionalLight->position.set(5, 10, 7);
    directionalLight->castShadow = true;
    world.scene().add(directionalLight);

    // --- Setup Environment ---
    ObjectSpawner::SpawnConfig spawnConfig;
    spawnConfig.arenaRadius = 30.0f;
    spawnConfig.smallObjectCount = 100;
    spawnConfig.mediumObjectCount = 30;
    spawnConfig.largeObjectCount = 8;
    
    ObjectSpawner spawner(world.scene(), spawnConfig);
    spawner.generateEnvironment();

    // --- Setup Excavator ---
    // UPDATE THESE PATHS to match your OBJ file locations!
    Excavator::Paths excavatorPaths;
    excavatorPaths.leftTrack0  = "models/TrackAnimation1.obj";
    excavatorPaths.leftTrack1  = "models/TrackAnimation2.obj";
    excavatorPaths.leftTrack2  = "models/TrackAnimation3.obj";
    excavatorPaths.rightTrack0 = "models/TrackAnimation1.obj";
    excavatorPaths.rightTrack1 = "models/TrackAnimation2.obj";
    excavatorPaths.rightTrack2 = "models/TrackAnimation3.obj";
    excavatorPaths.base        = "models/BasePlate.obj";
    excavatorPaths.body        = "models/MainBody.obj";
    excavatorPaths.arm1        = "models/Arm1.obj";
    excavatorPaths.arm2        = "models/Arm2.obj";
    excavatorPaths.bucket      = "models/Bucket.obj";

    Excavator excavator(excavatorPaths, world.scene());

    // Position the excavator in the world (lift it up so it sits on the plane)
    excavator.root()->position.set(0, 0, 0);
    
    // --- Particle System ---
    ParticleSystem particleSystem(world.scene());
    excavator.setParticleSystem(&particleSystem);
    
    // --- Castle + Doorway Pile ---
    auto resolveAssetPath = [](const std::string& rel) -> std::string {
        using std::filesystem::exists;
        using std::filesystem::path;
        const path candidates[] = {
            path(rel),
            path("../") / rel,
            path("../../") / rel,
            path("../../../") / rel
        };
        for (const auto& p : candidates) if (exists(p)) return p.string();
        return rel;
    };

    // Castle placement config
    const Vector3 castlePos{0, 0, 18};
    const float castleScale = 0.03f; // scaled up 1.5x from previous (0.02 -> 0.03)
    const float doorwayOffsetWorld = 2.3f; // world-space distance from castle center to doorway pile
    // These will be determined dynamically after final castle orientation
    Vector3 doorwayDirWorld; // normalized world-space doorway direction
    Vector3 pilePos;         // world-space pile position derived from doorway

    // Load castle or split wall parts if present
    std::shared_ptr<Object3D> castle;
    {
        using std::filesystem::exists;
        using std::filesystem::path;

        threepp::OBJLoader loader;

        // Try loading split wall/door parts from models/ (WallLeft, WallRight, WallBack, DoorLeft, DoorRight)
        std::vector<std::string> partNames = {"WallLeft", "WallRight", "WallBack", "DoorLeft", "DoorRight"};
        std::vector<std::shared_ptr<Object3D>> loadedParts;
        for (const auto& name : partNames) {
            auto partPath = resolveAssetPath("models/" + name + ".obj");
            try {
                auto part = loader.load(partPath, true);
                if (part) {
                    loadedParts.push_back(part);
                }
            } catch (...) {
                // part missing or load error; skip
            }
        }

        if (!loadedParts.empty()) {
            auto castleGroup = Object3D::create();
            castleGroup->position.copy(castlePos);
            castleGroup->scale.setScalar(castleScale);
            for (auto& p : loadedParts) castleGroup->add(p);

            // Apply the same orientation tweaks as before
            castleGroup->rotation.x += threepp::math::PI / 2.0f;
            castleGroup->updateMatrixWorld(true);
            {
                threepp::Box3 box;
                box.setFromObject(*castleGroup, false);
                auto bmin = box.min();
                if (bmin.y != 0.f) castleGroup->position.y -= bmin.y;
            }
            castleGroup->rotation.x += threepp::math::PI; // 180° flip
            castleGroup->rotation.y = 0.f;
            castleGroup->rotation.z += threepp::math::PI / 2.0f; // 90° about Z
            castleGroup->updateMatrixWorld(true);
            {
                threepp::Box3 box;
                box.setFromObject(*castleGroup, false);
                auto bmin = box.min();
                if (bmin.y != 0.f) castleGroup->position.y -= bmin.y;
            }

            // Derive direction from castle toward arena center for pile placement at doorway
            threepp::Vector3 toCenter = threepp::Vector3(0,0,0) - castleGroup->position; toCenter.normalize();
            doorwayDirWorld = toCenter; // pile should be between castle and center (at doorway)
            pilePos = castleGroup->position + doorwayDirWorld * doorwayOffsetWorld;
            std::cout << "[CastleParts] Loaded " << loadedParts.size() << " wall/door parts, pile at: " 
                      << pilePos.x << "," << pilePos.y << "," << pilePos.z << std::endl;

            // Add per-part mesh colliders (keeps doorway as a gap)
            for (auto& child : castleGroup->children) {
                if (child) CollisionWorld::addRockMeshColliderFromObject(*child);
            }

            world.scene().add(castleGroup);
            castle = castleGroup;
        }

        // Fallback: single Castle.obj
        if (!castle) {
            threepp::OBJLoader loader;
            auto path = resolveAssetPath("models/Castle.obj");
            castle = loader.load(path, true);
            if (castle) {
                castle->position.copy(castlePos);
                castle->scale.setScalar(castleScale);

                auto applyRotationAndGetHeight = [&](float rx, float ry, float rz) -> std::pair<float, threepp::Box3> {
                    castle->rotation.set(rx, ry, rz);
                    castle->updateMatrixWorld(true);
                    threepp::Box3 box;
                    box.setFromObject(*castle, false);
                    threepp::Vector3 size; box.getSize(size);
                    return {size.y, box};
                };

                std::array<std::tuple<float,float,float>,5> candidates = {
                    std::make_tuple(0.f, 0.f, 0.f),
                    std::make_tuple(threepp::math::PI / 2.0f, 0.f, 0.f),
                    std::make_tuple(-threepp::math::PI / 2.0f, 0.f, 0.f),
                    std::make_tuple(0.f, 0.f, threepp::math::PI / 2.0f),
                    std::make_tuple(0.f, 0.f, -threepp::math::PI / 2.0f)
                };

                float bestH = -1.f; threepp::Box3 bestBox; std::tuple<float,float,float> bestRot = candidates[0];
                for (auto& rot : candidates) {
                    auto [rx, ry, rz] = rot;
                    auto [h, box] = applyRotationAndGetHeight(rx, ry, rz);
                    if (h > bestH) { bestH = h; bestRot = rot; bestBox = box; }
                }

                auto [brx, bry, brz] = bestRot;
                castle->rotation.set(brx, bry, brz);
                castle->updateMatrixWorld(true);

                auto bmin = bestBox.min();
                if (bmin.y != 0.f) {
                    castle->position.y -= bmin.y;
                }

                castle->rotation.x += threepp::math::PI / 2.0f;
                castle->updateMatrixWorld(true);
                {
                    threepp::Box3 boxAfter;
                    boxAfter.setFromObject(*castle, false);
                    auto bminAfter = boxAfter.min();
                    if (bminAfter.y != 0.f) castle->position.y -= bminAfter.y;
                }

                castle->rotation.x += threepp::math::PI; // 180° flip
                castle->updateMatrixWorld(true);
                {
                    threepp::Box3 boxAfter2;
                    boxAfter2.setFromObject(*castle, false);
                    auto bminAfter2 = boxAfter2.min();
                    if (bminAfter2.y != 0.f) castle->position.y -= bminAfter2.y;
                }

                // Manual 90° Z rotation
                castle->rotation.y = 0.f;
                castle->rotation.z += threepp::math::PI / 2.0f;
                castle->updateMatrixWorld(true);
                doorwayDirWorld.set(0,0,-1);
                doorwayDirWorld.applyEuler(castle->rotation);
                doorwayDirWorld.normalize();
                pilePos = castle->position + doorwayDirWorld * doorwayOffsetWorld;
                std::cout << "[Castle] Applied 90deg Z rotation. DoorwayDir="
                          << doorwayDirWorld.x << "," << doorwayDirWorld.y << "," << doorwayDirWorld.z << std::endl;

                world.scene().add(castle);
                // Single convex collider for whole castle
                CollisionWorld::addRockMeshColliderFromObject(*castle);

                // Keep interior pass-through via doorway zone for single-mesh fallback
                threepp::Vector3 toCenter = threepp::Vector3(0,0,0) - castle->position; toCenter.normalize();
                float doorYaw = std::atan2(toCenter.x, toCenter.z);
                CollisionWorld::NoCollisionZone doorZone{};
                doorZone.center = castle->position - toCenter * (doorwayOffsetWorld + 0.25f);
                doorZone.halfWidth = 1.2f;
                doorZone.halfDepth = 0.35f;
                doorZone.yaw = doorYaw;
                CollisionWorld::addNoCollisionZone(doorZone);
                std::cout << "[DoorZone] center=" << doorZone.center.x << "," << doorZone.center.z
                          << " yaw(deg)=" << (doorYaw * 180.0f / PI)
                          << " hw=" << doorZone.halfWidth << " hd=" << doorZone.halfDepth << std::endl;
            } else {
                std::cout << "[Castle] models/Castle.obj not found (loading skipped)." << std::endl;
                doorwayDirWorld.set(0,0,-1);
                pilePos = castlePos + doorwayDirWorld * doorwayOffsetWorld;
            }
        }
    }

    // --- Place Rails around perimeter ---
    {
        auto railPath = resolveAssetPath("models/rAIL.obj");
        std::cout << "Loading rail from " << railPath << " (with .mtl)..." << std::endl;
        
        OBJLoader loader;
        auto railTemplate = loader.load(railPath, true);
        
        if (railTemplate) {
            std::cout << "Rail template loaded successfully!" << std::endl;
            
            // Place 8 rails around the perimeter in a circle
            const int railCount = 8;
            const float railRadius = 20.0f; // Closer to center
            const float angleOffset = 22.5f * (PI / 180.0f); // Offset by 22.5 degrees
            
            for (int i = 0; i < railCount; i++) {
                float angle = (i / float(railCount)) * 2.0f * PI + angleOffset;
                
                auto rail = railTemplate->clone();
                
                // Scale rail appropriately
                rail->scale.setScalar(0.04f);
                
                // Rotate rail to stand upright
                rail->rotation.x = -PI / 2.0f; // Stand upright (flipped)
                rail->rotation.z = angle; // Align with circle tangent
                
                rail->updateMatrixWorld(true);
                
                // Position on the plane surface
                rail->position.x = railRadius * std::cos(angle);
                rail->position.y = 0.0f; // On the ground
                rail->position.z = railRadius * std::sin(angle);
                
                std::cout << "Placed rail " << i << " at (" << rail->position.x << ", " << rail->position.y << ", " << rail->position.z << ")" << std::endl;
                
                world.scene().add(rail);
                CollisionWorld::addRockMeshColliderFromObject(*rail);
            }
            
            std::cout << "Placed " << railCount << " rails around perimeter" << std::endl;
        } else {
            std::cout << "Failed to load rail model" << std::endl;
        }
    }

    // Place the dig pile at the dynamically computed doorway position
    DigZone digZone(pilePos, 3.0f); // Restore pile radius to original size
    world.scene().add(digZone.getVisual());
    CollisionWorld::addRockMeshColliderFromObject(*digZone.getVisual());
    
    DumpZone dumpZone(Vector3(10, 0, -10), 3.0f); // Dump area on the right
    world.scene().add(dumpZone.getVisual());
    
    std::cout << "Excavator root has " << excavator.root()->children.size() << " children\n";
    
    // Debug: print bounds of first loaded mesh to see scale
    if (!excavator.root()->children.empty()) {
        auto firstChild = excavator.root()->children[0];
        std::cout << "First child position: " << firstChild->position.x << ", " 
                  << firstChild->position.y << ", " << firstChild->position.z << "\n";
    }

    // --- Camera ---
    PerspectiveCamera camera(60, canvas.aspect(), 0.1, 1000);
    // Start camera behind excavator
    camera.position.set(-5, 5, -5);
    camera.lookAt(0, 0, 0);

    Clock clock;

    // --- Camera orbit state ---
    bool isMouseButtonDown = false;
    float cameraDistance = 7.0f; // Move camera closer (was 10.5f)
    float cameraAngleH = 0.0f;  // horizontal angle (around Y axis) - start behind excavator
    float cameraAngleV = PI * 0.35f;  // vertical angle (up/down) - lower camera
    Vector2 lastMousePos;

    // --- Control state ---
    bool wDown = false, sDown = false, aDown = false, dDown = false;
    bool qDown = false, eDown = false;
    bool rDown = false, fDown = false;
    bool tDown = false, gDown = false;
    bool yDown = false, hDown = false;
    // Debug alignment keys
    bool flipStick = false, flipBucket = false;
    bool nudgeStickPos = false, nudgeStickNeg = false;
    bool nudgeBucketPos = false, nudgeBucketNeg = false;
    // Pivot nudges
    bool nudgeStickPivotPos = false, nudgeStickPivotNeg = false;
    bool nudgeBucketPivotPos = false, nudgeBucketPivotNeg = false;
    // Nudge step size (in local units before scaling)
    float pivotStep = 0.5f; // start with a larger step so it's visible

    // --- Audio state ---
    float idleTargetVolume = 0.18f;   // desired idle volume based on speed
    float idleVolumeSmoothed = 0.18f; // smoothed idle volume to avoid clicks
    float masterVolume = 1.0f; // UI-controllable master volume
    
    // Arm movement tracking for hydraulics/steam sounds
    bool boomMovingUp = false, boomMovingDown = false;
    bool stickMovingUp = false, stickMovingDown = false;
    bool bucketMovingUp = false, bucketMovingDown = false;
    float prevBoomAngle = 0.0f;
    float prevStickAngle = 0.0f;
    float prevBucketAngle = 0.0f;

    // Debug collision visualization
    bool showCollisionDebug = false;
    std::vector<std::shared_ptr<Object3D>> debugObjects;

    // Digging state
    int digScoops = 0;
    bool pileGone = false;

    // --- Fade overlay for reset transition ---
    bool isResetting = false;
    float resetFadeTimer = 0.0f;
    float fadeOpacity = 0.0f;
    const float fadeDuration = 0.5f; // 0.5s fade in, 0.5s fade out = 1.0s total
    
    // Create overlay scene with orthographic camera for fullscreen fade
    Scene overlayScene;
    OrthographicCamera overlayCamera(-1, 1, 1, -1, 0, 1);
    auto overlayGeometry = PlaneGeometry::create(2, 2); // Fullscreen quad
    auto overlayMaterial = MeshBasicMaterial::create();
    overlayMaterial->color.setRGB(1.0f, 1.0f, 1.0f); // White
    overlayMaterial->transparent = true;
    overlayMaterial->opacity = 0.0f;
    overlayMaterial->depthTest = false;
    overlayMaterial->depthWrite = false;
    auto overlayMesh = Mesh::create(overlayGeometry, overlayMaterial);
    overlayMesh->position.set(0, 0, -0.5f);
    overlayScene.add(overlayMesh);

    // --- Audio System ---
    AudioListener audioListener;
    camera.add(audioListener); // Attach listener to camera
    AudioManager audioManager(audioListener);
    
    // Load sound files
    audioManager.loadStartupSound(resolveAssetPath("models/startExcavator.wav"));
    audioManager.loadIdleSound(resolveAssetPath("models/idleExcavator.wav"));
    audioManager.loadHydraulicsSound(resolveAssetPath("models/hydraulicsExcavator.wav"));
    audioManager.loadSteamSound(resolveAssetPath("models/steamExcavator.wav"));
    audioManager.loadDigSound(resolveAssetPath("models/dig.wav"));
    audioManager.loadDumpSound(resolveAssetPath("models/dump.wav"));
    audioManager.loadCollisionSound(resolveAssetPath("models/collision.wav"));
    audioManager.loadCoinSound(resolveAssetPath("models/coinExcavator.wav"));
    
    // Start engine with startup sound
    audioManager.startEngine();
    std::cout << "[audio] Engine startup sequence initiated" << std::endl;

    // --- Coin System ---
    CoinManager coinManager(world.scene());
    coinManager.spawnCoins(15, spawnConfig.arenaRadius); // 15 coins scattered around

    // --- Track marks ---
    TrackMarkManager trackMarks(world.scene());
    // Spawn marks 1.5x quicker: reduce distance (0.6 / 1.5 ≈ 0.4)
    trackMarks.setSpawnDistance(0.4f);
    trackMarks.setLifetime(3.f);       // disappear after 3 seconds

    // --- ImGui UI (persistent context) ---
    ImguiFunctionalContext ui(canvas.windowPtr(), [&] {
        ImGui::SetNextWindowPos({0, 0}, 0, {0, 0});
        ImGui::SetNextWindowSize({690, 120}, 0); // Further reduced height
        ImGui::Begin("Excavator UI");
        ImGui::SetWindowFontScale(1.5f); // Increase text size
        ImGui::Text("Coins collected: %d", coinManager.getCollectedCount());
        ImGui::SliderFloat("Master Volume", &masterVolume, 0.f, 1.f);
        if (ImGui::IsItemEdited()) {
            audioListener.setMasterVolume(masterVolume);
        }
        ImGui::End();
    });
    // Capture mouse so camera orbit doesn't move while interacting with UI
    IOCapture imguiCapture{};
    imguiCapture.preventMouseEvent = [] { return ImGui::GetIO().WantCaptureMouse; };
    canvas.setIOCapture(&imguiCapture);
    audioListener.setMasterVolume(masterVolume);

    // --- Reset game state function ---
    std::function<void()> resetGameState = [&]() {
        if (isResetting) return; // Prevent multiple resets
        std::cout << "Starting reset fade..." << std::endl;
        isResetting = true;
        resetFadeTimer = 0.0f;
    };
    
    // Actual reset logic (called when fade is complete)
    auto performReset = [&]() {
        std::cout << "Performing reset..." << std::endl;
        
        // Clear particles
        particleSystem.clearParticles();
        
        // Clear debug objects
        for (auto& obj : debugObjects) {
            if (obj) {
                world.scene().remove(*obj);
            }
        }
        debugObjects.clear();
        showCollisionDebug = false;
        
        // Reset gameplay counters
        digScoops = 0;
        pileGone = false;
        
        // Re-add dig pile visual if it was removed
        auto pileVisual = digZone.getVisual();
        if (pileVisual && !pileVisual->parent) {
            world.scene().add(pileVisual);
        }
        
        // Reset zones
        digZone.reset();
        CollisionWorld::updateLastRockMeshColliderFromObject(*digZone.getVisual());
        dumpZone.reset();
        
        // Reset coins
        coinManager.reset();
        coinManager.spawnCoins(15, spawnConfig.arenaRadius);
        
        // Reset excavator
        excavator.reset();
        
        // Reset camera
        cameraAngleH = 0.0f;
        cameraAngleV = threepp::math::PI * 0.35f;
        
        // Reset control states
        wDown = sDown = aDown = dDown = false;
        qDown = eDown = false;
        rDown = fDown = false;
        tDown = gDown = false;
        yDown = hDown = false;
        flipStick = flipBucket = false;
        nudgeStickPivotPos = nudgeStickPivotNeg = false;
        nudgeBucketPivotPos = nudgeBucketPivotNeg = false;
        isMouseButtonDown = false;
        
        // Restart engine audio
        audioManager.startEngine();
        
        std::cout << "Game state reset complete!" << std::endl;
    };

    // Key listeners
    // --- KeyPress listener ---
    KeyAdapter keyPress(KeyAdapter::KEY_PRESSED, [&](KeyEvent evt) {
        switch (evt.key) {
            case Key::W: wDown = true; break;
            case Key::S: sDown = true; break;
            case Key::A: aDown = true; break;
            case Key::D: dDown = true; break;
            case Key::Q: qDown = true; break;
            case Key::E: eDown = true; break;
            case Key::R: rDown = true; break;
            case Key::F: fDown = true; break;
            case Key::T: tDown = true; break;
            case Key::G: gDown = true; break;
            case Key::Y: yDown = true; break;
            case Key::H: hDown = true; break;
            case Key::U: flipStick = true; break;
            case Key::I: flipBucket = true; break;
            case Key::J: nudgeStickPivotNeg = true; break;    // move stick pivot -Z
            case Key::K: nudgeStickPivotPos = true; break;    // move stick pivot +Z
            case Key::N: nudgeBucketPivotNeg = true; break;   // move bucket pivot -Z
            case Key::M: nudgeBucketPivotPos = true; break;   // move bucket pivot +Z
            case Key::Z: pivotStep = std::max(0.01f, pivotStep * 0.5f); std::cout << "pivotStep=" << pivotStep << "\n"; break;
            case Key::X: pivotStep = std::min(100.0f, pivotStep * 2.0f); std::cout << "pivotStep=" << pivotStep << "\n"; break;
            case Key::V: 
                showCollisionDebug = !showCollisionDebug;
                std::cout << "Collision debug: " << (showCollisionDebug ? "ON" : "OFF") << "\n";
                break;
            case Key::P:
                resetGameState();
                break;
            default: break;
        }
    });

    KeyAdapter keyRelease(KeyAdapter::KEY_RELEASED, [&](KeyEvent evt) {
        switch (evt.key) {
            case Key::W: wDown = false; break;
            case Key::S: sDown = false; break;
            case Key::A: aDown = false; break;
            case Key::D: dDown = false; break;
            case Key::Q: qDown = false; break;
            case Key::E: eDown = false; break;
            case Key::R: rDown = false; break;
            case Key::F: fDown = false; break;
            case Key::T: tDown = false; break;
            case Key::G: gDown = false; break;
            case Key::Y: yDown = false; break;
            case Key::H: hDown = false; break;
            case Key::U: flipStick = false; break;
            case Key::I: flipBucket = false; break;
            case Key::J: nudgeStickPivotNeg = false; break;
            case Key::K: nudgeStickPivotPos = false; break;
            case Key::N: nudgeBucketPivotNeg = false; break;
            case Key::M: nudgeBucketPivotPos = false; break;
            case Key::Z: /* handled on press */ break;
            case Key::X: /* handled on press */ break;
            default: break;
        }
    });

    canvas.addKeyListener(keyPress);
    canvas.addKeyListener(keyRelease);

    // --- Mouse listeners for camera orbit ---
    struct CameraOrbitListener : MouseListener {
        bool& isMouseButtonDown;
        Vector2& lastMousePos;
        float& cameraAngleH;
        float& cameraAngleV;
        
        CameraOrbitListener(bool& mouseDown, Vector2& lastPos, float& angleH, float& angleV)
            : isMouseButtonDown(mouseDown), lastMousePos(lastPos), 
              cameraAngleH(angleH), cameraAngleV(angleV) {}
        
        void onMouseDown(int button, const Vector2& pos) override {
            if (button == 0) {  // Left mouse button
                isMouseButtonDown = true;
                lastMousePos = pos;
            }
        }
        
        void onMouseUp(int button, const Vector2& /*pos*/) override {
            if (button == 0) {
                isMouseButtonDown = false;
            }
        }
        
        void onMouseMove(const Vector2& pos) override {
            if (isMouseButtonDown) {
                Vector2 delta = pos - lastMousePos;
                
                // Update camera angles based on mouse movement
                float sensitivity = 0.005f;
                cameraAngleH += delta.x * sensitivity;  // horizontal rotation (reversed)
                // Keep camera in top hemisphere: cap at just above horizon (PI/2)
                const float vMin = 0.1f;
                const float vMax = PI * 0.5f - 0.05f; // avoid dipping below ground plane
                cameraAngleV = std::clamp(cameraAngleV - delta.y * sensitivity, vMin, vMax);
                
                lastMousePos = pos;
            }
        }
    };
    
    CameraOrbitListener orbitListener(isMouseButtonDown, lastMousePos, cameraAngleH, cameraAngleV);
    canvas.addMouseListener(orbitListener);

    // --- Animate ---
    logFile << "[loop] starting animate" << std::endl;
    canvas.animate([&] {
        float dt = clock.getDelta();

        // --- Handle reset fade animation ---
        if (isResetting) {
            resetFadeTimer += dt;
            
            if (resetFadeTimer < fadeDuration) {
                // Fade to white (0.0 -> 1.0)
                fadeOpacity = resetFadeTimer / fadeDuration;
            } else if (resetFadeTimer < fadeDuration * 2.0f) {
                // Perform reset at peak fade
                if (resetFadeTimer - dt < fadeDuration) {
                    performReset();
                }
                // Fade from white (1.0 -> 0.0)
                float fadeProgress = (resetFadeTimer - fadeDuration) / fadeDuration;
                fadeOpacity = 1.0f - fadeProgress;
            } else {
                // Fade complete
                fadeOpacity = 0.0f;
                isResetting = false;
                resetFadeTimer = 0.0f;
            }
            
            overlayMaterial->opacity = fadeOpacity;
        }

        // --- Track control (W/S both tracks, A/D differential) ---
        float leftSpeed = 0.f, rightSpeed = 0.f;
        // W = forward, S = reverse
        if (wDown) { leftSpeed += 2.0f; rightSpeed += 2.0f; }
        if (sDown) { leftSpeed -= 2.0f; rightSpeed -= 2.0f; }
        if (aDown) { leftSpeed -= 1.0f; rightSpeed += 1.0f; } // Turn left
        if (dDown) { leftSpeed += 1.0f; rightSpeed -= 1.0f; } // Turn right

        excavator.setTracksSpeed(leftSpeed, rightSpeed);

        // --- Update audio engine state (startup -> idle transition) ---
        audioManager.updateEngine(dt);
        
        // --- Idle volume scales with movement effort ---
        // Max instantaneous motor command can reach 3.0 (2 forward + 1 turn)
        const float maxMotor = 3.0f;
        float effort = std::max(std::abs(leftSpeed), std::abs(rightSpeed)) / maxMotor;
        effort = std::clamp(effort, 0.0f, 1.0f);
        const float idleVol = 0.18f; // audible idle
        const float fullVol = 0.55f; // cap to avoid clipping
        idleTargetVolume = idleVol + (fullVol - idleVol) * effort;
        // Exponential smoothing to avoid abrupt changes/clicks
        float alpha = 1.0f - std::exp(-dt * 8.0f); // ~125ms time constant
        idleVolumeSmoothed += (idleTargetVolume - idleVolumeSmoothed) * alpha;
        audioManager.setIdleVolume(idleVolumeSmoothed);

        // --- Turret control (Q/E) ---
        float turretSpeed = 1.0f; // rad/s
        float turretYaw = excavator.getTurretYaw();
        if (qDown) turretYaw += turretSpeed * dt;
        if (eDown) turretYaw -= turretSpeed * dt;
        excavator.setTurretYaw(turretYaw);

        // --- Boom control (R/F) ---
        float boomSpeed = 0.5f; // rad/s
        float boomAngle = excavator.getBoomAngle();
        boomMovingUp = rDown;
        boomMovingDown = fDown;
        if (rDown) boomAngle += boomSpeed * dt;
        if (fDown) boomAngle -= boomSpeed * dt;
        excavator.setBoomAngle(boomAngle);
        
        // Only play sound if boom actually moved
        float newBoomAngle = excavator.getBoomAngle();
        bool boomMoved = std::abs(newBoomAngle - prevBoomAngle) > 0.0001f;
        if (rDown && boomMoved) {
            audioManager.playHydraulics(0.5f); // Boom: loudest
        } else if (rDown && !boomMoved) {
            audioManager.stopHydraulics();
        }
        if (fDown && boomMoved) {
            audioManager.playSteam(0.45f); // Boom: loudest
        } else if (fDown && !boomMoved) {
            audioManager.stopSteam();
        }
        if (!rDown && !tDown && !yDown) {
            audioManager.stopHydraulics();
        }
        if (!fDown && !gDown && !hDown) {
            audioManager.stopSteam();
        }
        prevBoomAngle = newBoomAngle;

        // --- Stick control (T/G) ---
        float stickSpeed = 0.5f;
        float stickAngle = excavator.getStickAngle();
        stickMovingUp = tDown;
        stickMovingDown = gDown;
        if (tDown) stickAngle += stickSpeed * dt;
        if (gDown) stickAngle -= stickSpeed * dt;
        excavator.setStickAngle(stickAngle);
        
        // Only play sound if stick actually moved
        float newStickAngle = excavator.getStickAngle();
        bool stickMoved = std::abs(newStickAngle - prevStickAngle) > 0.0001f;
        if (tDown && !rDown && stickMoved) {
            audioManager.playHydraulics(0.35f); // Stick: medium volume
        } else if (tDown && !rDown && !stickMoved) {
            audioManager.stopHydraulics();
        }
        if (gDown && !fDown && stickMoved) {
            audioManager.playSteam(0.30f); // Stick: medium volume
        } else if (gDown && !fDown && !stickMoved) {
            audioManager.stopSteam();
        }
        prevStickAngle = newStickAngle;

        // --- Bucket control (Y/H) ---
        float bucketSpeed = 0.5f;
        float bucketAngle = excavator.getBucketAngle();
        bucketMovingUp = yDown;
        bucketMovingDown = hDown;
        if (yDown) bucketAngle += bucketSpeed * dt;
        if (hDown) bucketAngle -= bucketSpeed * dt;
        excavator.setBucketAngle(bucketAngle);
        
        // Only play sound if bucket actually moved
        float newBucketAngle = excavator.getBucketAngle();
        bool bucketMoved = std::abs(newBucketAngle - prevBucketAngle) > 0.0001f;
        if (yDown && !rDown && !tDown && bucketMoved) {
            audioManager.playHydraulics(0.25f); // Bucket: quietest
        } else if (yDown && !rDown && !tDown && !bucketMoved) {
            audioManager.stopHydraulics();
        }
        if (hDown && !fDown && !gDown && bucketMoved) {
            audioManager.playSteam(0.22f); // Bucket: quietest
        } else if (hDown && !fDown && !gDown && !bucketMoved) {
            audioManager.stopSteam();
        }
        prevBucketAngle = newBucketAngle;

    // Debug: alignment controls
    if (flipStick) { excavator.flipStickHingeEnd(); flipStick = false; }
    if (flipBucket) { excavator.flipBucketHingeEnd(); flipBucket = false; }
            // Use larger step (world units); conversion to local is handled inside Excavator
            if (nudgeStickPivotPos) { excavator.nudgeStickPivotZ(+0.1f); }
            if (nudgeStickPivotNeg) { excavator.nudgeStickPivotZ(-0.1f); }
            if (nudgeBucketPivotPos) { excavator.nudgeBucketPivotZ(+0.1f); }
            if (nudgeBucketPivotNeg) { excavator.nudgeBucketPivotZ(-0.1f); }

    // Update excavator (track animation)
        excavator.update(dt);
        
        // Update track marks before coin collection (uses excavator position)
        trackMarks.update(dt, excavator.root()->position);

        // Update & collect coins
        coinManager.update(dt);
        Vector3 excavatorPos = excavator.root()->position;
        if (coinManager.checkCollection(excavatorPos, 3.0f)) {
            audioManager.playCoin();
        }
        
        // Update particle system
        particleSystem.update(dt);
        
        // --- Dig/Dump Gameplay Logic ---
        Vector3 bucketPos = excavator.getBucketWorldPosition();
        
        // Check if bucket is in dig zone and not loaded
        if (!pileGone && !excavator.isBucketLoaded() && digZone.isInZone(bucketPos)) {
            excavator.loadBucket();
            // Shrink the dig pile a bit and update its collider hull
            // Aim for ~5 scoops to nearly clear the pile (down to ~5% scale)
            // Slightly faster: ~5 scoops target, a touch stronger than 0.20
            // f=0.21 => per-scoop scale=0.79, volume factor ≈ 0.79^3 ≈ 0.493
            const float digFraction = 0.21f;
            if (digZone.dig(digFraction)) {
                CollisionWorld::updateLastRockMeshColliderFromObject(*digZone.getVisual());
            }
            
            // Play dig sound
            audioManager.playDig();
            
            digScoops++;
            if (digScoops >= 5 && !pileGone) {
                pileGone = true;
                // Remove visual and collider so the pile fully disappears
                world.scene().remove(*digZone.getVisual());
                CollisionWorld::popLastRockMeshCollider();
            }
        }
        
        // Check if bucket is in dump zone and loaded
        if (excavator.isBucketLoaded() && dumpZone.isInZone(bucketPos)) {
            excavator.unloadBucket();
            dumpZone.recordDump();
            
            // Play dump sound
            audioManager.playDump();
        }

        // Debug visualization
        if (showCollisionDebug) {
            try {
                CollisionWorld::debugDrawRockHulls(world.scene(), debugObjects);
                CollisionWorld::debugDrawExcavatorHulls(world.scene(),
                    excavator.baseMesh(),
                    excavator.bodyMesh(),
                    excavator.boomMesh(),
                    excavator.stickMesh(),
                    excavator.bucketMesh(),
                    debugObjects);
            } catch (const std::exception& e) {
                std::cerr << "Debug draw error: " << e.what() << std::endl;
                showCollisionDebug = false;
            }
        } else if (!debugObjects.empty()) {
            for (auto& obj : debugObjects) {
                world.scene().remove(*obj);
            }
            debugObjects.clear();
        }

        // --- Update camera position (orbit around excavator) ---
        Vector3 target = excavator.root()->position; // Follow excavator
        target.y += 1.0f; // look slightly above base
        camera.position.x = target.x + cameraDistance * std::sin(cameraAngleV) * std::cos(cameraAngleH);
        camera.position.y = target.y + cameraDistance * std::cos(cameraAngleV);
        camera.position.z = target.z + cameraDistance * std::sin(cameraAngleV) * std::sin(cameraAngleH);
        camera.lookAt(target);

        renderer.render(world.scene(), camera);
        ui.render();
        
        // Render fade overlay on top if active
        if (fadeOpacity > 0.0f) {
            renderer.setAutoClear(false); // Don't clear before overlay
            renderer.render(overlayScene, overlayCamera);
            renderer.setAutoClear(true); // Re-enable for next frame
        }
        
        // heartbeat
        static double acc = 0.0;
        acc += dt;
        if (acc > 1.0) { logFile << "[loop] 1s tick" << std::endl; acc = 0.0; }
    });
    logFile << "[end] exited animate" << std::endl;
    logFile.close();

    return 0;
}
