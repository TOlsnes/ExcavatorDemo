#pragma once

#include <memory>

namespace threepp {
class Canvas;
class Scene;
class Mesh;
class KeyAdapter;
}

// Car: a simple movable block controlled by WASD keys.
class Car {
public:
    // Creates a block mesh and adds it to the scene. Registers key listeners on the canvas.
    Car(threepp::Canvas& canvas, threepp::Scene& scene);
    ~Car();

    // Call once per frame with delta time in seconds.
    void update(float dt);

    // Access underlying mesh (e.g., for external transforms if needed)
    threepp::Mesh* mesh();

    // Adjust movement speed in world units per second.
    void setSpeed(float unitsPerSecond);

private:
    threepp::Canvas& canvas_;
    threepp::Scene& scene_;
    std::shared_ptr<threepp::Mesh> mesh_;

    // key state
    bool wDown_{false}, aDown_{false}, sDown_{false}, dDown_{false};

    // listeners
    std::unique_ptr<threepp::KeyAdapter> pressListener_;
    std::unique_ptr<threepp::KeyAdapter> releaseListener_;

    float speed_{5.0f};
};
