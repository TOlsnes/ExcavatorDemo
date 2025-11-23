#include "Gate.hpp"

using namespace threepp;

Gate::Gate(const Vector3& position, int requiredDumps) 
    : m_position(position), m_requiredDumps(requiredDumps) {
    createVisual();
}

void Gate::update(int currentDumpCount) {
    // Check if requirements are met
    if (!m_isOpen && currentDumpCount >= m_requiredDumps) {
        m_isOpen = true;
    }
    
    // Shrink the blocking pile as dumps increase
    if (m_gateMesh && currentDumpCount > 0) {
        float progress = std::min(1.0f, static_cast<float>(currentDumpCount) / m_requiredDumps);
        
        // Scale down the pile
        float scale = 1.0f - progress;
        m_gateMesh->scale.set(1.0f, scale, 1.0f);
        
        // Lower the pile as it shrinks
        m_gateMesh->position.y = m_height / 2.0f * scale;
    }
}

void Gate::createVisual() {
    m_visual = Group::create();
    
    // Create a blocking pile of sand/debris (brown/tan color)
    auto pileMat = MeshPhongMaterial::create();
    pileMat->color = Color(0.7f, 0.5f, 0.3f); // Sandy brown
    
    // Pile blocking the path (wider to fill doorway)
    m_gateMesh = Mesh::create(
        BoxGeometry::create(m_width, m_height, m_depth * 3.0f), // Thicker depth
        pileMat
    );
    m_gateMesh->position.set(0, m_height / 2.0f, 0);
    m_visual->add(m_gateMesh);
    
    m_visual->position.copy(m_position);
}
