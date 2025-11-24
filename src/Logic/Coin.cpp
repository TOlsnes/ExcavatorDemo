#include "Coin.hpp"
#include "Settings.hpp"
#include <cmath>

using namespace Settings;

Coin::Coin(const threepp::Vector3& position, std::shared_ptr<threepp::Mesh> mesh)
    : position_(position), mesh_(mesh) {
    mesh_->position.copy(position_);
}

void Coin::update(float dt) {
    if (collected_) return;
    
    time_ += dt;
    
    // Rotate around Z axis (coin spinning flat)
    mesh_->rotation.z += rotationSpeed_ * dt;
    
    // Bob up and down
    float bob = std::sin(time_ * bobSpeed_) * bobHeight_;
    mesh_->position.y = position_.y + bob;
}
