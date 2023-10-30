#include "WinInput.hpp"

#include <algorithm>
#include <iostream>

namespace Engine {

void WinInput::update() {}

bool WinInput::IsKeyPressed(KeyCode key) {
    return false;
}

bool WinInput::IsAnyKeyPressed(const std::vector<KeyCode> keys) {
    return false;
}

bool WinInput::IsMousePressed(MouseButton button) {
    return false;
}

glm::vec2 WinInput::GetMousePosition() {
    return glm::vec2(0.0f, 0.0f);
}

void WinInput::SetTextInput(const std::string &input) { m_TextInput = input; }

std::string WinInput::GetTextInput() { return m_TextInput; }

} // namespace Engine
