#include "CoinManager.hpp"
#include "Settings.hpp"
#include <threepp/geometries/CylinderGeometry.hpp>
#include <threepp/materials/MeshStandardMaterial.hpp>
#include <threepp/math/MathUtils.hpp>
#include <random>

using namespace Settings;

CoinManager::CoinManager(threepp::Scene& scene) : scene_(scene) {}

void CoinManager::spawnCoins(int count, float arenaRadius) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, threepp::math::TWO_PI);
    std::uniform_real_distribution<float> radiusDist(5.0f, arenaRadius - 3.0f);
    std::uniform_real_distribution<float> rotationDist(0.0f, threepp::math::TWO_PI);
    
    auto coinGeometry = threepp::CylinderGeometry::create(0.5f, 0.5f, 0.1f, 16);
    auto coinMaterial = threepp::MeshStandardMaterial::create();
    coinMaterial->color.setHex(0xFFD700); // Gold 
    coinMaterial->metalness = 0.8f;
    coinMaterial->roughness = 0.2f;
    
    for (int i = 0; i < count; ++i) {
        float angle = angleDist(gen);
        float radius = radiusDist(gen);
        
        threepp::Vector3 pos(
            radius * std::cos(angle),
            1.0f, // Hover above ground
            radius * std::sin(angle)
        );
        
        auto coinMesh = threepp::Mesh::create(coinGeometry, coinMaterial);
        coinMesh->rotation.x = threepp::math::PI / 2.0f; // Lay flat initially
        coinMesh->rotation.z = rotationDist(gen); // Random starting rotation
        scene_.add(coinMesh);
        
        auto coin = std::make_unique<Coin>(pos, coinMesh);
        coins_.push_back(std::move(coin));
    }
}

void CoinManager::update(float dt) {
    for (auto& coin : coins_) {
        coin->update(dt);
    }
}

bool CoinManager::checkCollection(const threepp::Vector3& position, float collectionRadius) {
    bool collected = false;
    for (auto& coin : coins_) {
        if (!coin->isCollected()) {
            float distance = position.distanceTo(coin->getPosition());
            if (distance < collectionRadius) {
                coin->collect();
                scene_.remove(*coin->getMesh());
                collectedCount_++;
                collected = true;
            }
        }
    }
    return collected;
}

void CoinManager::reset() {
    for (auto& coin : coins_) {
        scene_.remove(*coin->getMesh());
    }
    coins_.clear();
    collectedCount_ = 0;
}
