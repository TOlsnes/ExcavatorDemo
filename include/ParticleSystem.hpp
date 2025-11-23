#pragma once

#include <threepp/threepp.hpp>
#include <vector>
#include <memory>

/**
 * ParticleSystem: Manages dust/dirt particles that spawn during excavator movement.
 * Particles fade out over a lifetime and are automatically cleaned up.
 */
class ParticleSystem {
public:
    struct Particle {
        std::shared_ptr<threepp::Mesh> mesh;
        float lifetime{0.0f};  // Time alive (seconds)
        float maxLifetime{1.0f}; // Time until fully transparent
    };

    ParticleSystem(threepp::Scene& scene);

    // Spawn a particle at a given position
    void spawnParticle(const threepp::Vector3& position);

    // Update all particles (fade, remove dead ones)
    void update(float deltaTime);

    // Get count for debugging
    size_t getActiveCount() const { return particles_.size(); }
    
    // Get all particles (for reset cleanup)
    const std::vector<Particle>& getParticles() const { return particles_; }
    
    // Clear all particles
    void clearParticles();

private:
    threepp::Scene& scene_;
    std::vector<Particle> particles_;
    std::shared_ptr<threepp::MeshBasicMaterial> particleMaterial_;
    std::shared_ptr<threepp::BufferGeometry> sphereGeometry_;
    std::shared_ptr<threepp::BufferGeometry> pyramidGeometry_;
    std::shared_ptr<threepp::BufferGeometry> boxGeometry_;
};
