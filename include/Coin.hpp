#pragma once

#include <threepp/math/Vector3.hpp>
#include <threepp/objects/Mesh.hpp>
#include <memory>

class Coin {
public:
    Coin(const threepp::Vector3& position, std::shared_ptr<threepp::Mesh> mesh);
    
    bool isCollected() const { return collected_; }
    void collect() { collected_ = true; }
    
    const threepp::Vector3& getPosition() const { return position_; }
    std::shared_ptr<threepp::Mesh> getMesh() { return mesh_; }
    
    void update(float dt);
    
private:
    threepp::Vector3 position_;
    std::shared_ptr<threepp::Mesh> mesh_;
    bool collected_ = false;
    float rotationSpeed_ = 2.0f;
    float bobHeight_ = 0.5f;
    float bobSpeed_ = 3.0f;
    float time_ = 0.0f;
};
