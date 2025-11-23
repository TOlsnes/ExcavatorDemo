#include "Car.hpp"

#include <threepp/threepp.hpp>

using namespace threepp;

Car::Car(Canvas& canvas, Scene& scene)
    : canvas_(canvas), scene_(scene) {

    // Create a simple block: 1 x 0.5 x 2
    auto geom = BoxGeometry::create(1.f, 0.5f, 2.f);
    auto mat = MeshBasicMaterial::create();
    mat->color = Color::blue; // blue car
    mesh_ = Mesh::create(geom, mat);

    // Start slightly above the plane
    mesh_->position.set(0, 0.25f, 0);

    scene_.add(mesh_);

    // Key listeners: pressed / released
    pressListener_ = std::make_unique<KeyAdapter>(KeyAdapter::KEY_PRESSED, [this](KeyEvent evt) {
        switch (evt.key) {
            case Key::W: wDown_ = true; break;
            case Key::A: aDown_ = true; break;
            case Key::S: sDown_ = true; break;
            case Key::D: dDown_ = true; break;
            default: break;
        }
    });
    canvas_.addKeyListener(*pressListener_);

    releaseListener_ = std::make_unique<KeyAdapter>(KeyAdapter::KEY_RELEASED, [this](KeyEvent evt) {
        switch (evt.key) {
            case Key::W: wDown_ = false; break;
            case Key::A: aDown_ = false; break;
            case Key::S: sDown_ = false; break;
            case Key::D: dDown_ = false; break;
            default: break;
        }
    });
    canvas_.addKeyListener(*releaseListener_);
}

Car::~Car() {
    // Optional: could remove listeners, but Canvas destruction or process exit will clean these up.
}

void Car::update(float dt) {
    if (!mesh_) return;

    // Move in XZ plane. Assume forward (-Z) on W, back (+Z) on S, left (-X) on A, right (+X) on D
    Vector3 delta(0, 0, 0);
    if (wDown_) delta.z -= 1.f;
    if (sDown_) delta.z += 1.f;
    if (aDown_) delta.x -= 1.f;
    if (dDown_) delta.x += 1.f;

    if (delta.lengthSq() > 0) {
        delta.normalize();
        delta.multiplyScalar(speed_ * dt);
        mesh_->position.add(delta);
    }
}

Mesh* Car::mesh() { return mesh_.get(); }

void Car::setSpeed(float unitsPerSecond) { speed_ = unitsPerSecond; }
