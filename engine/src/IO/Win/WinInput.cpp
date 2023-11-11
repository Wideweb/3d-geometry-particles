#include "WinInput.hpp"

#include <WinUser.h>
#include <Windef.h>

#include <algorithm>
#include <iostream>

namespace Engine {

static int getSDLScancode(KeyCode code) {
    switch (code) {
        case KeyCode::Backspace:
            return VK_BACK;
        case KeyCode::Space:
            return VK_SPACE;
        case KeyCode::A:
            return 65;
        case KeyCode::D:
            return 68;
        case KeyCode::S:
            return 83;
        case KeyCode::W:
            return 87;
        case KeyCode::F:
            return 70;
        case KeyCode::Q:
            return 81;
        case KeyCode::E:
            return 69;
        case KeyCode::X:
            return 88;
        case KeyCode::Y:
            return 89;
        case KeyCode::Z:
            return 90;
        case KeyCode::Escape:
            return VK_ESCAPE;
        case KeyCode::Shift:
            return VK_SHIFT;
        case KeyCode::LeftControl:
            return VK_LCONTROL;
        default:
            return -1;
    }
}

void WinInput::update() {}

bool WinInput::IsKeyPressed(KeyCode key) { return GetKeyState(getSDLScancode(key)) & 0x8000; }

bool WinInput::IsAnyKeyPressed(const std::vector<KeyCode> keys) { return false; }

bool WinInput::IsMousePressed(MouseButton button) { return GetKeyState(VK_LBUTTON) & 0x8000; }

glm::vec2 WinInput::GetMousePosition() {
    POINT p;
    if (GetCursorPos(&p)) {
        glm::vec2(p.x, p.y);
    }
    return glm::vec2(0.0f, 0.0f);
}

void WinInput::SetTextInput(const std::string& input) { m_TextInput = input; }

std::string WinInput::GetTextInput() { return m_TextInput; }

}  // namespace Engine
