#include "input.h"
#include <GLFW/glfw3.h>
#include <algorithm>

KeyboardKey MapKeyCode(int32_t platformKeyCode)
{
    KeyboardKey key = KeyboardKey::Key_MaxKeyNum;

    switch(platformKeyCode)
    {
        case GLFW_KEY_SPACE:
            key = KeyboardKey::Key_Spacebar;
            break;
        case GLFW_KEY_APOSTROPHE:
            key = KeyboardKey::Key_Apostrophe;
            break;
        case GLFW_KEY_COMMA:
            key = KeyboardKey::Key_Comma;
            break;
        case GLFW_KEY_MINUS:
            key = KeyboardKey::Key_Minus;
            break;
        case GLFW_KEY_PERIOD:
            key = KeyboardKey::Key_Period;
            break;
        case GLFW_KEY_SLASH:
            key = KeyboardKey::Key_Slash;
            break;
        case GLFW_KEY_0:
            key = KeyboardKey::Key_0;
            break;
        case GLFW_KEY_1:
            key = KeyboardKey::Key_1;
            break;
        case GLFW_KEY_2:
            key = KeyboardKey::Key_2;
            break;
        case GLFW_KEY_3:
            key = KeyboardKey::Key_3;
            break;
        case GLFW_KEY_4:
            key = KeyboardKey::Key_4;
            break;
        case GLFW_KEY_5:
            key = KeyboardKey::Key_5;
            break;
        case GLFW_KEY_6:
            key = KeyboardKey::Key_6;
            break;
        case GLFW_KEY_7:
            key = KeyboardKey::Key_7;
            break;
        case GLFW_KEY_8:
            key = KeyboardKey::Key_8;
            break;
        case GLFW_KEY_9:
            key = KeyboardKey::Key_9;
            break;
        case GLFW_KEY_SEMICOLON:
            key = KeyboardKey::Key_Semicolon;
            break;
        case GLFW_KEY_EQUAL:
            key = KeyboardKey::Key_Equals;
            break;
        case GLFW_KEY_A:
            key = KeyboardKey::Key_A;
            break;
        case GLFW_KEY_B:
            key = KeyboardKey::Key_B;
            break;
        case GLFW_KEY_C:
            key = KeyboardKey::Key_C;
            break;
        case GLFW_KEY_D:
            key = KeyboardKey::Key_D;
            break;
        case GLFW_KEY_E:
            key = KeyboardKey::Key_E;
            break;
        case GLFW_KEY_F:
            key = KeyboardKey::Key_F;
            break;
        case GLFW_KEY_G:
            key = KeyboardKey::Key_G;
            break;
        case GLFW_KEY_H:
            key = KeyboardKey::Key_H;
            break;
        case GLFW_KEY_I:
            key = KeyboardKey::Key_I;
            break;
        case GLFW_KEY_J:
            key = KeyboardKey::Key_J;
            break;
        case GLFW_KEY_K:
            key = KeyboardKey::Key_K;
            break;
        case GLFW_KEY_L:
            key = KeyboardKey::Key_L;
            break;
        case GLFW_KEY_M:
            key = KeyboardKey::Key_M;
            break;
        case GLFW_KEY_N:
            key = KeyboardKey::Key_N;
            break;
        case GLFW_KEY_O:
            key = KeyboardKey::Key_O;
            break;
        case GLFW_KEY_P:
            key = KeyboardKey::Key_P;
            break;
        case GLFW_KEY_Q:
            key = KeyboardKey::Key_Q;
            break;
        case GLFW_KEY_R:
            key = KeyboardKey::Key_R;
            break;
        case GLFW_KEY_S:
            key = KeyboardKey::Key_S;
            break;
        case GLFW_KEY_T:
            key = KeyboardKey::Key_T;
            break;
        case GLFW_KEY_U:
            key = KeyboardKey::Key_U;
            break;
        case GLFW_KEY_V:
            key = KeyboardKey::Key_V;
            break;
        case GLFW_KEY_W:
            key = KeyboardKey::Key_W;
            break;
        case GLFW_KEY_X:
            key = KeyboardKey::Key_X;
            break;
        case GLFW_KEY_Y:
            key = KeyboardKey::Key_Y;
            break;
        case GLFW_KEY_Z:
            key = KeyboardKey::Key_Z;
            break;
        case GLFW_KEY_LEFT_BRACKET:
            key = KeyboardKey::Key_LeftBracket;
            break;
        case GLFW_KEY_BACKSLASH:
            key = KeyboardKey::Key_Backslash;
            break;
        case GLFW_KEY_RIGHT_BRACKET:
            key = KeyboardKey::Key_RightBracket;
            break;
        case GLFW_KEY_GRAVE_ACCENT:
            key = KeyboardKey::Key_Grave;
            break;
        case GLFW_KEY_ESCAPE:
            key = KeyboardKey::Key_Escape;
            break;
        case GLFW_KEY_ENTER:
            key = KeyboardKey::Key_Enter;
            break;
        case GLFW_KEY_TAB:
            key = KeyboardKey::Key_Tab;
            break;
        case GLFW_KEY_BACKSPACE:
            key = KeyboardKey::Key_Backspace;
            break;
        case GLFW_KEY_INSERT:
            key = KeyboardKey::Key_Insert;
            break;
        case GLFW_KEY_DELETE:
            key = KeyboardKey::Key_Delete;
            break;
        case GLFW_KEY_RIGHT:
            key = KeyboardKey::Key_RightArrow;
            break;
        case GLFW_KEY_LEFT:
            key = KeyboardKey::Key_LeftArrow;
            break;
        case GLFW_KEY_DOWN:
            key = KeyboardKey::Key_DownArrow;
            break;
        case GLFW_KEY_UP:
            key = KeyboardKey::Key_UpArrow;
            break;
        case GLFW_KEY_PAGE_UP:
            key = KeyboardKey::Key_PageUp;
            break;
        case GLFW_KEY_PAGE_DOWN:
            key = KeyboardKey::Key_PageDown;
            break;
        case GLFW_KEY_HOME:
            key = KeyboardKey::Key_Home;
            break;
        case GLFW_KEY_END:
            key = KeyboardKey::Key_End;
            break;
        case GLFW_KEY_CAPS_LOCK:
            key = KeyboardKey::Key_CapsLock;
            break;
        case GLFW_KEY_SCROLL_LOCK:
            key = KeyboardKey::Key_ScrollLock;
            break;
        case GLFW_KEY_NUM_LOCK:
            key = KeyboardKey::Key_NumLock;
            break;
        case GLFW_KEY_PRINT_SCREEN:
            key = KeyboardKey::Key_PrintScreen;
            break;
        case GLFW_KEY_PAUSE:
            key = KeyboardKey::Key_Pause;
            break;
        case GLFW_KEY_F1:
            key = KeyboardKey::Key_F1;
            break;
        case GLFW_KEY_F2:
            key = KeyboardKey::Key_F2;
            break;
        case GLFW_KEY_F3:
            key = KeyboardKey::Key_F3;
            break;
        case GLFW_KEY_F4:
            key = KeyboardKey::Key_F4;
            break;
        case GLFW_KEY_F5:
            key = KeyboardKey::Key_F5;
            break;
        case GLFW_KEY_F6:
            key = KeyboardKey::Key_F6;
            break;
        case GLFW_KEY_F7:
            key = KeyboardKey::Key_F7;
            break;
        case GLFW_KEY_F8:
            key = KeyboardKey::Key_F8;
            break;
        case GLFW_KEY_F9:
            key = KeyboardKey::Key_F9;
            break;
        case GLFW_KEY_F10:
            key = KeyboardKey::Key_F10;
            break;
        case GLFW_KEY_F11:
            key = KeyboardKey::Key_F11;
            break;
        case GLFW_KEY_F12:
            key = KeyboardKey::Key_F12;
            break;
        case GLFW_KEY_LEFT_SHIFT:
            key = KeyboardKey::Key_LeftShift;
            break;
        case GLFW_KEY_LEFT_CONTROL:
            key = KeyboardKey::Key_LeftCtrl;
            break;
        case GLFW_KEY_LEFT_ALT:
            key = KeyboardKey::Key_LeftAlt;
            break;
        case GLFW_KEY_LEFT_SUPER:
            key = KeyboardKey::Key_LeftWin;
            break;
        case GLFW_KEY_RIGHT_SHIFT:
            key = KeyboardKey::Key_RightShift;
            break;
        case GLFW_KEY_RIGHT_CONTROL:
            key = KeyboardKey::Key_RightCtrl;
            break;
        case GLFW_KEY_RIGHT_ALT:
            key = KeyboardKey::Key_RightAlt;
            break;
        case GLFW_KEY_RIGHT_SUPER:
            key = KeyboardKey::Key_RightWin;
            break;
    }

    return key;
}

bool Input::isAnyKeyPressed() const
{
    return std::any_of(std::begin(m_keys_states), std::end(m_keys_states), [](bool k) { return k; });
}

bool Input::isMouseButtonPressed(MouseButton button_id) const
{
    return m_mouse_buttons_state[static_cast<size_t>(button_id)];
}

void Input::buttonEvent(MouseButton button_id, bool press)
{
    if(press)
    {
        m_mouse_buttons_state[static_cast<size_t>(button_id)] = true;
    }
    else
    {
        m_mouse_buttons_state[static_cast<size_t>(button_id)] = false;
    }
}

void Input::mousePos(int32_t xpos, int32_t ypos)
{
    m_mouse_position = glm::ivec2{xpos, ypos};
}

void Input::keyEvent(KeyboardKey key, bool press)
{
    auto key_ind = static_cast<size_t>(key);

    if(press)
    {
        m_last_key             = key;
        m_keys_states[key_ind] = true;
    }
    else
    {
        m_keys_states[key_ind] = false;
    }
}