#ifndef DIGZONE_HPP
#define DIGZONE_HPP

#include <threepp/threepp.hpp>
#include <memory>

/**
 * DigZone represents a sand/rock pile that can be "dug" by the excavator bucket.
 * When the bucket enters this zone, it becomes loaded with material.
 */
class DigZone {
public:
    DigZone(const threepp::Vector3& position, float radius);
    
    // Check if a point (bucket position) is within the dig zone
    bool isInZone(const threepp::Vector3& point) const;
    
    // Get the visual representation for the scene
    std::shared_ptr<threepp::Group> getVisual() const { return m_visual; }
    
    // Get zone position and radius for collision checks
    const threepp::Vector3& getPosition() const { return m_position; }
    float getRadius() const { return m_radius; }

    // Reduce the pile by a fraction [0..1], returns true if changed
    bool dig(float fraction);
    float currentScale() const { return m_scale; }
    
    // Reset to initial state (full size)
    void reset();
    
private:
    threepp::Vector3 m_position;
    float m_radius;
    std::shared_ptr<threepp::Group> m_visual;
    float m_scale{1.0f};
    
    void createVisual();
};

#endif // DIGZONE_HPP
