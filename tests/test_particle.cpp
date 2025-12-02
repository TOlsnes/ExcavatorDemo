#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "ParticleSystem.hpp"
#include <threepp/scenes/Scene.hpp>
#include <threepp/math/Vector3.hpp>

TEST_CASE("ParticleSystem lifecycle", "[particle]") {
    threepp::Scene scene;
    ParticleSystem ps(scene);
    
    SECTION("Initially has no particles") {
        REQUIRE(ps.getActiveCount() == 0);
    }
    
    SECTION("Can spawn particle") {
        threepp::Vector3 pos(1.0f, 0.5f, 2.0f);
        ps.spawnParticle(pos);
        
        REQUIRE(ps.getActiveCount() == 1);
    }
    
    SECTION("Particles fade over time") {
        threepp::Vector3 pos(0.0f, 0.0f, 0.0f);
        ps.spawnParticle(pos);
        
        // Update for 5 seconds (exceeds lifetime)
        ps.update(5.0f);
        
        // Particle should be cleared
        REQUIRE(ps.getActiveCount() == 0);
    }
    
    SECTION("Can clear all particles") {
        ps.spawnParticle({1, 0, 0});
        ps.spawnParticle({2, 0, 0});
        ps.spawnParticle({3, 0, 0});
        
        REQUIRE(ps.getActiveCount() == 3);
        
        ps.clearParticles();
        REQUIRE(ps.getActiveCount() == 0);
    }
}
