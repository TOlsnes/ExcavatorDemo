#ifndef DUMPZONE_HPP
#define DUMPZONE_HPP

#include <threepp/threepp.hpp>
#include <memory>

/**
 * DumpZone represents an area where excavated material can be dumped.
 * Tracks how many successful dumps have occurred.
 */
class DumpZone {
public:
    DumpZone(const threepp::Vector3& position, float radius);
    
    // Check if a point (bucket position) is within the dump zone
    bool isInZone(const threepp::Vector3& point) const;
    
    // Record a successful dump
    void recordDump();
    
    // Get total dumps made (unused)
    // int getDumpCount() const { return m_dumpCount; }
    
    // Get the visual representation for the scene
    std::shared_ptr<threepp::Group> getVisual() const { return m_visual; }
    
    // Get zone position and radius
    const threepp::Vector3& getPosition() const { return m_position; }
    float getRadius() const { return m_radius; }
    
    // Reset to initial state (clear dumps)
    void reset();
    
private:
    threepp::Vector3 m_position;
    float m_radius;
    int m_dumpCount{0};
    std::shared_ptr<threepp::Group> m_visual;
    std::shared_ptr<threepp::Mesh> m_pileMesh;
    
    void createVisual();
    void updatePileHeight();
};

#endif // DUMPZONE_HPP
