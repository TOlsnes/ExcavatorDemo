#include "DumpZone.hpp"
#include <cmath>

using namespace threepp;

DumpZone::DumpZone(const Vector3& position, float radius) 
    : m_position(position), m_radius(radius) {
    createVisual();
}

bool DumpZone::isInZone(const Vector3& point) const {
    // Check if point is within cylinder (XZ plane + height check)
    float dx = point.x - m_position.x;
    float dz = point.z - m_position.z;
    float distXZ = std::sqrt(dx * dx + dz * dz);
    
    // Allow vertical tolerance
    bool withinHeight = (point.y >= m_position.y - 1.0f) && 
                        (point.y <= m_position.y + 5.0f);
    
    return (distXZ <= m_radius) && withinHeight;
}

void DumpZone::recordDump() {
    m_dumpCount++;
    updatePileHeight();
}

void DumpZone::createVisual() {
    m_visual = Group::create();
    
    // Create a platform/marker for the dump zone soyou can see it before dumpint the first time
    auto platformGeom = CylinderGeometry::create(m_radius, m_radius, 0.2f, 16);
    auto platformMat = MeshPhongMaterial::create();
    platformMat->color = Color(0.7f, 0.7f, 0.7f); // Light gray platform
    
    auto platform = Mesh::create(platformGeom, platformMat);
    platform->position.y = 0.1f;
    m_visual->add(platform);
    
    // Create the growing pile (starts empty)
    auto pileGeom = CylinderGeometry::create(m_radius * 0.7f, m_radius * 0.7f, 0.01f, 16);
    auto pileMat = MeshPhongMaterial::create();
    pileMat->color = Color(0.6f, 0.4f, 0.2f); // Brown material
    
    m_pileMesh = Mesh::create(pileGeom, pileMat);
    m_pileMesh->position.y = 0.21f;
    m_visual->add(m_pileMesh);
    
    m_visual->position.copy(m_position);
}

void DumpZone::updatePileHeight() {
    if (!m_pileMesh) return;
    
    // Grow pile height with each dump ADJUST HERE CUS IT LOOKS DUMB RN
    float height = 0.01f + m_dumpCount * 0.3f;
    
    // Recreate geometry with new height
    auto newGeom = CylinderGeometry::create(m_radius * 0.7f, m_radius * 0.7f, height, 16);
    m_pileMesh->setGeometry(newGeom);
    m_pileMesh->position.y = 0.2f + height / 2.0f;
}

void DumpZone::reset() {
    m_dumpCount = 0;
    if (m_pileMesh) {
        auto newGeom = CylinderGeometry::create(m_radius * 0.7f, m_radius * 0.7f, 0.01f, 16);
        m_pileMesh->setGeometry(newGeom);
        m_pileMesh->position.y = 0.21f;
    }
}
