#pragma once

#include "Coin.hpp"
#include <threepp/scenes/Scene.hpp>
#include <vector>
#include <memory>

class CoinManager {
public:
    CoinManager(threepp::Scene& scene);
    
    void spawnCoins(int count, float arenaRadius);
    void update(float dt);
    bool checkCollection(const threepp::Vector3& position, float collectionRadius = 2.0f);
    
    int getCollectedCount() const { return collectedCount_; }
    int getTotalCount() const { return static_cast<int>(coins_.size()); }
    
    void reset();
    
private:
    threepp::Scene& scene_;
    std::vector<std::unique_ptr<Coin>> coins_;
    int collectedCount_ = 0;
};
