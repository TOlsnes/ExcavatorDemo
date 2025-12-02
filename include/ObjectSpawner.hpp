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
        float arenaRadius = 30.0f;          // Radius of circular spawn area
        int smallObjectCount = 100;         // Small debris (rocks, chunks)
        int mediumObjectCount = 30;         // Medium objects (barrels, crates)
        int largeObjectCount = 8;           // Large obstacles (boulders, blocks)
        unsigned int randomSeed = 12345;    // For reproducible generation
    };

    explicit ObjectSpawner(threepp::Scene& scene);
    ObjectSpawner(threepp::Scene& scene, const SpawnConfig& config);

    // Generate and place all environment objects
    void generateEnvironment();

private:
    void spawnGroundPlane_();
    void spawnPerimeterRocks_();

    threepp::Scene& scene_;
    SpawnConfig config_;
    std::mt19937 rng_;
};
