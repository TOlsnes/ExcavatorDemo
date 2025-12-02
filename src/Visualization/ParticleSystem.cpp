#include "ParticleSystem.hpp"
#include <algorithm>
#include <random>

using namespace threepp;

ParticleSystem::ParticleSystem(Scene& scene)
    : scene_(scene) {
    // shared geometries for all particles
    sphereGeometry_ = SphereGeometry::create(0.05f, 6, 6); // Small sphere, low poly
    pyramidGeometry_ = ConeGeometry::create(0.05f, 0.1f, 4); // 4-sided cone = pyramid
    boxGeometry_ = BoxGeometry::create(0.08f, 0.08f, 0.08f); // Small box
    
    particleMaterial_ = MeshBasicMaterial::create();
    particleMaterial_->color = Color(0.5f, 0.45f, 0.4f); // Dusty brown
    particleMaterial_->transparent = true;
    particleMaterial_->opacity = 1.0f;
    particleMaterial_->depthWrite = false; // Avoid z-fighting with ground
}

void ParticleSystem::spawnParticle(const Vector3& position) {
    Particle p;
    p.lifetime = 0.0f;
    p.maxLifetime = 1.0f;
    
    // Add random offset to position so its not just a line
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(-0.1f, 0.1f);
    static std::uniform_int_distribution<int> shapeDis(0, 2);
    
    Vector3 randomPos = position;
    randomPos.x += dis(gen);
    randomPos.y += dis(gen) * 0.5f; // Less vertical randomness cus i dont want it being a cone
    randomPos.z += dis(gen);
    
    // Randomly select geometry type
    std::shared_ptr<BufferGeometry> selectedGeometry;
    int shapeType = shapeDis(gen);
    switch (shapeType) {
        case 0: selectedGeometry = sphereGeometry_; break;
        case 1: selectedGeometry = pyramidGeometry_; break;
        case 2: selectedGeometry = boxGeometry_; break;
        default: selectedGeometry = sphereGeometry_;
    }
    
    // Create new material for this particle
    auto mat = MeshBasicMaterial::create();
    mat->color = Color(0.5f, 0.45f, 0.4f); // Dusty brown
    mat->transparent = true;
    mat->opacity = 0.8f;
    mat->depthWrite = false;
    
    // Create mesh instance
    p.mesh = Mesh::create(selectedGeometry, mat);
    p.mesh->position.set(randomPos.x, randomPos.y + 0.05f, randomPos.z); // Slightly above ground
    
    scene_.add(p.mesh);
    particles_.push_back(p);
}

void ParticleSystem::update(float deltaTime) {
    // Update lifetimes and fade opacity
    for (auto& p : particles_) {
        p.lifetime += deltaTime;
        
        float normalizedLife = p.lifetime / p.maxLifetime;
        float opacity = 1.0f - normalizedLife;
        opacity = std::max(0.0f, opacity);
        
        auto mat = std::dynamic_pointer_cast<MeshBasicMaterial>(p.mesh->material());
        if (mat) {
            mat->opacity = opacity * 0.8f; // Scale to max 0.8
        }
        
        // Slight upward drift cus i can
        p.mesh->position.y += deltaTime * 0.1f;
    }
    
    // Remove dead particles (lifetime >= maxLifetime)
    particles_.erase(
        std::remove_if(particles_.begin(), particles_.end(),
            [this](const Particle& p) {
                if (p.lifetime >= p.maxLifetime) {
                    scene_.remove(*p.mesh);
                    return true;
                }
                return false;
            }),
        particles_.end()
    );
}

void ParticleSystem::clearParticles() {
    for (auto& p : particles_) {
        if (p.mesh) {
            scene_.remove(*p.mesh);
        }
    }
    particles_.clear();
}
