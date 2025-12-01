#include "CollisionWorld.hpp"
#include <threepp/threepp.hpp>
#include <algorithm>
#include <limits>
#include <iostream>

using namespace threepp;

// use convex hulls so you dont have to do full mesh to mesh collision
std::vector<CollisionWorld::MeshXZCollider> CollisionWorld::s_rockMeshes;
float CollisionWorld::s_rockHullPadding = 0.005f; // shrink hull by 5mm cus its colliding w air
std::vector<CollisionWorld::NoCollisionZone> CollisionWorld::s_noCollisionZones;

float CollisionWorld::groundY() {
    return 0.0f;
}

void CollisionWorld::clear() {
    s_rockMeshes.clear();
    s_noCollisionZones.clear();
}

namespace {
// 2D cross product of OA x OB
inline float cross2(const threepp::Vector2& a, const threepp::Vector2& b, const threepp::Vector2& c) {
    float abx = b.x - a.x;
    float aby = b.y - a.y;
    float acx = c.x - a.x;
    float acy = c.y - a.y;
    return abx * acy - aby * acx;
}

std::vector<threepp::Vector2> convexHull(std::vector<threepp::Vector2> pts) {
    if (pts.size() < 3) return pts;
    
    // Remove duplicates
    std::sort(pts.begin(), pts.end(), [](const threepp::Vector2& p1, const threepp::Vector2& p2){
        if (std::abs(p1.x - p2.x) < 1e-6f) return p1.y < p2.y;
        return p1.x < p2.x;
    });
    pts.erase(std::unique(pts.begin(), pts.end(), [](const threepp::Vector2& a, const threepp::Vector2& b) {
        return std::abs(a.x - b.x) < 1e-6f && std::abs(a.y - b.y) < 1e-6f;
    }), pts.end());
    
    if (pts.size() < 3) return pts;
    
    std::vector<threepp::Vector2> lower;
    for (const auto& p : pts) {
        while (lower.size() >= 2 && cross2(lower[lower.size()-2], lower[lower.size()-1], p) <= 0) lower.pop_back();
        lower.push_back(p);
    }
    std::vector<threepp::Vector2> upper;
    for (int i = static_cast<int>(pts.size()) - 1; i >= 0; --i) {
        const auto& p = pts[i];
        while (upper.size() >= 2 && cross2(upper[upper.size()-2], upper[upper.size()-1], p) <= 0) upper.pop_back();
        upper.push_back(p);
    }
    if (lower.size() > 0) lower.pop_back();
    if (upper.size() > 0) upper.pop_back();
    lower.insert(lower.end(), upper.begin(), upper.end());
    return lower.empty() ? pts : lower; // CCW order
}

// Compute convex hull for a single object
std::vector<threepp::Vector2> computeObjectHull(threepp::Object3D* obj, float minY = -0.1f) {
    if (!obj) {
        std::cout << "computeObjectHull: null object" << std::endl;
        return {};
    }
    obj->updateMatrixWorld(true);
    std::vector<threepp::Vector2> pts;
    pts.reserve(256);
    
    obj->traverseType<threepp::Mesh>([&](threepp::Mesh& m){
        m.updateMatrixWorld(true);
        auto geom = m.geometry();
        if (!geom) return;
        const auto* pos = geom->getAttribute<float>("position");
        if (!pos) return;
        threepp::Vector3 v;
        for (int i = 0, c = pos->count(); i < c; ++i) {
            v.x = pos->getX(i);
            v.y = pos->getY(i);
            v.z = pos->getZ(i);
            v.applyMatrix4(*m.matrixWorld);
            // Only include vertices above ground level for collision hull
            if (v.y >= minY) {
                pts.emplace_back(v.x, v.z);
            }
        }
    });
    
    if (pts.size() < 3) {
        return {};
    }
    auto hull = convexHull(std::move(pts));
    if (hull.size() < 3) {
        return {};
    }
    return hull;
}
}

static inline bool pointInOrientedRect(float px, float pz, const CollisionWorld::NoCollisionZone& z, float expandW = 0.f, float expandD = 0.f) {
    float dx = px - z.center.x;
    float dz = pz - z.center.z;
    float c = std::cos(z.yaw);
    float s = std::sin(z.yaw);
    // rotate by -yaw into zone local space
    float lx =  c * dx + s * dz;
    float lz = -s * dx + c * dz;
    float hw = z.halfWidth + expandW;
    float hd = z.halfDepth + expandD;
    return (std::abs(lx) <= hw && std::abs(lz) <= hd);
}

void CollisionWorld::addRockMeshColliderFromObject(Object3D& obj) {
    std::cout << "addRockMeshColliderFromObject: entering" << std::endl;
    // Collect all mesh vertices transformed to world, projected xz
    obj.updateMatrixWorld(true);
    std::vector<threepp::Vector2> pts;
    pts.reserve(512);

    obj.traverseType<threepp::Mesh>([&](threepp::Mesh& m){
        m.updateMatrixWorld(true);
        auto geom = m.geometry();
        if (!geom) {
            std::cout << "  mesh has no geometry" << std::endl;
            return;
        }
        const auto* pos = geom->getAttribute<float>("position");
        if (!pos) {
            std::cout << "  mesh has no position attribute" << std::endl;
            return;
        }
        std::cout << "  processing mesh with " << pos->count() << " vertices" << std::endl;
        threepp::Vector3 v;
        for (int i = 0, c = pos->count(); i < c; ++i) {
            v.x = pos->getX(i);
            v.y = pos->getY(i);
            v.z = pos->getZ(i);
            v.applyMatrix4(*m.matrixWorld);
            pts.emplace_back(v.x, v.z);
        }
    });

    std::cout << "addRockMeshColliderFromObject: collected " << pts.size() << " points total" << std::endl;
    if (pts.size() < 3) {
        std::cout << "addRockMeshColliderFromObject: too few points, returning" << std::endl;
        return;
    }

    std::cout << "addRockMeshColliderFromObject: calling convexHull" << std::endl;
    // Compute convex hull of the projected points
    auto hull = convexHull(std::move(pts));
    std::cout << "addRockMeshColliderFromObject: convexHull returned " << hull.size() << " points" << std::endl;
    if (hull.size() < 3) {
        std::cout << "addRockMeshColliderFromObject: hull too small, returning" << std::endl;
        return;
    }

    MeshXZCollider mc;
    mc.hull = std::move(hull);
    s_rockMeshes.push_back(std::move(mc));
    std::cout << "addRockMeshColliderFromObject: added collider" << std::endl;
}

void CollisionWorld::updateLastRockMeshColliderFromObject(Object3D& obj) {
    if (s_rockMeshes.empty()) return;
    // Recompute hull just like addRockMeshColliderFromObject, but replace the last entry
    obj.updateMatrixWorld(true);
    std::vector<threepp::Vector2> pts;
    pts.reserve(512);

    obj.traverseType<threepp::Mesh>([&](threepp::Mesh& m){
        m.updateMatrixWorld(true);
        auto geom = m.geometry();
        if (!geom) return;
        const auto* pos = geom->getAttribute<float>("position");
        if (!pos) return;
        threepp::Vector3 v;
        for (int i = 0, c = pos->count(); i < c; ++i) {
            v.x = pos->getX(i);
            v.y = pos->getY(i);
            v.z = pos->getZ(i);
            v.applyMatrix4(*m.matrixWorld);
            pts.emplace_back(v.x, v.z);
        }
    });

    if (pts.size() < 3) return;
    auto hull = convexHull(std::move(pts));
    if (hull.size() < 3) return;

    s_rockMeshes.back().hull = std::move(hull);
}

void CollisionWorld::popLastRockMeshCollider() {
    if (!s_rockMeshes.empty()) {
        s_rockMeshes.pop_back();
    }
}

void CollisionWorld::addNoCollisionZone(const NoCollisionZone& zone) {
    s_noCollisionZones.push_back(zone);
}

void CollisionWorld::clearNoCollisionZones() {
    s_noCollisionZones.clear();
}

bool CollisionWorld::resolveExcavatorMove(float& x, float& z, float excavatorRadius) {
    bool adjusted = false;
    // If inside any pass-through zone, skip mesh/AABB pushes
    for (const auto& zc : s_noCollisionZones) {
        if (pointInOrientedRect(x, z, zc, excavatorRadius, excavatorRadius)) {
            // us no collison zones so you can pass through dump piles and other stuff
            // Still resolve spheres (piles) to avoid falling through those but still being able to dig
            goto spheres_only;
        }
    }
    // First, resolve against mesh hulls (closest to real rock shapes)
    // This loop finds the closest point on the hull edges and pushes out if inside (2)
    for (const auto& mc : s_rockMeshes) {
        const auto& poly = mc.hull;
        if (poly.size() < 3) continue;
        // Find maximum signed distance to polygon edges using outward normals (CCW hull)
        float maxD = -std::numeric_limits<float>::infinity();
        threepp::Vector2 bestN{0,0};
        threepp::Vector2 p{x, z};
        for (size_t i = 0, n = poly.size(); i < n; ++i) {
            const auto& a = poly[i];
            const auto& b = poly[(i + 1) % n];
            threepp::Vector2 e{b.x - a.x, b.y - a.y};
            // Outward normal for CCW polygon is (e.y, -e.x)
            threepp::Vector2 nrm{e.y, -e.x};
            float len = std::sqrt(nrm.x * nrm.x + nrm.y * nrm.y);
            if (len <= 1e-6f) continue;
            nrm.x /= len; nrm.y /= len;
            float d = nrm.x * (p.x - a.x) + nrm.y * (p.y - a.y);
            if (d > maxD) { maxD = d; bestN = nrm; }
        }
        float effectiveR = std::max(0.f, excavatorRadius - s_rockHullPadding);
        if (maxD <= effectiveR) {
            float push = (effectiveR - maxD) + 1e-3f;
            x += bestN.x * push;
            z += bestN.y * push;
            adjusted = true;
        }
    }

    spheres_only:
    // Sphere collision handling removed - only mesh colliders used
    return adjusted;
}

bool CollisionWorld::resolveExcavatorMeshCollisions(threepp::Object3D* root,
                                                      threepp::Object3D* baseMesh,
                                                      threepp::Object3D* bodyMesh,
                                                      threepp::Object3D* boomMesh,
                                                      threepp::Object3D* stickMesh,
                                                      threepp::Object3D* bucketMesh) {
    if (!root) return false;
    
    // Check base/tracks, body, and boom for collision, bucket not included cus of digging
    std::vector<threepp::Object3D*> parts;
    if (baseMesh) parts.push_back(baseMesh);
    if (bodyMesh) parts.push_back(bodyMesh);
    if (boomMesh) parts.push_back(boomMesh);
    
    bool adjusted = false;
    threepp::Vector3 totalPush{0, 0, 0};
    
    // For each excavator part, compute its convex hull and check against rock hulls
    for (auto* part : parts) {
        auto partHull = computeObjectHull(part);
        if (partHull.size() < 3) continue;

        // If any vertex of this part is in a pass-through zone, skip mesh/AABB collision for this part (helped with air collision around the enterance)
        bool inNoCollide = false;
        if (!s_noCollisionZones.empty()) {
            for (const auto& zc : s_noCollisionZones) {
                for (const auto& p : partHull) {
                    if (pointInOrientedRect(p.x, p.y, zc)) { inNoCollide = true; break; }
                }
                if (inNoCollide) break;
            }
        }

        if (inNoCollide) {
            continue; // skip mesh pushes for this part; still allow spheres later via root push accumulation (mesh collision skipped)
        }
        
        // Check this excavator parts hull against each rock hull
        for (const auto& mc : s_rockMeshes) {
            const auto& rockHull = mc.hull;
            if (rockHull.size() < 3) continue;
            
            // Find the deepest edge on the rock hull
            float maxPenetration = -std::numeric_limits<float>::infinity();
            threepp::Vector2 bestN{0, 0};
            
            // Check each rock edge
            for (size_t i = 0, n = rockHull.size(); i < n; ++i) {
                const auto& a = rockHull[i];
                const auto& b = rockHull[(i + 1) % n];
                threepp::Vector2 e{b.x - a.x, b.y - a.y};
                threepp::Vector2 nrm{e.y, -e.x};
                float len = std::sqrt(nrm.x * nrm.x + nrm.y * nrm.y);
                if (len <= 1e-6f) continue;
                nrm.x /= len; nrm.y /= len;
                
                // Find minimum signed distance of excavator hull vertices to this rock edge
                float minDist = std::numeric_limits<float>::infinity();
                for (const auto& p : partHull) {
                    float d = nrm.x * (p.x - a.x) + nrm.y * (p.y - a.y);
                    minDist = std::min(minDist, d);
                }
                
                // Track the most penetrating edge (smallest distance, possibly negative)
                if (minDist > maxPenetration) {
                    maxPenetration = minDist;
                    bestN = nrm;
                }
            }
            
            // If penetrating push out
            float threshold = -s_rockHullPadding;
            if (maxPenetration < threshold) {
                float push = (threshold - maxPenetration) + 1e-3f;
                totalPush.x += bestN.x * push;
                totalPush.z += bestN.y * push;
                adjusted = true;
            }
        }
    }
    
    if (adjusted) {
        root->position.x += totalPush.x;
        root->position.z += totalPush.z;
    }
    
    return adjusted;
}

void CollisionWorld::debugDrawRockHulls(threepp::Scene& scene, std::vector<std::shared_ptr<threepp::Object3D>>& debugObjects) {
    // Clear previous debug objects from scene cus they were lingering
    for (auto& obj : debugObjects) {
        if (obj) scene.remove(*obj);
    }
    debugObjects.clear();
    
    // Draw rock hulls
    for (const auto& mc : s_rockMeshes) {
        const auto& hull = mc.hull;
        if (hull.size() < 3) continue;
        
        std::vector<float> vertices;
        vertices.reserve(hull.size() * 6);
        
        for (size_t i = 0; i < hull.size(); ++i) {
            const auto& p = hull[i];
            const auto& next = hull[(i + 1) % hull.size()];
            
            vertices.push_back(p.x);
            vertices.push_back(0.1f); // move up so i can see the debug lines
            vertices.push_back(p.y);
            
            vertices.push_back(next.x);
            vertices.push_back(0.1f);
            vertices.push_back(next.y);
        }
        
        if (vertices.empty()) continue;
        
        try {
            auto geom = threepp::BufferGeometry::create();
            geom->setAttribute("position", FloatBufferAttribute::create(std::vector<float>(vertices), 3)); // I get a no instance of overloaded function error but it still works so imma not question it
            auto mat = threepp::LineBasicMaterial::create();
            mat->color = threepp::Color::red;
            auto line = threepp::LineSegments::create(geom, mat);
            scene.add(line);
            debugObjects.push_back(line);
        } catch (...) {
            // Ignore errors creating debug geometry
        }
    }
}

void CollisionWorld::debugDrawExcavatorHulls(threepp::Scene& scene,
                                              threepp::Object3D* baseMesh,
                                              threepp::Object3D* bodyMesh,
                                              threepp::Object3D* boomMesh,
                                              threepp::Object3D* stickMesh,
                                              threepp::Object3D* bucketMesh,
                                              std::vector<std::shared_ptr<threepp::Object3D>>& debugObjects) {
    std::vector<threepp::Object3D*> parts;
    if (baseMesh) parts.push_back(baseMesh);
    if (bodyMesh) parts.push_back(bodyMesh);
    if (boomMesh) parts.push_back(boomMesh);
    if (stickMesh) parts.push_back(stickMesh);
    if (bucketMesh) parts.push_back(bucketMesh);
    
    for (auto* part : parts) {
        auto hull = computeObjectHull(part);
        if (hull.size() < 3) continue;
        
        std::vector<float> vertices;
        for (size_t i = 0; i < hull.size(); ++i) {
            const auto& p = hull[i];
            vertices.push_back(p.x);
            vertices.push_back(0.2f); // slightly higher than rock hulls
            vertices.push_back(p.y);
            
            const auto& next = hull[(i + 1) % hull.size()];
            vertices.push_back(next.x);
            vertices.push_back(0.2f);
            vertices.push_back(next.y);
        }
        
        auto geom = threepp::BufferGeometry::create();
        geom->setAttribute("position", FloatBufferAttribute::create(vertices, 3)); // same overload error but imma not question it
        auto mat = threepp::LineBasicMaterial::create();
        mat->color = threepp::Color::green;
        auto line = threepp::LineSegments::create(geom, mat);
        scene.add(line);
        debugObjects.push_back(line);
    }
}
