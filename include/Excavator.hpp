#pragma once

#include <memory>
#include <array>
#include <string>
#include <filesystem>
#include <threepp/math/Vector3.hpp>
#include <threepp/objects/Group.hpp>
#include "Settings.hpp"

class ParticleSystem;

namespace threepp {
class Canvas;
class Scene;
class Object3D;
}

/**
 * Excavator: articulated construction vehicle with:
 * - Left/Right tracks (3 mesh variants per side for animation)
 * - Base (hull/chassis)
 * - Turret (upper body, rotates 360°)
 * - Boom (arm1), Stick (arm2), Bucket
 * 
 * All parts loaded from separate OBJ files and arranged in a parent-child hierarchy
 * so joint rotations compose properly.
 */
class Excavator {
public:
    struct Paths {
        // Track meshes (3 variants for animation)
        std::filesystem::path leftTrack0;
        std::filesystem::path leftTrack1;
        std::filesystem::path leftTrack2;
        std::filesystem::path rightTrack0;
        std::filesystem::path rightTrack1;
        std::filesystem::path rightTrack2;
        
        // Main parts
        std::filesystem::path base;      // hull/chassis
        std::filesystem::path body;      // turret/upper structure
        std::filesystem::path arm1;      // boom
        std::filesystem::path arm2;      // stick
        std::filesystem::path bucket;
    };

    /**
     * Loads all OBJ models and builds the scene graph hierarchy.
     * @param paths Paths to all OBJ files
     * @param scene Scene to add the excavator root to
     */
    Excavator(const Paths& paths, threepp::Scene& scene);
    ~Excavator();

    /**
     * Update per frame: advance track animation based on speeds, etc.
     * @param dt Delta time in seconds
     */
    void update(float dt);

    // Attach particle system for dust effects
    void setParticleSystem(ParticleSystem* ps) { particleSystem_ = ps; }

    // Get world positions of left/right track centers for particle spawning
    threepp::Vector3 getLeftTrackWorldPosition() const;
    threepp::Vector3 getRightTrackWorldPosition() const;

    // --- Track control ---
    // Speed in meters/sec. Positive = forward, negative = reverse
    void setLeftTrackSpeed(float mps);
    void setRightTrackSpeed(float mps);
    void setTracksSpeed(float left_mps, float right_mps);
    // Target (desired) speeds: will be reached gradually using acceleration
    void setTargetLeftTrackSpeed(float mps);
    void setTargetRightTrackSpeed(float mps);

    // --- Joint control (angles in radians) ---
    void setTurretYaw(float radians);           // Rotate turret around vertical axis
    void setBoomAngle(float radians);           // Boom pivot (up/down)
    void setStickAngle(float radians);          // Stick pivot relative to boom
    void setBucketAngle(float radians);         // Bucket pivot relative to stick

    // --- Joint limit config (radians) ---
    void setBoomLimits(float minRadians, float maxRadians);
    void setStickLimits(float minRadians, float maxRadians);
    void setBucketLimits(float minRadians, float maxRadians);

    // Debug helpers for aligning pivots at runtime
    void flipStickHingeEnd();   // Toggle which end (min/max) aligns to pivot
    void flipBucketHingeEnd();  // Toggle which end (min/max) aligns to pivot
    void nudgeStickAlongZ(float dz);   // Small offset adjustments after align (mesh)
    void nudgeBucketAlongZ(float dz);
    void nudgeStickPivotZ(float dz);   // Move the stick pivot itself along Z
    void nudgeBucketPivotZ(float dz);  // Move the bucket pivot itself along Z

    // --- Getters ---
    float getTurretYaw() const { return Settings::turretYaw_; }
    float getBoomAngle() const { return Settings::boomAngle_; }
    float getStickAngle() const { return Settings::stickAngle_; }
    float getBucketAngle() const { return Settings::bucketAngle_; }

    // Access root node (for positioning the whole excavator in world)
    threepp::Object3D* root();
    
    // Access mesh parts for collision visualization
    threepp::Object3D* baseMesh() { return baseMesh_.get(); }
    threepp::Object3D* bodyMesh() { return bodyMesh_.get(); }
    threepp::Object3D* boomMesh() { return arm1Mesh_.get(); }
    threepp::Object3D* stickMesh() { return arm2Mesh_.get(); }
    threepp::Object3D* bucketMesh() { return bucketMesh_.get(); }
    
    // Bucket load state for dig/dump gameplay
    bool isBucketLoaded() const { return Settings::bucketLoaded_; }
    void loadBucket();
    void unloadBucket();
    
    // Get bucket world position for zone detection
    threepp::Vector3 getBucketWorldPosition() const;
    
    // Reset to initial state
    void reset();

private:
    void loadModels_(const Paths& paths);
    void buildHierarchy_();
    void updateTrackFrame_(bool isLeft);

    threepp::Scene& scene_;

    // Root of the excavator hierarchy
    std::shared_ptr<threepp::Object3D> root_;

    // Loaded meshes (Groups from OBJLoader)
    std::array<std::shared_ptr<threepp::Group>, 3> leftTrackMeshes_;
    std::array<std::shared_ptr<threepp::Group>, 3> rightTrackMeshes_;
    std::shared_ptr<threepp::Group> baseMesh_;
    std::shared_ptr<threepp::Group> bodyMesh_;
    std::shared_ptr<threepp::Group> arm1Mesh_;
    std::shared_ptr<threepp::Group> arm2Mesh_;
    std::shared_ptr<threepp::Group> bucketMesh_;

    // Pivot nodes for articulation
    std::shared_ptr<threepp::Object3D> leftTrackPivot_;
    std::shared_ptr<threepp::Object3D> rightTrackPivot_;
    std::shared_ptr<threepp::Object3D> turretPivot_;
    std::shared_ptr<threepp::Object3D> boomPivot_;
    std::shared_ptr<threepp::Object3D> stickPivot_;
    std::shared_ptr<threepp::Object3D> bucketPivot_;

    // Debug flags and nudges for pivots
    bool stickUseMaxEnd_ = false;
    bool bucketUseMaxEnd_ = false;
    float stickNudgeZ_ = 0.0f;
    float bucketNudgeZ_ = 0.0f;

    // Particle system for dust effects
    ParticleSystem* particleSystem_{nullptr};
};
