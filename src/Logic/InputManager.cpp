#include "InputManager.hpp"

#include <threepp/threepp.hpp>
#include <threepp/canvas/Canvas.hpp>
#include <threepp/input/KeyListener.hpp>
#include <threepp/input/MouseListener.hpp>

#include <iostream>

using namespace threepp;

struct InputManager::Listeners {
    std::unique_ptr<threepp::KeyAdapter> onKeyPress;
    std::unique_ptr<threepp::KeyAdapter> onKeyRelease;
    struct MouseImpl: threepp::MouseListener {
        InputManager* self;
        explicit MouseImpl(InputManager* s): self(s) {}
        void onMouseDown(int button, const threepp::Vector2& pos) override {
            if (button >= 0 && button < static_cast<int>(self->m_mouseButtonStates.size())) {
                self->m_mouseButtonStates[button] = true;
            }
            if (self->m_mouseButtonCallback) {
                self->m_mouseButtonCallback(static_cast<MouseButton>(button), true);
            }
        }
        void onMouseUp(int button, const threepp::Vector2& pos) override {
            if (button >= 0 && button < static_cast<int>(self->m_mouseButtonStates.size())) {
                self->m_mouseButtonStates[button] = false;
            }
            if (self->m_mouseButtonCallback) {
                self->m_mouseButtonCallback(static_cast<MouseButton>(button), false);
            }
        }
        void onMouseMove(const threepp::Vector2& pos) override {
            if (self->m_firstMouseInput) {
                self->m_mouseX = pos.x;
                self->m_mouseY = pos.y;
                self->m_firstMouseInput = false;
            }

            self->m_previousMouseX = self->m_mouseX;
            self->m_previousMouseY = self->m_mouseY;
            self->m_mouseX = pos.x;
            self->m_mouseY = pos.y;
            self->m_mouseDeltaX = self->m_mouseX - self->m_previousMouseX;
            self->m_mouseDeltaY = self->m_mouseY - self->m_previousMouseY;

            if (self->m_mouseMoveCallback) {
                self->m_mouseMoveCallback(self->m_mouseX, self->m_mouseY, self->m_mouseDeltaX, self->m_mouseDeltaY);
            }
        }
    };
    std::unique_ptr<MouseImpl> mouse;
};

InputManager::InputManager()
    : m_canvas(nullptr),
      m_mouseX(0.0), m_mouseY(0.0),
      m_previousMouseX(0.0), m_previousMouseY(0.0),
      m_mouseDeltaX(0.0), m_mouseDeltaY(0.0),
      m_firstMouseInput(true) {

    m_keyStates.fill(false);
    m_previousKeyStates.fill(false);
    m_mouseButtonStates.fill(false);
    m_previousMouseButtonStates.fill(false);
}

InputManager::~InputManager() = default;

bool InputManager::initialize(threepp::Canvas* canvas) {
    if (!canvas) return false;
    std::cout << "InputManager::initialize() - Using threepp Canvas" << std::endl;
    m_canvas = canvas;

    m_listeners = std::make_unique<Listeners>();

    // --- Key Events ---
    m_listeners->onKeyPress = std::make_unique<threepp::KeyAdapter>(threepp::KeyAdapter::KEY_PRESSED, [this](threepp::KeyEvent event) {
        const int key = static_cast<int>(event.key);
        if (key >= 0 && key < static_cast<int>(m_keyStates.size())) {
            m_keyStates[key] = true;
        }
        if (m_keyCallback) m_keyCallback(static_cast<Key>(key), true);
    });
    m_canvas->addKeyListener(*m_listeners->onKeyPress);

    m_listeners->onKeyRelease = std::make_unique<threepp::KeyAdapter>(threepp::KeyAdapter::KEY_RELEASED, [this](threepp::KeyEvent event) {
        const int key = static_cast<int>(event.key);
        if (key >= 0 && key < static_cast<int>(m_keyStates.size())) {
            m_keyStates[key] = false;
        }
        if (m_keyCallback) m_keyCallback(static_cast<Key>(key), false);
    });
    m_canvas->addKeyListener(*m_listeners->onKeyRelease);

    // --- Mouse Events ---
    m_listeners->mouse = std::make_unique<Listeners::MouseImpl>(this);
    m_canvas->addMouseListener(*m_listeners->mouse);

    return true;
}

void InputManager::update() {
    // Update previous states for edge detection
    m_previousKeyStates = m_keyStates;
    m_previousMouseButtonStates = m_mouseButtonStates;
}

bool InputManager::isKeyPressed(Key key) const {
    int code = static_cast<int>(key);
    return (code >= 0 && code < static_cast<int>(m_keyStates.size())) && m_keyStates[code];
}
// Just comenting it out cus its unused but could be useful later
// bool InputManager::wasKeyJustPressed(Key key) const {
//     int code = static_cast<int>(key);
//     return (code >= 0 && code < static_cast<int>(m_keyStates.size())) &&
//            (m_keyStates[code] && !m_previousKeyStates[code]);
// }

// bool InputManager::wasKeyJustReleased(Key key) const {
//     int code = static_cast<int>(key);
//     return (code >= 0 && code < static_cast<int>(m_keyStates.size())) &&
//            (!m_keyStates[code] && m_previousKeyStates[code]);
// }

// bool InputManager::isMouseButtonPressed(MouseButton button) const {
//     int code = static_cast<int>(button);
//     return (code >= 0 && code < static_cast<int>(m_mouseButtonStates.size())) &&
//            m_mouseButtonStates[code];
// }

// void InputManager::getMousePosition(double& x, double& y) const {
//     x = m_mouseX;
//     y = m_mouseY;
// }

// void InputManager::getMouseDelta(double& dx, double& dy) const {
//     dx = m_mouseDeltaX;
//     dy = m_mouseDeltaY;
// }

// void InputManager::setCursorEnabled(bool enabled) {
//     // threepp::Canvas does not currently expose cursor enable/disable.
//     // This is a no-op placeholder to maintain API compatibility.
//     (void)enabled;
// }