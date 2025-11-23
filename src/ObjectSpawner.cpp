
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
    // Large circular ground plane
    auto groundGeometry = CircleGeometry::create(config_.arenaRadius * 1.5f, 64);
    auto groundMaterial = MeshStandardMaterial::create();
    groundMaterial->roughness = 0.9f;

    // Load and tile the PNG texture
    threepp::TextureLoader loader;
    auto texture = loader.load(resolveAssetPath("models/0pfla2.png"), true);
    if (texture) {
        texture->wrapS = threepp::TextureWrapping::Repeat;
        texture->wrapT = threepp::TextureWrapping::Repeat;
        texture->repeat.set(20, 20); // Tile 20x20 times
        groundMaterial->map = texture;
        groundMaterial->color = threepp::Color(1, 1, 1); // No tint
        groundMaterial->needsUpdate();
    } else {
        groundMaterial->color = Color(0.4f, 0.35f, 0.3f); // Fallback brown
    }

    auto ground = Mesh::create(groundGeometry, groundMaterial);
    ground->rotation.x = -Settings::PI / 2.0f; // Flat on XZ plane
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
        float angle = (i / static_cast<float>(rockCount)) * 2.0f * Settings::PI;
        float x = std::cos(angle) * perimeterRadius;
        float z = std::sin(angle) * perimeterRadius;

        auto rock = rockGroup->clone<Object3D>(true);
        rock->position.set(x, 0, z);
        rock->scale.setScalar(rockScale);
        rock->rotation.y = randomRange_(0.0f, 2.0f * Settings::PI);

        scene_.add(rock);
        // Register a mesh-based collider that matches the rock footprint
        CollisionWorld::addRockMeshColliderFromObject(*rock);
    }

    std::cout << "Perimeter rocks placed using Rock1: " << rockCount << "\n";
}

void ObjectSpawner::spawnSmallDebris_() {
    // Small rocks and debris scattered around
    for (int i = 0; i < config_.smallObjectCount; ++i) {
        // Random position within arena
        float angle = randomRange_(0.0f, 2.0f * Settings::PI);
        float distance = randomRange_(2.0f, config_.arenaRadius - 2.0f);
        float x = std::cos(angle) * distance;
        float z = std::sin(angle) * distance;
        
        // Random small object
        std::shared_ptr<Mesh> obj;
        int type = static_cast<int>(randomRange_(0.0f, 3.0f));
        
        if (type == 0) {
            // Small rock (sphere)
            float radius = randomRange_(0.1f, 0.3f);
            obj = createSphere_(radius);
            obj->position.y = radius;
        } else if (type == 1) {
            // Debris chunk (box)
            float size = randomRange_(0.1f, 0.4f);
            obj = createBox_(size, size * 0.8f, size * 1.2f);
            obj->position.y = size * 0.4f;
        } else {
            // Small cylinder (pipe piece)
            float radius = randomRange_(0.08f, 0.15f);
            float height = randomRange_(0.2f, 0.5f);
            obj = createCylinder_(radius, radius, height);
            obj->position.y = height * 0.5f;
            obj->rotation.z = randomRange_(-0.3f, 0.3f);
        }
        
        obj->position.x = x;
        obj->position.z = z;
        obj->rotation.y = randomRange_(0.0f, 2.0f * Settings::PI);
        obj->castShadow = true;
        obj->receiveShadow = true;
        
        scene_.add(obj);
        objects_.push_back(obj);
    }
}

void ObjectSpawner::spawnMediumObjects_() {
    // Barrels, crates, larger rocks
    for (int i = 0; i < config_.mediumObjectCount; ++i) {
        float angle = randomRange_(0.0f, 2.0f * Settings::PI);
        float distance = randomRange_(5.0f, config_.arenaRadius - 3.0f);
        float x = std::cos(angle) * distance;
        float z = std::sin(angle) * distance;
        
        std::shared_ptr<Mesh> obj;
        int type = static_cast<int>(randomRange_(0.0f, 3.0f));
        
        if (type == 0) {
            // Barrel (cylinder)
            obj = createCylinder_(0.3f, 0.3f, 0.8f);
            obj->position.y = 0.4f;
        } else if (type == 1) {
            // Crate (box)
            float size = randomRange_(0.5f, 0.8f);
            obj = createBox_(size, size, size);
            obj->position.y = size * 0.5f;
        } else {
            // Large rock (sphere)
            float radius = randomRange_(0.4f, 0.7f);
            obj = createSphere_(radius);
            obj->position.y = radius;
        }
        
        obj->position.x = x;
        obj->position.z = z;
        obj->rotation.y = randomRange_(0.0f, 2.0f * Settings::PI);
        obj->castShadow = true;
        obj->receiveShadow = true;
        
        scene_.add(obj);
        objects_.push_back(obj);
    }
}

void ObjectSpawner::spawnLargeObstacles_() {
    // Large boulders and concrete blocks
    for (int i = 0; i < config_.largeObjectCount; ++i) {
        // Place in a rough circle pattern
        float angle = (i / static_cast<float>(config_.largeObjectCount)) * 2.0f * Settings::PI;
        angle += randomRange_(-0.3f, 0.3f);
        float distance = config_.arenaRadius * 0.6f + randomRange_(-3.0f, 3.0f);
        float x = std::cos(angle) * distance;
        float z = std::sin(angle) * distance;
        
        std::shared_ptr<Mesh> obj;
        int type = static_cast<int>(randomRange_(0.0f, 2.0f));
        
        if (type == 0) {
            // Large boulder (sphere)
            float radius = randomRange_(1.0f, 1.5f);
            obj = createSphere_(radius);
            obj->position.y = radius;
        } else {
            // Concrete block (box)
            float sizeX = randomRange_(1.2f, 2.0f);
            float sizeY = randomRange_(0.8f, 1.5f);
            float sizeZ = randomRange_(1.2f, 2.0f);
            obj = createBox_(sizeX, sizeY, sizeZ);
            obj->position.y = sizeY * 0.5f;
        }
        
        obj->position.x = x;
        obj->position.z = z;
        obj->rotation.y = randomRange_(0.0f, 2.0f * Settings::PI);
        obj->castShadow = true;
        obj->receiveShadow = true;
        
        scene_.add(obj);
        objects_.push_back(obj);
    }
}

std::shared_ptr<Mesh> ObjectSpawner::createBox_(float sizeX, float sizeY, float sizeZ) {
    auto geometry = BoxGeometry::create(sizeX, sizeY, sizeZ);
    auto material = MeshStandardMaterial::create();
    material->color = randomColor_();
    material->roughness = randomRange_(0.6f, 0.9f);
    material->metalness = randomRange_(0.0f, 0.3f);
    return Mesh::create(geometry, material);
}

std::shared_ptr<Mesh> ObjectSpawner::createSphere_(float radius) {
    auto geometry = SphereGeometry::create(radius, 16, 12);
    auto material = MeshStandardMaterial::create();
    material->color = randomColor_();
    material->roughness = randomRange_(0.7f, 1.0f);
    material->metalness = randomRange_(0.0f, 0.2f);
    return Mesh::create(geometry, material);
}

std::shared_ptr<Mesh> ObjectSpawner::createCylinder_(float radiusTop, float radiusBottom, float height) {
    auto geometry = CylinderGeometry::create(radiusTop, radiusBottom, height, 16);
    auto material = MeshStandardMaterial::create();
    material->color = randomColor_();
    material->roughness = randomRange_(0.5f, 0.8f);
    material->metalness = randomRange_(0.1f, 0.4f);
    return Mesh::create(geometry, material);
}

float ObjectSpawner::randomRange_(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng_);
}

Color ObjectSpawner::randomColor_() {
    // Earthy tones for debris: browns, grays, rusted reds
    int palette = static_cast<int>(randomRange_(0.0f, 4.0f));
    
    switch (palette) {
        case 0: // Brown/tan
            return Color(randomRange_(0.4f, 0.6f), randomRange_(0.3f, 0.5f), randomRange_(0.2f, 0.3f));
        case 1: // Gray
            return Color(randomRange_(0.3f, 0.6f), randomRange_(0.3f, 0.6f), randomRange_(0.3f, 0.6f));
        case 2: // Rust red
            return Color(randomRange_(0.5f, 0.7f), randomRange_(0.2f, 0.4f), randomRange_(0.1f, 0.2f));
        default: // Dark green (military crates)
            return Color(randomRange_(0.2f, 0.4f), randomRange_(0.3f, 0.5f), randomRange_(0.2f, 0.3f));
    }
}
