#include <threepp/threepp.hpp>

#include "World.hpp"
#include "Renderer.hpp"
#include "Car.hpp"

using namespace threepp;

int main() {
    Canvas canvas;
    Renderer renderer(canvas);
    renderer.setClearColor(Color(0.5f, 0.7f, 1.0f)); // Sky blue background

    World world;

    // Create a car and simple update loop
    Car car(canvas, world.scene());
    car.setSpeed(6.0f);

    PerspectiveCamera camera(60, canvas.aspect(), 0.1, 1000);
    camera.position.set(0, 5, 10);
    camera.lookAt(0, 0, 0);

    Clock clock;

    canvas.animate([&] {
        float dt = clock.getDelta();
        car.update(dt);
        renderer.render(world.scene(), camera);
    });

    return 0;
}

