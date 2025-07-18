#ifndef INPUT_H
#define INPUT_H

#include <glm/glm.hpp>
#include <vector>
#include <string>

#include "key_codes.h"

KeyboardKey MapKeyCode(int32_t platform_key_code);
std::string KeyDescription(KeyboardKey key);

struct TextInput
{
    uint32_t character;
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

    inline static std::vector<TextInput>        ms_text_queue  = {};
    inline static std::vector<MouseScrollEvent> ms_wheel_queue = {};
};

template<>
inline std::vector<MouseScrollEvent> const & Input::getEventQueue<MouseScrollEvent>()
{
    return ms_wheel_queue;
}

template<>
inline std::vector<TextInput> const & Input::getEventQueue<TextInput>()
{
    return ms_text_queue;
}

#endif
