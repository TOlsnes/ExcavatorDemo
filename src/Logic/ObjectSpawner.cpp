
#include "ObjectSpawner.hpp"
#include <cmath>
#include <numbers>
#include <iostream>
#include <threepp/loaders/TextureLoader.hpp>
#include <threepp/loaders/OBJLoader.hpp>
#include <filesystem>
#include <string>
#include "CollisionWorld.hpp"
#include "Settings.hpp"

using namespace threepp;
using namespace Settings;

namespace {
    std::string resolveAssetPath(const std::string& rel) {
        using std::filesystem::exists;
        using std::filesystem::path;
        const path candidates[] = {
            path(rel),
            path("../") / rel,
            path("../../") / rel,
            path("../../../") / rel
        };
        for (const auto& p : candidates) {
            if (exists(p)) return p.string();
        }
        return rel; // fallback; loader will report error
    }
}

ObjectSpawner::ObjectSpawner(Scene& scene, const SpawnConfig& config)
    : scene_(scene), config_(config), rng_(config.randomSeed) {}

void ObjectSpawner::generateEnvironment() {
    // Reset colliders when regenerating
    CollisionWorld::clear();
    spawnGroundPlane_();
    spawnPerimeterRocks_();
}

void ObjectSpawner::spawnGroundPlane_() {
    // Circle as a ground plane so you dont see random corners when peeking over hte rocks
    auto groundGeometry = CircleGeometry::create(config_.arenaRadius * 1.5f, 64);
    auto groundMaterial = MeshStandardMaterial::create();
    groundMaterial->roughness = 0.9f;

    // Load and tile the PNG for ground
    threepp::TextureLoader loader;
    auto texture = loader.load(resolveAssetPath("models/0pfla2.png"), true);
    if (texture) {
        texture->wrapS = threepp::TextureWrapping::Repeat;
        texture->wrapT = threepp::TextureWrapping::Repeat;
        texture->repeat.set(20, 20); // Tile 20x20 times so its not too repetetive but not toomuch cus its pretty low quality lol
        groundMaterial->map = texture;
        groundMaterial->color = threepp::Color(1, 1, 1); // No tint
        groundMaterial->needsUpdate();
    } else {
        groundMaterial->color = Color(0.4f, 0.35f, 0.3f); // Fallback brown
    }

    auto ground = Mesh::create(groundGeometry, groundMaterial);
    ground->rotation.x = -math::PI / 2.0f; // Flat on XZ plane
    ground->receiveShadow = true;
    scene_.add(ground);
}

void ObjectSpawner::spawnPerimeterRocks_() {
    std::cout << "Loading perimeter rock from models/Rock2.obj (with .mtl)...\n";
    OBJLoader loader;
    auto rockGroup = loader.load(resolveAssetPath("models/Rock2.obj"), true);
    if (!rockGroup) {
        std::cout << "Failed to load Rock2.obj. Skipping perimeter rocks.\n";
        return;
    }

    // The exported MTL sets Kd (diffuse) to black and has no texture.
    // Override with a rock-like PBR material so meshes aren't black.
    auto rockMat = MeshStandardMaterial::create();
    rockMat->color = Color(0.6f, 0.6f, 0.6f);
    rockMat->roughness = 0.95f;
    rockMat->metalness = 0.0f;
    rockGroup->traverseType<Mesh>([&](Mesh& m) {
        m.setMaterial(rockMat);
        m.castShadow = true;
        m.receiveShadow = true;
    });

    const int rockCount = 20; // a few more for coverage
    const float perimeterRadius = config_.arenaRadius * 1.0f; // bring closer to plane edge
    const float rockScale = 0.5f; // much larger scale

    for (int i = 0; i < rockCount; ++i) {
        float angle = (i / static_cast<float>(rockCount)) * 2.0f * math::PI; //Use math::PI instead of Settings::PI as it was causing errors
        float x = std::cos(angle) * perimeterRadius;
        float z = std::sin(angle) * perimeterRadius;

        auto rock = rockGroup->clone<Object3D>(true);
        rock->position.set(x, 0, z);
        rock->scale.setScalar(rockScale);
        
        // Random rotation (i mean this doesnt rly matter anymore since i changed it to be circular rocks but whatever ig)
        std::uniform_real_distribution<float> dist(0.0f, 2.0f * math::PI);
        rock->rotation.y = dist(rng_);

        scene_.add(rock);
        // Register a mesh-based collider that matches the rock footprint
        CollisionWorld::addRockMeshColliderFromObject(*rock);
    }

    std::cout << "Perimeter rocks placed using Rock1: " << rockCount << "\n";
}
