#ifndef INPUT_H
#define INPUT_H

#include <glm/glm.hpp>
#include <vector>
#include <string>

enum class KeyboardKey
{
    Key_0,
    Key_1,
    Key_2,
    Key_4,
    Key_3,
    Key_5,
    Key_6,
    Key_7,
    Key_8,
    Key_9,

    Key_A,
    Key_B,
    Key_C,
    Key_D,
    Key_E,
    Key_F,
    Key_G,
    Key_H,
    Key_I,
    Key_J,
    Key_K,
    Key_L,
    Key_M,
    Key_N,
    Key_O,
    Key_P,
    Key_Q,
    Key_R,
    Key_S,
    Key_T,
    Key_U,
    Key_V,
    Key_W,
    Key_X,
    Key_Y,
    Key_Z,

    Key_F1,
    Key_F2,
    Key_F3,
    Key_F4,
    Key_F5,
    Key_F6,
    Key_F7,
    Key_F8,
    Key_F9,
    Key_F10,
    Key_F11,
    Key_F12,

    Key_UpArrow,
    Key_DownArrow,
    Key_LeftArrow,
    Key_RightArrow,

    Key_Home,
    Key_End,
    Key_PageUp,
    Key_PageDown,
    Key_Insert,
    Key_Delete,

    Key_Escape,

    Key_NumLock,
    Key_NumPad0,
    Key_NumPad1,
    Key_NumPad2,
    Key_NumPad3,
    Key_NumPad4,
    Key_NumPad5,
    Key_NumPad6,
    Key_NumPad7,
    Key_NumPad8,
    Key_NumPad9,
    Key_NumPadEnter,
    Key_NumSubtract,   // (-) on numeric keypad
    Key_NumAdd,        // (+) on numeric keypad
    Key_NumMultiply,   // (*) on numeric keypad
    Key_NumDivide,     // (/) on numeric keypad
    Key_NumPoint,      // PERIOD (decimal point) on numeric keypad
    Key_NumEqual,

    Key_LeftCtrl,    // Left CTRL
    Key_LeftAlt,     // Left ALT
    Key_LeftShift,   // Left SHIFT
    Key_LeftWin,     // Left Windows logo key

    Key_RightCtrl,    // Right CTRL
    Key_RightAlt,     // Right ALT
    Key_RightShift,   // Right SHIFT
    Key_RightWin,     // Right Windows logo key
    Key_MenuKey,      // Application key

    Key_PrintScreen,
    Key_ScrollLock,
    Key_Pause,

    Key_Spacebar,
    Key_Backspace,
    Key_Enter,   // ENTER on main keyboard
    Key_Tab,
    Key_CapsLock,
    Key_LeftBracket,    // Left square bracket [
    Key_RightBracket,   // Right square bracket ]
    Key_Slash,          // (/) On main keyboard
    Key_Backslash,      // (\)
    Key_Comma,          // (,)
    Key_Semicolon,      // (;)
    Key_Period,         // (.) On main keyboard
    Key_Grave,          // (`) Grave accent
    Key_Apostrophe,     // (')
    Key_Minus,          // (-) On main keyboard
    Key_Equals,

    Key_MaxKeyNum   // Max and unknown code
};

enum class MouseButton
{
    Left,
    Right,
    Middle,

    ButtonsCount
};

KeyboardKey MapKeyCode(int32_t platform_key_code);
std::string KeyDescription(KeyboardKey key);

struct TextInput
{
    std::uint32_t character;
};

struct MouseScrollEvent
{
    glm::vec2 old_pos;
    glm::vec2 new_pos;
};

class Input
{
public:
    Input()          = default;
    virtual ~Input() = default;

    void resize(int32_t w, int32_t h) { m_screen_size = glm::ivec2{w, h}; }

    // Keyboard
    KeyboardKey getKeyPressed() const { return m_last_key; }
    bool        isKeyPressed(KeyboardKey key_id) const { return m_keys_states[static_cast<size_t>(key_id)]; }
    bool        isAnyKeyPressed() const;

    // Mouse
    glm::ivec2 getMousePosition() const
    {
        return {m_mouse_position.x, m_screen_size.y - m_mouse_position.y - 1};
    }
    bool    isMouseButtonPressed(MouseButton button_id) const;
    int32_t getMouseWheel() { return 0; }

    template<class T>
    std::vector<T> const & getEventQueue();
    void                   clearEventQueues();

    // functions for platform callbacks to call
    void buttonEvent(MouseButton button_id, bool press);
    void mousePos(int32_t xpos, int32_t ypos);
    void mouseWhell(int32_t offset) {}
    void keyEvent(KeyboardKey key, bool press);

protected:
    glm::ivec2 m_screen_size;
    // Keyboard states
    KeyboardKey m_last_key                                                     = KeyboardKey::Key_MaxKeyNum;
    bool        m_keys_states[static_cast<size_t>(KeyboardKey::Key_MaxKeyNum)] = {};
    // Mouse states
    glm::ivec2 m_mouse_position                                                      = {};
    bool       m_mouse_buttons_state[static_cast<size_t>(MouseButton::ButtonsCount)] = {};

    static std::vector<TextInput>        m_text_queue;
    static std::vector<MouseScrollEvent> m_wheel_queue;
};

template<>
inline std::vector<MouseScrollEvent> const & Input::getEventQueue<MouseScrollEvent>()
{
    return m_wheel_queue;
}

template<>
inline std::vector<TextInput> const & Input::getEventQueue<TextInput>()
{
    return m_text_queue;
}

#endif
