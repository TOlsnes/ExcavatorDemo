#include "World.hpp"

#include <threepp/threepp.hpp>

using namespace threepp;

World::World()
    : scene_(std::make_unique<Scene>()) {
    // Ground plane now created by ObjectSpawner
}

Scene& World::scene() { return *scene_; }
const Scene& World::scene() const { return *scene_; }
