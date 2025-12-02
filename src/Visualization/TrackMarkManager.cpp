#include "TrackMarkManager.hpp"
#include "Settings.hpp"
#include <threepp/math/MathUtils.hpp>
#include <threepp/math/Vector3.hpp>
#include <cmath>

TrackMarkManager::TrackMarkManager(threepp::Scene& scene): scene_(scene) {}

void TrackMarkManager::update(float dt, const threepp::Vector3& excavatorPos) {
    // Update lifetimes & remove expired
    for (auto it = marks_.begin(); it != marks_.end(); ) {
        it->timeRemaining -= dt;
        // Fade opacity based on remaining lifetime (just same shit as particles really)
        if (auto stdMat = std::dynamic_pointer_cast<threepp::MeshStandardMaterial>(it->mesh->material())) {
            float t = std::max(0.f, it->timeRemaining) / Settings::lifetime_;
            float alpha = t * t; // smoother end fade
            stdMat->transparent = true;
            stdMat->opacity = alpha;
        }
        if (it->timeRemaining <= 0.f) {
            scene_.remove(*it->mesh);
            it = marks_.erase(it);
        } else {
            ++it;
        }
    }

    // Handle spawning based on travelled distance on XZ plane
    if (!hasSpawnedFirst_) {
        lastSpawnPos_.copy(excavatorPos);
        hasSpawnedFirst_ = true;
        return; // wait for movement
    }

    threepp::Vector3 delta = excavatorPos - lastSpawnPos_;
    threepp::Vector3 deltaXZ(delta.x, 0.f, delta.z);
    float moved = deltaXZ.length();
    if (moved <= 0.0001f) return;

    Settings::distanceAccumulator_ += moved;

    // Forward direction from movement
    threepp::Vector3 forwardDir = deltaXZ;
    forwardDir.normalize();

    if (Settings::distanceAccumulator_ >= Settings::spawnDistance_) {
        // Spawn at current position minus a small offset so theyre behind the tracks
        threepp::Vector3 spawnPos = excavatorPos - forwardDir * 0.2f;
        spawnMark(spawnPos, forwardDir);
        Settings::distanceAccumulator_ = 0.f;
        lastSpawnPos_.copy(excavatorPos);
    }
}

void TrackMarkManager::spawnMark(const threepp::Vector3& pos, const threepp::Vector3& forwardDir) {
    // Spawn two separate narrow rectangles: left & right tracks
    // Parameters tuned to excavator dimensions (trackWidth_ ≈ 1.0)
    const float markWidth = Settings::markWidth_ * 0.18f;  // 30% of previous (0.6 * markWidth_)
    const float markLength = Settings::markLength_;        // keep length
    // Inward offset: start from half separation then nudge inward by half of (original) width
    float offsetBase = Settings::trackSeparation_ * 0.5f;
    float inwardNudge = Settings::markWidth_ * 0.5f; // use original width for symmetry
    float offsetDist = offsetBase - inwardNudge;
    if (offsetDist < 0.05f) offsetDist = 0.05f; // avoid overlap

    // Compute lateral (right) direction from forward
    threepp::Vector3 rightDir(forwardDir.z, 0.f, -forwardDir.x); // perpendicular on XZ
    rightDir.normalize();

    // Track world positions
    threepp::Vector3 leftPos = pos - rightDir * offsetDist;
    threepp::Vector3 rightPos = pos + rightDir * offsetDist;

    auto createMarkMesh = [&](const threepp::Vector3& mpos) {
        auto geom = threepp::PlaneGeometry::create(markWidth, markLength);
        // Correct orientation: rotate -90° so plane lies in XZ and normal points up
        geom->rotateX(-threepp::math::PI / 2.0f);
        auto mat = threepp::MeshStandardMaterial::create();
        mat->color.setHex(0x333333);
        mat->roughness = 1.0f;
        mat->metalness = 0.0f;
        mat->transparent = true; // allow fading
        mat->opacity = 1.0f;
        auto mesh = threepp::Mesh::create(geom, mat);
        // Derive excavator base yaw from movement direction.
        // forwardDir = (-cos(baseYaw), sin(baseYaw)) in XZ from Excavator::update logic.
        // Recover baseYaw: cos(baseYaw) = -forwardDir.x, sin(baseYaw) = forwardDir.z
        float yaw = std::atan2(forwardDir.z, -forwardDir.x);
        mesh->rotation.y = yaw;
        mesh->position.set(mpos.x, 0.02f, mpos.z); // slight lift to avoid z-fighting
        scene_.add(mesh);
        marks_.push_back({mesh, Settings::lifetime_});
    };

    createMarkMesh(leftPos);
    createMarkMesh(rightPos);
}
