#include "DigZone.hpp"
#include <cmath>

using namespace threepp;

DigZone::DigZone(const Vector3& position, float radius) 
    : m_position(position), m_radius(radius) {
    createVisual();
}

bool DigZone::isInZone(const Vector3& point) const {
    // Use the actual pile shape: cylinder (h=1.5) + dome (hemisphere with r=0.8*radius)
    const float r = m_radius * m_scale;            // effective base radius
    const float hCyl = 1.5f * m_scale;             // cylinder height
    const float rDome = (m_radius * 0.8f) * m_scale; // dome radius
    const Vector3 center = m_position;

    const float dx = point.x - center.x;
    const float dz = point.z - center.z;
    const float distXZ = std::sqrt(dx * dx + dz * dz);

    // Inside cylinder volume (from ground up to hCyl)
    const bool inCylinder = (point.y >= center.y) && (point.y <= center.y + hCyl) && (distXZ <= r + 0.05f);

    // Inside hemisphere dome: (x^2 + z^2 + (y - yCenter)^2 <= rDome^2) for y >= center.y + hCyl
    const float yRel = point.y - (center.y + hCyl);
    const bool aboveCyl = point.y >= center.y + hCyl;
    const bool inDome = aboveCyl && (distXZ * distXZ + yRel * yRel <= (rDome + 0.02f) * (rDome + 0.02f));

    return inCylinder || inDome;
}

void DigZone::createVisual() {
    m_visual = Group::create();
    
    // Create a mound/pile of material (brown cylinder with rounded top)
    auto geometry = CylinderGeometry::create(m_radius, m_radius, 1.5f, 16);
    auto material = MeshPhongMaterial::create();
    material->color = Color(0.6f, 0.4f, 0.2f); // Brown sand/dirt color
    
    auto cylinder = Mesh::create(geometry, material);
    cylinder->position.y = 0.75f; // Half height above ground
    m_visual->add(cylinder);
    
    // Add a dome on top to make it look like a pile
    auto domeGeometry = SphereGeometry::create(m_radius * 0.8f, 16, 8, 0, threepp::math::PI * 2, 0, threepp::math::PI / 2);
    auto dome = Mesh::create(domeGeometry, material);
    dome->position.y = 1.5f;
    m_visual->add(dome);
    
    m_visual->position.copy(m_position);
}

bool DigZone::dig(float fraction) {
    if (fraction <= 0.0f) return false;
    // Reduce uniformly, clamp to a minimum scale to avoid disappearing instantly
    const float old = m_scale;
    m_scale = std::max(0.05f, m_scale * (1.0f - fraction));
    if (std::abs(m_scale - old) < 1e-4f) return false;
    if (m_visual) {
        m_visual->scale.set(m_scale, m_scale, m_scale);
        m_visual->updateMatrixWorld(true);
    }
    return true;
}

void DigZone::reset() {
    m_scale = 1.0f;
    if (m_visual) {
        m_visual->scale.set(1.0f, 1.0f, 1.0f);
        m_visual->updateMatrixWorld(true);
    }
}
