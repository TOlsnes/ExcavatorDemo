#pragma once

#include <threepp/scenes/Scene.hpp>
#include <threepp/objects/Mesh.hpp>
#include <threepp/geometries/PlaneGeometry.hpp>
#include <threepp/materials/MeshStandardMaterial.hpp>
#include <vector>
#include <memory>

class TrackMarkManager {
public:
    explicit TrackMarkManager(threepp::Scene& scene);

    // Call each frame with dt and current excavator world position
    void update(float dt, const threepp::Vector3& excavatorPos);

    // Adjust spawning behavior
    void setSpawnDistance(float d) { spawnDistance_ = d; }
    void setLifetime(float seconds) { lifetime_ = seconds; }
    void setMarkDimensions(float width, float length) { markWidth_ = width; markLength_ = length; }
    void setTrackSeparation(float sep) { trackSeparation_ = sep; }

private:
    struct Mark {
        std::shared_ptr<threepp::Mesh> mesh;
        float timeRemaining;
    };

    threepp::Scene& scene_;
    std::vector<Mark> marks_;
    threepp::Vector3 lastSpawnPos_{};
    bool hasSpawnedFirst_ = false;
    float distanceAccumulator_ = 0.f;
    float spawnDistance_ = 0.6f; // meters between marks
    float lifetime_ = 3.f;       // seconds a mark persists
    float markWidth_ = 0.25f;    // default individual track imprint width (smaller)
    float markLength_ = 0.25f;   // default imprint length along travel (smaller)
    float trackSeparation_ = 1.0f; // distance between track centers

    void spawnMark(const threepp::Vector3& pos, const threepp::Vector3& forwardDir);
};
