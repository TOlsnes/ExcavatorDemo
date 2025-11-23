#pragma once

#include <threepp/threepp.hpp>
#include <vector>
#include <memory>
#include <random>

/**
 * ObjectSpawner: Procedurally generates environment objects (rocks, debris, crates)
 * in a circular arena pattern for the excavator to interact with.
 */
class ObjectSpawner {
public:
    struct SpawnConfig {
        float arenaRadius{30.0f};          // Radius of circular spawn area
        int smallObjectCount{100};         // Small debris (rocks, chunks)
        int mediumObjectCount{30};         // Medium objects (barrels, crates)
        int largeObjectCount{8};           // Large obstacles (boulders, blocks)
        unsigned int randomSeed{12345};    // For reproducible generation
    };

    ObjectSpawner(threepp::Scene& scene, const SpawnConfig& config = SpawnConfig{});

    // Generate and place all environment objects
    void generateEnvironment();

    // Get all spawned objects (for later physics integration)
    const std::vector<std::shared_ptr<threepp::Mesh>>& getObjects() const { return objects_; }

private:
    void spawnGroundPlane_();
    void spawnPerimeterRocks_();
    void spawnArenaWalls_();
    void spawnSmallDebris_();
    void spawnMediumObjects_();
    void spawnLargeObstacles_();

    // Helper: create simple geometry with random material
    std::shared_ptr<threepp::Mesh> createBox_(float sizeX, float sizeY, float sizeZ);
    std::shared_ptr<threepp::Mesh> createSphere_(float radius);
    std::shared_ptr<threepp::Mesh> createCylinder_(float radiusTop, float radiusBottom, float height);

    // Random helpers
    float randomRange_(float min, float max);
    threepp::Color randomColor_();

    threepp::Scene& scene_;
    SpawnConfig config_;
    std::mt19937 rng_;
    std::vector<std::shared_ptr<threepp::Mesh>> objects_;
};
