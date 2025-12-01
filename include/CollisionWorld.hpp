#pragma once

#include <threepp/math/Vector3.hpp>
#include <threepp/math/Vector2.hpp>
#include <vector>

namespace threepp {
    class Object3D;
    class Scene;
    struct Vector2;
    struct Vector3;
}

// Minimal collision world: static ground plane and rock mesh colliders
class CollisionWorld {
public:
    struct MeshXZCollider {
        // Convex hull of the rock footprint projected to XZ (CCW order)
        std::vector<threepp::Vector2> hull;
    };

    struct NoCollisionZone {
        // Oriented rectangle on XZ plane where collisions are ignored
        threepp::Vector3 center; // use x,z; y ignored
        float halfWidth{0};      // along local right axis
        float halfDepth{0};      // along local forward axis
        float yaw{0};            // rotation around Y (radians)
    };

    // Ground plane Y (after ground is rotated to XZ plane)
    static float groundY();

    // Registers a mesh-based collider by computing the convex hull of all mesh vertices projected to XZ
    static void addRockMeshColliderFromObject(threepp::Object3D& obj);

    // Recomputes the convex hull from the given object and updates the last mesh collider.
    // Useful when a pile changes size (e.g., digging reduces the pile).
    static void updateLastRockMeshColliderFromObject(threepp::Object3D& obj);

    // Remove the most recently added mesh collider (e.g., when a dynamic pile is depleted)
    static void popLastRockMeshCollider();

    // Clears all rock colliders (useful when regenerating the environment)
    static void clear();

    // Adjust a proposed excavator (x,z) position to avoid penetrating any rock sphere
    // Returns true if any adjustment was made
    static bool resolveExcavatorMove(float& x, float& z, float excavatorRadius);

    // Check each excavator mesh part against rocks and resolve root position
    // Pass pointers to mesh objects (base, body, boom, stick, bucket)
    // Returns true if any adjustment was made
    static bool resolveExcavatorMeshCollisions(threepp::Object3D* root,
                                                 threepp::Object3D* baseMesh,
                                                 threepp::Object3D* bodyMesh,
                                                 threepp::Object3D* boomMesh,
                                                 threepp::Object3D* stickMesh,
                                                 threepp::Object3D* bucketMesh);

    // Add a pass-through zone (doorway) where mesh/AABB collisions are ignored
    static void addNoCollisionZone(const NoCollisionZone& zone);
    static void clearNoCollisionZones();

    // Debug visualization
    static void debugDrawRockHulls(threepp::Scene& scene, std::vector<std::shared_ptr<threepp::Object3D>>& debugObjects);
    static void debugDrawExcavatorHulls(threepp::Scene& scene, 
                                         threepp::Object3D* baseMesh,
                                         threepp::Object3D* bodyMesh,
                                         threepp::Object3D* boomMesh,
                                         threepp::Object3D* stickMesh,
                                         threepp::Object3D* bucketMesh,
                                         std::vector<std::shared_ptr<threepp::Object3D>>& debugObjects);

private:
    static std::vector<MeshXZCollider> s_rockMeshes;
    static float s_rockHullPadding;

    static std::vector<NoCollisionZone> s_noCollisionZones;
};
