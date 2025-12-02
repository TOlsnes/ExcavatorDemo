#pragma once

#include <threepp/scenes/Scene.hpp>
#include <threepp/objects/Mesh.hpp>
#include <threepp/geometries/PlaneGeometry.hpp>
#include <threepp/materials/MeshStandardMaterial.hpp>
#include <vector>
#include <memory>
#include "Settings.hpp"

class TrackMarkManager {
public:
    explicit TrackMarkManager(threepp::Scene& scene);

    // Call each frame with dt and current excavator world position
    void update(float dt, const threepp::Vector3& excavatorPos);

    // Adjust spawning behavior
    void setSpawnDistance(float d) { Settings::spawnDistance_ = d; }
    void setLifetime(float seconds) { Settings::lifetime_ = seconds; }

private:
    struct Mark {
        std::shared_ptr<threepp::Mesh> mesh;
        float timeRemaining;
    };

    threepp::Scene& scene_;
    std::vector<Mark> marks_;
    threepp::Vector3 lastSpawnPos_{};
    bool hasSpawnedFirst_ = false;
    void spawnMark(const threepp::Vector3& pos, const threepp::Vector3& forwardDir);
};
