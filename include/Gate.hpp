#ifndef GATE_HPP
#define GATE_HPP

#include <threepp/threepp.hpp>
#include <memory>

/**
 * Gate/Door that blocks passage until a certain number of dumps are completed.
 * Opens (slides/rotates) when the requirement is met.
 */
class Gate {
public:
    Gate(const threepp::Vector3& position, int requiredDumps);
    
    // Check if gate should open based on dump count
    void update(int currentDumpCount);
    
    // Check if gate is currently open
    bool isOpen() const { return m_isOpen; }
    
    // Get the visual representation
    std::shared_ptr<threepp::Group> getVisual() const { return m_visual; }
    
    // Get the pile mesh for collision detection
    threepp::Object3D* getPileMesh() const { return m_gateMesh.get(); }
    
    // Get gate bounds for collision checking when closed
    const threepp::Vector3& getPosition() const { return m_position; }
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }
    
private:
    threepp::Vector3 m_position;
    int m_requiredDumps;
    bool m_isOpen{false};
    float m_width{4.0f};
    float m_height{3.0f};
    float m_depth{0.5f};
    
    std::shared_ptr<threepp::Group> m_visual;
    std::shared_ptr<threepp::Mesh> m_gateMesh;
    std::shared_ptr<threepp::Mesh> m_frameMesh;
    
    float m_openProgress{0.0f}; // 0.0 = closed, 1.0 = fully open
    const float m_openSpeed{1.0f}; // Units per second
    
    void createVisual();
    void animateOpening(float dt);
};

#endif // GATE_HPP
