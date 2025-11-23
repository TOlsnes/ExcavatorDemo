#pragma once

#include <threepp/math/Vector3.hpp>
#include <threepp/objects/Mesh.hpp>
#include <memory>
#include "Settings.hpp"

class Coin {
public:
    Coin(const threepp::Vector3& position, std::shared_ptr<threepp::Mesh> mesh);
    
    bool isCollected() const { return Settings::collected_; }
    void collect() { Settings::collected_ = true; }
    
    const threepp::Vector3& getPosition() const { return position_; }
    std::shared_ptr<threepp::Mesh> getMesh() { return mesh_; }
    
    void update(float dt);
    
private:
    threepp::Vector3 position_;
    std::shared_ptr<threepp::Mesh> mesh_;
};
