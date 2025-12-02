#pragma once

#include <memory>
#include <threepp/scenes/Scene.hpp>

// World: builds and holds the 3D scene graph.
class World {
public:
    World();

    // Access the scene to render
    threepp::Scene& scene();
    const threepp::Scene& scene() const;

private:
    std::unique_ptr<threepp::Scene> scene_;
};
