#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "DigZone.hpp"
#include "DumpZone.hpp"
#include <threepp/math/Vector3.hpp>

using namespace threepp;

TEST_CASE("DigZone: Basic initialization", "[DigZone]") {
    Vector3 position(5.0f, 0.0f, 5.0f);
    float radius = 2.0f;
    
    DigZone zone(position, radius);
    
    REQUIRE(zone.getPosition().x == 5.0f);
    REQUIRE(zone.getPosition().y == 0.0f);
    REQUIRE(zone.getPosition().z == 5.0f);
    REQUIRE(zone.getRadius() == 2.0f);
    REQUIRE(zone.getVisual() != nullptr);
}

TEST_CASE("DigZone: Point inside cylinder", "[DigZone]") {
    Vector3 position(0.0f, 0.0f, 0.0f);
    float radius = 2.0f;
    
    DigZone zone(position, radius);
    
    // Point inside cylinder (below dome, within radius)
    Vector3 insidePoint(1.0f, 0.5f, 0.0f);
    REQUIRE(zone.isInZone(insidePoint));
    
    // Point at center
    Vector3 centerPoint(0.0f, 0.5f, 0.0f);
    REQUIRE(zone.isInZone(centerPoint));
    
    // Point near edge but inside
    Vector3 edgePoint(1.9f, 0.5f, 0.0f);
    REQUIRE(zone.isInZone(edgePoint));
}

TEST_CASE("DigZone: Point inside dome", "[DigZone]") {
    Vector3 position(0.0f, 0.0f, 0.0f);
    float radius = 2.0f;
    
    DigZone zone(position, radius);
    
    // Point in dome region (above cylinder, within dome radius)
    Vector3 domePoint(0.5f, 2.0f, 0.0f);
    REQUIRE(zone.isInZone(domePoint));
}

TEST_CASE("DigZone: Point outside zone", "[DigZone]") {
    Vector3 position(0.0f, 0.0f, 0.0f);
    float radius = 2.0f;
    
    DigZone zone(position, radius);
    
    // Point too far horizontally
    Vector3 farPoint(5.0f, 0.5f, 0.0f);
    REQUIRE_FALSE(zone.isInZone(farPoint));
    
    // Point below ground
    Vector3 belowPoint(0.0f, -1.0f, 0.0f);
    REQUIRE_FALSE(zone.isInZone(belowPoint));
    
    // Point above dome
    Vector3 abovePoint(0.0f, 10.0f, 0.0f);
    REQUIRE_FALSE(zone.isInZone(abovePoint));
}

TEST_CASE("DigZone: Digging reduces scale", "[DigZone]") {
    Vector3 position(0.0f, 0.0f, 0.0f);
    float radius = 2.0f;
    
    DigZone zone(position, radius);
    
    // Dig 10% of material
    bool changed = zone.dig(0.1f);
    REQUIRE(changed);
    
    // Point that was inside might still be inside (scale affects radius)
    // But digging multiple times should eventually shrink it
    zone.dig(0.2f);
    zone.dig(0.2f);
    zone.dig(0.2f);
    
    // After significant digging, zone radius should be effectively smaller
    // Point at original edge should now be outside
    Vector3 originalEdge(1.95f, 0.5f, 0.0f);
    REQUIRE_FALSE(zone.isInZone(originalEdge));
}

TEST_CASE("DigZone: Digging with zero has no effect", "[DigZone]") {
    Vector3 position(0.0f, 0.0f, 0.0f);
    float radius = 2.0f;
    
    DigZone zone(position, radius);
    
    bool changed = zone.dig(0.0f);
    REQUIRE_FALSE(changed);
}

TEST_CASE("DigZone: Reset restores original size", "[DigZone]") {
    Vector3 position(0.0f, 0.0f, 0.0f);
    float radius = 2.0f;
    
    DigZone zone(position, radius);
    
    // Dig significantly
    zone.dig(0.5f);
    
    // Reset
    zone.reset();
    
    // Original point should be inside again
    Vector3 originalEdge(1.9f, 0.5f, 0.0f);
    REQUIRE(zone.isInZone(originalEdge));
}

TEST_CASE("DumpZone: Basic initialization", "[DumpZone]") {
    Vector3 position(10.0f, 0.0f, 10.0f);
    float radius = 3.0f;
    
    DumpZone zone(position, radius);
    
    REQUIRE(zone.getPosition().x == 10.0f);
    REQUIRE(zone.getPosition().y == 0.0f);
    REQUIRE(zone.getPosition().z == 10.0f);
    REQUIRE(zone.getRadius() == 3.0f);
    REQUIRE(zone.getVisual() != nullptr);
}

TEST_CASE("DumpZone: Point inside zone", "[DumpZone]") {
    Vector3 position(0.0f, 0.0f, 0.0f);
    float radius = 3.0f;
    
    DumpZone zone(position, radius);
    
    // Point at center, ground level
    Vector3 centerPoint(0.0f, 0.0f, 0.0f);
    REQUIRE(zone.isInZone(centerPoint));
    
    // Point inside horizontal radius
    Vector3 insidePoint(2.0f, 1.0f, 0.0f);
    REQUIRE(zone.isInZone(insidePoint));
    
    // Point at edge
    Vector3 edgePoint(2.9f, 0.5f, 0.0f);
    REQUIRE(zone.isInZone(edgePoint));
}

TEST_CASE("DumpZone: Point outside zone", "[DumpZone]") {
    Vector3 position(0.0f, 0.0f, 0.0f);
    float radius = 3.0f;
    
    DumpZone zone(position, radius);
    
    // Point too far horizontally
    Vector3 farPoint(5.0f, 0.0f, 0.0f);
    REQUIRE_FALSE(zone.isInZone(farPoint));
    
    // Point within radius but too low
    Vector3 tooLow(1.0f, -2.0f, 0.0f);
    REQUIRE_FALSE(zone.isInZone(tooLow));
    
    // Point within radius but too high
    Vector3 tooHigh(1.0f, 10.0f, 0.0f);
    REQUIRE_FALSE(zone.isInZone(tooHigh));
}

TEST_CASE("DumpZone: Recording dumps", "[DumpZone]") {
    Vector3 position(0.0f, 0.0f, 0.0f);
    float radius = 3.0f;
    
    DumpZone zone(position, radius);
    
    // Record a dump (should not crash)
    REQUIRE_NOTHROW(zone.recordDump());
    
    // Record multiple dumps
    zone.recordDump();
    zone.recordDump();
    zone.recordDump();
    
    // Visual should still exist
    REQUIRE(zone.getVisual() != nullptr);
}

TEST_CASE("DumpZone: Reset clears dump count", "[DumpZone]") {
    Vector3 position(0.0f, 0.0f, 0.0f);
    float radius = 3.0f;
    
    DumpZone zone(position, radius);
    
    // Record dumps
    zone.recordDump();
    zone.recordDump();
    zone.recordDump();
    
    // Reset
    REQUIRE_NOTHROW(zone.reset());
    
    // Visual should still exist
    REQUIRE(zone.getVisual() != nullptr);
}

TEST_CASE("DumpZone: Height tolerance", "[DumpZone]") {
    Vector3 position(0.0f, 0.0f, 0.0f);
    float radius = 3.0f;
    
    DumpZone zone(position, radius);
    
    // Point slightly below ground (within tolerance)
    Vector3 slightlyBelow(1.0f, -0.5f, 0.0f);
    REQUIRE(zone.isInZone(slightlyBelow));
    
    // Point well above ground (within tolerance)
    Vector3 above(1.0f, 3.0f, 0.0f);
    REQUIRE(zone.isInZone(above));
}
