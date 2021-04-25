#pragma once

#include <vector>
#include <string>

#include <Windows.h>

enum KeyState : bool {
    Down = 0,
    Up = 1
};

class KeyboardInput {
private:
    void sendKeystrokes(std::vector<INPUT> &inputs);
    INPUT KeyboardInput::CreateInputFromChar(wchar_t character, KeyState isKeyUpInput);
    INPUT KeyboardInput::CreateInputFromKey(wchar_t key, KeyState isKeyUpInput);
    bool KeyboardInput::IsKeyExtended(char keyCode);
public:
    void sendText(std::wstring text);
    void deleteWord();
};