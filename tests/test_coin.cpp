#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Coin.hpp"
#include "CoinManager.hpp"
#include <threepp/scenes/Scene.hpp>
#include <threepp/geometries/CylinderGeometry.hpp>
#include <threepp/materials/MeshStandardMaterial.hpp>

TEST_CASE("Coin state management", "[coin]") {
    threepp::Vector3 pos(1.0f, 1.0f, 1.0f);
    auto geom = threepp::CylinderGeometry::create(0.5f, 0.5f, 0.1f);
    auto mat = threepp::MeshStandardMaterial::create();
    auto mesh = threepp::Mesh::create(geom, mat);
    
    Coin coin(pos, mesh);
    
    SECTION("Initially not collected") {
        REQUIRE_FALSE(coin.isCollected());
    }
    
    SECTION("Can be collected") {
        coin.collect();
        REQUIRE(coin.isCollected());
    }
    
    SECTION("Update stops after collection") {
        coin.collect();
        float initialY = mesh->position.y;
        
        // Update should do nothing when collected
        coin.update(1.0f);
        
        REQUIRE(mesh->position.y == initialY);
    }
}

TEST_CASE("CoinManager spawning and collection", "[coin]") {
    threepp::Scene scene;
    CoinManager manager(scene);
    
    SECTION("Can spawn coins") {
        manager.spawnCoins(5, 15.0f);
        
        REQUIRE(manager.getTotalCount() == 5);
        REQUIRE(manager.getCollectedCount() == 0);
    }
    
    SECTION("Can collect coins in radius") {
        manager.spawnCoins(3, 10.0f);
        
        // Try to collect at origin
        threepp::Vector3 excavatorPos(0.0f, 0.0f, 0.0f);
        bool collected = manager.checkCollection(excavatorPos, 50.0f); // Large radius
        
        // Should collect at least some coins
        REQUIRE(manager.getCollectedCount() > 0);
    }
    
    SECTION("Reset clears all coins") {
        manager.spawnCoins(5, 10.0f);
        manager.reset();
        
        REQUIRE(manager.getTotalCount() == 0);
        REQUIRE(manager.getCollectedCount() == 0);
    }
}
