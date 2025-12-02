#pragma once

#include <array>
#include <functional>
#include <memory>

namespace threepp {
    class Canvas;
}

/**
 * Manages keyboard and mouse input handling for threepp applications.
 * Provides a simple interface for querying input state and registering callbacks.
 */
class InputManager {
public:
    // Key codes (simplified subset for our needs)
    enum class Key {
        Unknown = -1,
        W = 87, A = 65, S = 83, D = 68,
        Q = 81, E = 69, Z = 90, X = 88,
        ArrowUp = 265, ArrowDown = 264, ArrowLeft = 263, ArrowRight = 262,
        Escape = 256,
        Space = 32,
        Enter = 257
    };

    // Mouse buttons
    enum class MouseButton {
        Left = 0,
        Right = 1,
        Middle = 2
    };

    // Input callback types
    using KeyCallback = std::function<void(Key key, bool pressed)>;
    using MouseButtonCallback = std::function<void(MouseButton button, bool pressed)>;
    using MouseMoveCallback = std::function<void(double xpos, double ypos, double deltaX, double deltaY)>;

    InputManager();
    ~InputManager();

    /**
     * Initialize the input manager with a threepp Canvas
     * @param canvas threepp Canvas pointer
     * @return true if initialization succeeded
     */
    bool initialize(threepp::Canvas* canvas);

    /**
     * Update input state (call once per frame)
     */
    void update();

    /**
     * Check if a key is currently pressed
     * @param key Key to check
     * @return true if pressed, false otherwise
     */
    bool isKeyPressed(Key key) const;

    // Unused methods - commented out
    // /**
    //  * Check if a key was just pressed this frame
    //  * @param key Key to check
    //  * @return true if just pressed, false otherwise
    //  */
    // bool wasKeyJustPressed(Key key) const;

    // /**
    //  * Check if a key was just released this frame
    //  * @param key Key to check
    //  * @return true if just released, false otherwise
    //  */
    // bool wasKeyJustReleased(Key key) const;

    // /**
    //  * Check if a mouse button is currently pressed
    //  * @param button Mouse button to check
    //  * @return true if pressed, false otherwise
    //  */
    // bool isMouseButtonPressed(MouseButton button) const;

    // /**
    //  * Get current mouse position
    //  * @param x Output x coordinate
    //  * @param y Output y coordinate
    //  */
    // void getMousePosition(double& x, double& y) const;

    // /**
    //  * Get mouse movement delta since last frame
    //  * @param deltaX Output x delta
    //  * @param deltaY Output y delta
    //  */
    // void getMouseDelta(double& deltaX, double& deltaY) const;

    // Unused callback setters - commented out
    // /**
    //  * Set key callback
    //  * @param callback Function to call on key events
    //  */
    // void setKeyCallback(KeyCallback callback);

    // /**
    //  * Set mouse button callback
    //  * @param callback Function to call on mouse button events
    //  */
    // void setMouseButtonCallback(MouseButtonCallback callback);

    // /**
    //  * Set mouse movement callback
    //  * @param callback Function to call on mouse movement
    //  */
    // void setMouseMoveCallback(MouseMoveCallback callback);

    // /**
    //  * Enable/disable mouse cursor
    //  * @param enabled true to show cursor, false to hide and capture
    //  */
    // void setCursorEnabled(bool enabled);

private:
    threepp::Canvas* m_canvas;

    // Listener storage to keep references alive while registered on Canvas
    struct Listeners;
    std::unique_ptr<Listeners> m_listeners;

    // Input state tracking
    std::array<bool, 512> m_keyStates;
    std::array<bool, 512> m_previousKeyStates;
    std::array<bool, 8> m_mouseButtonStates;
    std::array<bool, 8> m_previousMouseButtonStates;

    // Mouse state
    double m_mouseX, m_mouseY;
    double m_previousMouseX, m_previousMouseY;
    double m_mouseDeltaX, m_mouseDeltaY;
    bool m_firstMouseInput;

    // Callbacks
    KeyCallback m_keyCallback;
    MouseButtonCallback m_mouseButtonCallback;
    MouseMoveCallback m_mouseMoveCallback;
};