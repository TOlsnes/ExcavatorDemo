#include "Excavator.hpp"
#include "ParticleSystem.hpp"
#include "CollisionWorld.hpp"
#include "Settings.hpp"

#include <threepp/threepp.hpp>
#include <threepp/loaders/OBJLoader.hpp>
#include <threepp/math/Box3.hpp>
#include <cmath>
#include <iostream>
#include <algorithm>

using namespace Settings;
using namespace threepp;

namespace {

// Align along a axis (0=X,1=Y,2=Z): place either min or max end at the parent's pivot.
void alignEndAtPivotAxis(Object3D* obj, int axis, bool useMaxEnd = false) {
    if (!obj) return;

    Box3 box;
    box.setFromObject(*obj, true);

    const auto& bmin = box.min();
    const auto& bmax = box.max();

    float source = 0.0f;
    switch (axis) {
        case 0: source = useMaxEnd ? bmax.x : bmin.x; break;
        case 1: source = useMaxEnd ? bmax.y : bmin.y; break;
        default: source = useMaxEnd ? bmax.z : bmin.z; break;
    }

    switch (axis) {
        case 0: obj->position.x += -source; break;
        case 1: obj->position.y += -source; break;
        default: obj->position.z += -source; break;
    }
}

}

Excavator::Excavator(const Paths& paths, Scene& scene)
    : scene_(scene) {

    loadModels_(paths);
    buildHierarchy_();

    scene_.add(root_);
}

Excavator::~Excavator() = default;

void Excavator::reset() {
    // Resets
    root_->position.set(0, 0, 0);
    baseYaw_ = 0.0f;
    root_->rotation.z = 0.0f;
    
    targetLeftTrackSpeed_ = 0.0f;
    targetRightTrackSpeed_ = 0.0f;
    leftTrackSpeed_ = 0.0f;
    rightTrackSpeed_ = 0.0f;
    leftTrackDist_ = 0.0f;
    rightTrackDist_ = 0.0f;
    leftTrackFrame_ = 0;
    rightTrackFrame_ = 0;
    
    for (int i = 0; i < 3; ++i) {
        if (leftTrackMeshes_[i]) {
            leftTrackMeshes_[i]->visible = (i == 0);
        }
        if (rightTrackMeshes_[i]) {
            rightTrackMeshes_[i]->visible = (i == 0);
        }
    }
    
    turretYaw_ = 0.0f;
    boomAngle_ = 0.0f;
    stickAngle_ = 0.0f;
    bucketAngle_ = 0.0f;

    if (turretPivot_) turretPivot_->rotation.z = 0.0f;
    if (boomPivot_) boomPivot_->rotation.y = 0.0f;
    if (stickPivot_) stickPivot_->rotation.y = 0.0f;
    if (bucketPivot_) bucketPivot_->rotation.y = 0.0f;

    bucketLoaded_ = false;
    
    // Reset bucket color to gray
    if (bucketMesh_) {
        bucketMesh_->traverseType<Mesh>([](Mesh& m) {
            if (auto mat = m.material()) {
                if (auto phong = dynamic_cast<threepp::MeshPhongMaterial*>(mat.get())) { //Copilot fixed this cus i forgot to use phong material as using the "as" thing didnt want to work with threepp
                    phong->color.setRGB(0.5f, 0.5f, 0.5f); // Gray
                }
            }
        });
    }
    
    // Update matrixes
    root_->updateMatrixWorld(true);
}

void Excavator::loadModels_(const Paths& paths) {
    OBJLoader loader;

    std::cout << "Loading excavator models...\n";

    // Load track animations
    leftTrackMeshes_[0] = loader.load(paths.leftTrack0);
    leftTrackMeshes_[1] = loader.load(paths.leftTrack1);
    leftTrackMeshes_[2] = loader.load(paths.leftTrack2);

    rightTrackMeshes_[0] = loader.load(paths.rightTrack0);
    rightTrackMeshes_[1] = loader.load(paths.rightTrack1);
    rightTrackMeshes_[2] = loader.load(paths.rightTrack2);

    // Load main parts
    baseMesh_ = loader.load(paths.base);
    bodyMesh_ = loader.load(paths.body);
    arm1Mesh_ = loader.load(paths.arm1);
    arm2Mesh_ = loader.load(paths.arm2);
    bucketMesh_ = loader.load(paths.bucket);

    std::cout << "All models loaded.\n";
}

void Excavator::buildHierarchy_() {
    // Create root node
    root_ = Object3D::create();
    root_->name = "excavator_root";
    
    // Scale down from Fusion 360 export (from mm to meters)
    root_->scale.set(0.01f, 0.01f, 0.01f);
    
    // Rotated cus its being dumb and upsidr down
    root_->rotation.x = -math::PI / 2;

    // --- Base (chassis) ---
    baseMesh_->name = "base";
    root_->add(baseMesh_);

    // --- Left Track Pivot ---
    // Create pivot before use (was null -> crash)
    leftTrackPivot_ = Object3D::create();
    leftTrackPivot_->name = "leftTrackPivot";
    // root has scale=0.01 and rotation=-90°X, so local coords are 100x larger than world
    leftTrackPivot_->position.set(-20.0f, -50.0f, 0.0f); 
    std::cout << "Left track pivot at X=" << leftTrackPivot_->position.x 
              << " Y=" << leftTrackPivot_->position.y 
              << " Z=" << leftTrackPivot_->position.z << "\n";
    root_->add(leftTrackPivot_);

    // Align all left track frames: add frame 0 first
    // then offset frames 1 and 2 to occupy the same world space
    leftTrackMeshes_[0]->name = "leftTrack_0";
    leftTrackMeshes_[0]->visible = true;
    leftTrackPivot_->add(leftTrackMeshes_[0]);
    
    // Update matrix world to get correct world positions
    root_->updateMatrixWorld(true);
    
    // Compute frame 0's world center
    Box3 leftBox0;
    leftBox0.setFromObject(*leftTrackMeshes_[0], false);  // use world coords
    Vector3 leftCenter0;
    leftBox0.getCenter(leftCenter0);
    std::cout << "Left track center: X=" << leftCenter0.x << " Y=" << leftCenter0.y << " Z=" << leftCenter0.z << "\n";
    
    for (size_t i = 1; i < leftTrackMeshes_.size(); ++i) {
        leftTrackMeshes_[i]->name = "leftTrack_" + std::to_string(i);
        leftTrackMeshes_[i]->visible = false;
        leftTrackPivot_->add(leftTrackMeshes_[i]);
        
        // Compute this frame's world bounding box center
        Box3 boxI;
        boxI.setFromObject(*leftTrackMeshes_[i], false);
        Vector3 centerI;
        boxI.getCenter(centerI);
        
        // Offset so world centers match
        Vector3 offset = leftCenter0 - centerI;
        leftTrackMeshes_[i]->position.add(offset);
    }

    // --- Right Track Pivot ---
    rightTrackPivot_ = Object3D::create();
    rightTrackPivot_->name = "rightTrackPivot";
    // Manual position: same as left track 
    rightTrackPivot_->position.set(-20.0f, 25.0f, 0.0f);  
    std::cout << "Right track pivot at X=" << rightTrackPivot_->position.x 
              << " Y=" << rightTrackPivot_->position.y 
              << " Z=" << rightTrackPivot_->position.z << "\n";
    root_->add(rightTrackPivot_);

    // Right track: add without mirroring (OBJ already has correct offset at X=+24.6)
    rightTrackMeshes_[0]->name = "rightTrack_0";
    rightTrackMeshes_[0]->visible = true;
    
    // Don't mirror - let it use its natural position from the OBJ
    // rightTrackMeshes_[0]->scale.x = -1.0f;
    
    rightTrackPivot_->add(rightTrackMeshes_[0]);
    
    // Update matrix world to get correct world positions
    root_->updateMatrixWorld(true);
    
    // Compute frame 0's world bounding box center for right track
    Box3 rightBox0;
    rightBox0.setFromObject(*rightTrackMeshes_[0], false);  // use world coords
    Vector3 rightCenter0;
    rightBox0.getCenter(rightCenter0);
    std::cout << "Right track center: X=" << rightCenter0.x << " Y=" << rightCenter0.y << " Z=" << rightCenter0.z << "\n";
    
    // Manual track alignment - adjust the pivot positions above to move the tracks
    // The frame alignment below ensures all 3 frames for each track stay together
    
    // Align frames 1 and 2 to frame 0 (same as left track alignment)
    for (size_t i = 1; i < rightTrackMeshes_.size(); ++i) {
        rightTrackMeshes_[i]->name = "rightTrack_" + std::to_string(i);
        rightTrackMeshes_[i]->visible = false;
        
        // Don't mirror - use natural position from OBJ
        // rightTrackMeshes_[i]->scale.x = -1.0f;
        
        rightTrackPivot_->add(rightTrackMeshes_[i]);
        
        // Compute this frame's world bounding box center
        Box3 boxI;
        boxI.setFromObject(*rightTrackMeshes_[i], false);
        Vector3 centerI;
        boxI.getCenter(centerI);
        
        // Offset so world centers match
        Vector3 offset = rightCenter0 - centerI;
        rightTrackMeshes_[i]->position.add(offset);
    }

    // --- Turret Pivot (on top of base) ---
    // Position at the center of the turret ring on the base
    turretPivot_ = Object3D::create();
    turretPivot_->name = "turretPivot";
    turretPivot_->position.set(0.0f, 0.5f, 0.0f); // Adjust Y to base height
    root_->add(turretPivot_);

    // Add body (turret upper structure) as child of turret pivot
    bodyMesh_->name = "body";
    turretPivot_->add(bodyMesh_);

    // --- Boom Pivot (on turret body) ---
    // Position at the boom's pin location on the turret
    boomPivot_ = Object3D::create();
    boomPivot_->name = "boomPivot";
    boomPivot_->position.set(0.0f, 0.3f, 0.5f); // Adjust to boom pin location
    turretPivot_->add(boomPivot_);

    // Add boom mesh relative to its pivot
    arm1Mesh_->name = "boom";
    // If your boom mesh's origin is at the pin, no offset needed
    // If not, adjust position so rotation swings correctly
    boomPivot_->add(arm1Mesh_);

    // --- Stick Pivot (at end of boom) ---
    stickPivot_ = Object3D::create();
    stickPivot_->name = "stickPivot";
    // After root rotation (-90 deg around X), the boom's length axis maps to +Y
    // Place the stick pivot along +Y roughly at the boom's end
    stickPivot_->position.set(-150.0f, 5.0f, 100.0f); // Adjust to boom length
    boomPivot_->add(stickPivot_);

    arm2Mesh_->name = "stick";
    // Arm2 re-exported with origin at hinge: don't auto-align; allow pivot nudging instead
    arm2Mesh_->position.z += stickNudgeZ_;
    stickPivot_->add(arm2Mesh_);

    // --- Bucket Pivot (at end of stick) ---
    bucketPivot_ = Object3D::create();
    bucketPivot_->name = "bucketPivot";
    // Start at zero; use runtime nudges to place along +Y (stick direction)
    bucketPivot_->position.set(10.0f, 0.0f, -87.0f); 
    stickPivot_->add(bucketPivot_);

    bucketMesh_->name = "bucket";
    // Align bucket along Z axis (minZ by default); can flip at runtime
    alignEndAtPivotAxis(bucketMesh_.get(), 2, bucketUseMaxEnd_);
    bucketMesh_->position.z += bucketNudgeZ_;
    bucketPivot_->add(bucketMesh_);

    std::cout << "Excavator hierarchy built.\n";

    // After hierarchy is built, compute a reasonable base collision radius from base & tracks footprint
    if (root_) {
        root_->updateMatrixWorld(true);
        threepp::Box3 bb; bb.makeEmpty();
        if (baseMesh_) bb.expandByObject(*baseMesh_, false);
        if (leftTrackMeshes_[0]) bb.expandByObject(*leftTrackMeshes_[0], false);
        if (rightTrackMeshes_[0]) bb.expandByObject(*rightTrackMeshes_[0], false);
        threepp::Vector3 sz; bb.getSize(sz);
        float r = 0.5f * std::min(sz.x, sz.z);
        // Slight shrink to avoid "air" touches and keep steering smooth
        baseRadius_ = std::max(0.1f, r * 0.9f);
        std::cout << "Computed baseRadius_= " << baseRadius_ << " from footprint w=" << sz.x << " z=" << sz.z << "\n";
    }
}

void Excavator::update(float dt) {
    // Ramp current speeds toward targets (acceleration/deceleration)
    auto ramp = [&](float current, float target) {
        float diff = target - current;
        if (std::abs(diff) < 1e-5f) return target;
        // Use symmetric ramp rate to avoid jerky braking on differential turns
        float rate = acceleration_;
        float step = rate * dt;
        if (diff > 0) {
            current += step;
            if (current > target) current = target;
        } else {
            current -= step;
            if (current < target) current = target;
        }
        return current;
    };

    leftTrackSpeed_ = ramp(leftTrackSpeed_, targetLeftTrackSpeed_);
    rightTrackSpeed_ = ramp(rightTrackSpeed_, targetRightTrackSpeed_);

    // Accumulate distance for animation using current (ramped) speeds
    leftTrackDist_ += leftTrackSpeed_ * dt;
    rightTrackDist_ += rightTrackSpeed_ * dt;

    // Update track frame selection (animation)
    updateTrackFrame_(true);
    updateTrackFrame_(false);

    // Compute movement & turning from differential track speeds
    float linearSpeed = (leftTrackSpeed_ + rightTrackSpeed_) * 0.5f; // m/s forward
    float angularVelocity = (rightTrackSpeed_ - leftTrackSpeed_) / trackWidth_; // rad/s yaw
    baseYaw_ += angularVelocity * dt;
    // Apply yaw to root (around vertical axis => rotation.z after -90° X rotation)
    root_->rotation.z = baseYaw_;

    // Forward direction: use standard polar coordinates
    float dx = -std::cos(baseYaw_) * linearSpeed * dt;
    // Flip Z component to align motion with visual yaw (post -90° X rotation)
    float dz = std::sin(baseYaw_) * linearSpeed * dt;
    root_->position.x += dx;
    root_->position.z += dz;
    
    // Update world matrices before collision check
    root_->updateMatrixWorld(true);
    
    // Resolve collisions for each mesh part
    CollisionWorld::resolveExcavatorMeshCollisions(root_.get(),
                                                     baseMesh_.get(),
                                                     bodyMesh_.get(),
                                                     arm1Mesh_.get(),
                                                     arm2Mesh_.get(),
                                                     bucketMesh_.get());

    // Spawn dust particles when moving above threshold
    if (particleSystem_ && std::abs(linearSpeed) > speedThresholdForParticles_) {
        particleSpawnTimer_ += dt;
        if (particleSpawnTimer_ >= particleSpawnInterval_) {
            particleSpawnTimer_ = 0.f;
            // Spawn at left and right track positions
            particleSystem_->spawnParticle(getLeftTrackWorldPosition());
            particleSystem_->spawnParticle(getRightTrackWorldPosition());
        }
    }
}

void Excavator::updateTrackFrame_(bool isLeft) {
    float& dist = isLeft ? leftTrackDist_ : rightTrackDist_;
    int& frame = isLeft ? leftTrackFrame_ : rightTrackFrame_;
    auto& meshes = isLeft ? leftTrackMeshes_ : rightTrackMeshes_;

    // Map distance to phase [0,1)
    float phase = std::fmod(dist / trackCircumference_, 1.0f);
    if (phase < 0.0f) phase += 1.0f;

    // Map phase to frame index [0,1,2]
    int newFrame = static_cast<int>(phase * 3.0f) % 3;

    if (newFrame != frame) {
        // Hide old, show new
        meshes[frame]->visible = false;
        meshes[newFrame]->visible = true;
        frame = newFrame;
    }
}

void Excavator::setLeftTrackSpeed(float mps) {
    targetLeftTrackSpeed_ = mps; // treat as desired speed (will ramp)
}

void Excavator::setRightTrackSpeed(float mps) {
    targetRightTrackSpeed_ = mps;
}

void Excavator::setTracksSpeed(float left_mps, float right_mps) {
    targetLeftTrackSpeed_ = left_mps;
    targetRightTrackSpeed_ = right_mps;
}

void Excavator::setTargetLeftTrackSpeed(float mps) { targetLeftTrackSpeed_ = mps; }
void Excavator::setTargetRightTrackSpeed(float mps) { targetRightTrackSpeed_ = mps; }

void Excavator::setTurretYaw(float radians) {
    turretYaw_ = radians;
    // After rotating root -90° around X, turret spins around Z (vertical)
    turretPivot_->rotation.z = radians;
}

void Excavator::setBoomAngle(float radians) {
    // Clamp to mechanical limits
    radians = std::clamp(radians, boomMin_, boomMax_);
    float old = boomAngle_;
    auto apply = [&](float val){
        boomAngle_ = val;
        boomPivot_->rotation.y = val; // After -90° X, boom pitches around Y
        if (root_) root_->updateMatrixWorld(true);
    };
    apply(radians);
    bool violates = false;
    if (bucketMesh_) {
        threepp::Box3 b; b.setFromObject(*bucketMesh_, false);
        if (b.min().y < CollisionWorld::groundY()) violates = true;
    }
    if (violates) {
        apply(old);
    }
}

void Excavator::setStickAngle(float radians) {
    // Clamp to mechanical limits
    radians = std::clamp(radians, stickMin_, stickMax_);
    float old = stickAngle_;
    auto apply = [&](float val){
        stickAngle_ = val;
        stickPivot_->rotation.x = 0;
        stickPivot_->rotation.y = val;
        stickPivot_->rotation.z = 0;
        if (root_) root_->updateMatrixWorld(true);
    };
    apply(radians);
    bool violates = false;
    if (bucketMesh_) {
        threepp::Box3 b; b.setFromObject(*bucketMesh_, false);
        if (b.min().y < CollisionWorld::groundY()) violates = true;
    }
    if (violates) {
        apply(old);
    }
}

void Excavator::setBucketAngle(float radians) {
    // Clamp to mechanical limits
    radians = std::clamp(radians, bucketMin_, bucketMax_);
    float old = bucketAngle_;
    auto apply = [&](float val){
        bucketAngle_ = val;
        bucketPivot_->rotation.x = 0;
        bucketPivot_->rotation.y = val;
        bucketPivot_->rotation.z = 0;
        if (root_) root_->updateMatrixWorld(true);
    };
    apply(radians);
    bool violates = false;
    if (bucketMesh_) {
        threepp::Box3 b; b.setFromObject(*bucketMesh_, false);
        if (b.min().y < CollisionWorld::groundY()) violates = true;
    }
    if (violates) {
        apply(old);
    }
}

void Excavator::setBoomLimits(float minRadians, float maxRadians) {
    if (minRadians > maxRadians) std::swap(minRadians, maxRadians);
    boomMin_ = minRadians;
    boomMax_ = maxRadians;
    // Re-apply clamp to current value
    setBoomAngle(boomAngle_);
}

void Excavator::setStickLimits(float minRadians, float maxRadians) {
    if (minRadians > maxRadians) std::swap(minRadians, maxRadians);
    stickMin_ = minRadians;
    stickMax_ = maxRadians;
    setStickAngle(stickAngle_);
}

void Excavator::setBucketLimits(float minRadians, float maxRadians) {
    if (minRadians > maxRadians) std::swap(minRadians, maxRadians);
    bucketMin_ = minRadians;
    bucketMax_ = maxRadians;
    setBucketAngle(bucketAngle_);
}

Object3D* Excavator::root() {
    return root_.get();
}

// --- Debug alignment helpers ---
void Excavator::flipStickHingeEnd() {
    stickUseMaxEnd_ = !stickUseMaxEnd_;
    // Re-align current mesh in-place relative to its parent pivot
    if (arm2Mesh_) {
        // Reset local position on Z before re-aligning
        arm2Mesh_->position.z = 0;
        alignEndAtPivotAxis(arm2Mesh_.get(), 2, stickUseMaxEnd_);
        arm2Mesh_->position.z += stickNudgeZ_;
    }
}

void Excavator::flipBucketHingeEnd() {
    bucketUseMaxEnd_ = !bucketUseMaxEnd_;
    if (bucketMesh_) {
        bucketMesh_->position.z = 0;
        alignEndAtPivotAxis(bucketMesh_.get(), 2, bucketUseMaxEnd_);
        bucketMesh_->position.z += bucketNudgeZ_;
    }
}

void Excavator::nudgeStickAlongZ(float dz) {
    stickNudgeZ_ += dz;
    if (arm2Mesh_) arm2Mesh_->position.z += dz;
}

void Excavator::nudgeBucketAlongZ(float dz) {
    bucketNudgeZ_ += dz;
    if (bucketMesh_) bucketMesh_->position.z += dz;
}

void Excavator::nudgeStickPivotZ(float dz) {
    if (stickPivot_) {
        // Convert desired world-space delta to local by compensating for root scale
        float invScale = (root_) ? (1.f / root_->scale.y) : 1.f;
        stickPivot_->position.y += dz * invScale;
        std::cout << "stickPivot Y: " << stickPivot_->position.y << "\n";
    }
}

void Excavator::nudgeBucketPivotZ(float dz) {
    if (bucketPivot_) {
        float invScale = (root_) ? (1.f / root_->scale.y) : 1.f;
        bucketPivot_->position.y += dz * invScale;
        std::cout << "bucketPivot Y: " << bucketPivot_->position.y << "\n";
    }
}

Vector3 Excavator::getLeftTrackWorldPosition() const {
    if (!leftTrackPivot_) {
        return root_ ? root_->position : Vector3{0, 0, 0};
    }
    Vector3 pos;
    leftTrackPivot_->getWorldPosition(pos);
    
    // Offset toward inside (positive Y in local space = toward center)
    // The left track pivot is at Y=-50 in local space, so positive Y moves toward center
    if (root_) {
        float yaw = baseYaw_; // Use the stored yaw instead of root_->rotation.z
        // Local offset in Y direction (toward center)
        float offsetAmount = -0.25f;
        // Transform to world space: Y axis in local becomes perpendicular to forward
        // Forward is (-cos(yaw), sin(yaw)), so perpendicular (left Y+) is (sin(yaw), cos(yaw))
        pos.x += std::sin(yaw) * offsetAmount;
        pos.z += std::cos(yaw) * offsetAmount;
    }
    
    return pos;
}

Vector3 Excavator::getRightTrackWorldPosition() const {
    if (!rightTrackPivot_) {
        return root_ ? root_->position : Vector3{0, 0, 0};
    }
    Vector3 pos;
    rightTrackPivot_->getWorldPosition(pos);
    return pos;
}

Vector3 Excavator::getBucketWorldPosition() const {
    if (!bucketMesh_) {
        return root_ ? root_->position : Vector3{0, 0, 0};
    }
    Vector3 pos;
    bucketMesh_->getWorldPosition(pos);
    return pos;
}

void Excavator::loadBucket() {
    bucketLoaded_ = true;
    
    // Change bucket color to show it's loaded (brown/sandy color)
    if (bucketMesh_) {
        bucketMesh_->traverse([](Object3D& obj) {
            if (auto* mesh = obj.as<Mesh>()) {
                if (auto* mat = mesh->material()->as<MaterialWithColor>()) { // Think this is a false error cus its still working so idek
                    mat->color = Color(0.7f, 0.5f, 0.3f); // Sandy brown
                }
            }
        });
    }
}

void Excavator::unloadBucket() {
    bucketLoaded_ = false;
    
    // Reset bucket color to original (gray/metal)
    if (bucketMesh_) {
        bucketMesh_->traverse([](Object3D& obj) {
            if (auto* mesh = obj.as<Mesh>()) {
                if (auto* mat = mesh->material()->as<MaterialWithColor>()) { // same here, think its a false error
                    mat->color = Color(0.7f, 0.7f, 0.7f); // Gray metal
                }
            }
        });
    }
}
