#pragma once

#include <memory>

namespace threepp {
class Scene;
class Mesh;
}

// World: builds and holds the 3D scene graph.
// Currently creates a single plane mesh and adds it to the scene.
class World {
public:
    World();

    // Access the scene to render
    threepp::Scene& scene();
    const threepp::Scene& scene() const;

    // Access the plane mesh (may be nullptr if construction failed)
    threepp::Mesh* plane();

private:
    std::unique_ptr<threepp::Scene> scene_;
    std::shared_ptr<threepp::Mesh> plane_;
};
