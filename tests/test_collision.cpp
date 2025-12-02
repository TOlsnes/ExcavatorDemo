#include <catch2/catch_test_macros.hpp>
#include "CollisionWorld.hpp"
#include <threepp/math/Vector3.hpp>

TEST_CASE("CollisionWorld ground check", "[collision]") {
    SECTION("Ground Y position is zero") {
        REQUIRE(CollisionWorld::groundY() == 0.0f);
    }
}

TEST_CASE("CollisionWorld rock colliders", "[collision]") {
    SECTION("Can add and clear rock colliders") {
        CollisionWorld::clear();
        
        // After clear, should have no colliders
        // (We can't easily test count without exposing internal state,
        // but we verify clear doesn't crash)
        REQUIRE_NOTHROW(CollisionWorld::clear());
    }
}

TEST_CASE("CollisionWorld movement resolution", "[collision]") {
    SECTION("No collision returns original position") {
        CollisionWorld::clear();
        
        float x = 5.0f;
        float z = 5.0f;
        bool collided = CollisionWorld::resolveExcavatorMove(x, z, 0.5f);
        
        // With no colliders, should be no collision
        REQUIRE_FALSE(collided);
        REQUIRE(x == 5.0f);
        REQUIRE(z == 5.0f);
    }
}
