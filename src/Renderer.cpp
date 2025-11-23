#include "Renderer.hpp"

#include <threepp/threepp.hpp>

using namespace threepp;

Renderer::Renderer(Canvas& canvas)
    : canvas_(canvas),
      renderer_(std::make_unique<GLRenderer>(canvas.size())) {
    hookResize_();
}

Renderer::~Renderer() = default;

void Renderer::setClearColor(const Color& color) {
    renderer_->setClearColor(color);
}

void Renderer::setClearColor(const Color& color, float alpha) {
    renderer_->setClearColor(color, alpha);
}

void Renderer::setAutoClear(bool autoClear) {
    renderer_->autoClear = autoClear;
}

void Renderer::render(Scene& scene, Camera& camera) {
    renderer_->render(scene, camera);
}

void Renderer::hookResize_() {
    canvas_.onWindowResize([this](WindowSize size) {
        renderer_->setSize({size.width(), size.height()});
    });
}
