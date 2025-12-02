#pragma once

#include <memory>
#include <threepp/renderers/GLRenderer.hpp>

// Forward declarations for parameters
namespace threepp {
    class Canvas;
    class Scene;
    class Camera;
    class Color;
}

// Renderer: wraps threepp::GLRenderer and renders a Scene on a Canvas.
class Renderer {
public:
    explicit Renderer(threepp::Canvas& canvas);
    ~Renderer();

    // Optional convenience: set clear color
    void setClearColor(const threepp::Color& color);
    
    // Set clear color with alpha (for fade effects)
    void setClearColor(const threepp::Color& color, float alpha);
    
    // Control auto clear (for multi-pass rendering)
    void setAutoClear(bool autoClear);

    // Render once (call inside your animate loop)
    void render(threepp::Scene& scene, threepp::Camera& camera);

private:
    threepp::Canvas& canvas_;
    std::unique_ptr<threepp::GLRenderer> renderer_;
    void hookResize_();
};
