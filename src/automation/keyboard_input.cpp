#include <automation/keyboard_input.hpp>

void KeyboardInput::sendText(std::wstring text)
{
    std::vector<INPUT> inputs;
    inputs.reserve(text.size() * 2);
    for (auto c : text)
    {
        inputs.push_back(CreateInputFromChar(c, KeyState::Down));
        inputs.push_back(CreateInputFromChar(c, KeyState::Up));
    }

    this->sendKeystrokes(inputs);
}

void KeyboardInput::deleteWord()
{
    std::vector<INPUT> selectWordInput;
    selectWordInput.push_back(CreateInputFromKey(VK_CONTROL, KeyState::Down));
    selectWordInput.push_back(CreateInputFromKey(VK_SHIFT, KeyState::Down));

    selectWordInput.push_back(CreateInputFromKey(VK_LEFT, KeyState::Down));
    selectWordInput.push_back(CreateInputFromKey(VK_LEFT, KeyState::Up));

    selectWordInput.push_back(CreateInputFromKey(VK_CONTROL, KeyState::Up));
    selectWordInput.push_back(CreateInputFromKey(VK_SHIFT, KeyState::Up));
    this->sendKeystrokes(selectWordInput);

    std::vector<INPUT> deleteWordInput;
    deleteWordInput.push_back(CreateInputFromKey(VK_DELETE, KeyState::Down));
    deleteWordInput.push_back(CreateInputFromKey(VK_DELETE, KeyState::Up));
    this->sendKeystrokes(deleteWordInput);
}

bool KeyboardInput::IsKeyExtended(char keyCode)
{
    return (keyCode == VK_MENU ||
            keyCode == VK_LMENU ||
            keyCode == VK_RMENU ||
            keyCode == VK_CONTROL ||
            keyCode == VK_RCONTROL ||
            keyCode == VK_INSERT ||
            keyCode == VK_DELETE ||
            keyCode == VK_HOME ||
            keyCode == VK_END ||
            keyCode == VK_PRIOR ||
            keyCode == VK_NEXT ||
            keyCode == VK_RIGHT ||
            keyCode == VK_UP ||
            keyCode == VK_LEFT ||
            keyCode == VK_DOWN ||
            keyCode == VK_NUMLOCK ||
            keyCode == VK_CANCEL ||
            keyCode == VK_SNAPSHOT ||
            keyCode == VK_DIVIDE);
}

INPUT KeyboardInput::CreateInputFromKey(wchar_t key, KeyState KeyState)
{
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = static_cast<WORD>(key);
    input.ki.wScan = MapVirtualKey(key, 0) & 0xFFU;
    input.ki.dwFlags = KeyState
        ? (this->IsKeyExtended(key) ? KEYEVENTF_KEYUP | KEYEVENTF_EXTENDEDKEY : KEYEVENTF_KEYUP)
        : (this->IsKeyExtended(key) ? KEYEVENTF_EXTENDEDKEY : 0);
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;

    return input;
}

INPUT KeyboardInput::CreateInputFromChar(wchar_t character, KeyState KeyState)
{
    INPUT input;
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = 0;
    input.ki.wScan = static_cast<WORD>(character);
    input.ki.dwFlags = KeyState
        ? KEYEVENTF_KEYUP | KEYEVENTF_UNICODE
        : KEYEVENTF_UNICODE;
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    if ((character & 0xFF00) == 0xE000)
        input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;

    return input;
}

void KeyboardInput::sendKeystrokes(std::vector<INPUT> &inputs)
{
    SendInput(inputs.size(), static_cast<LPINPUT>(inputs.data()), sizeof(INPUT)) == inputs.size();
}