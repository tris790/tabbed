#pragma once

#include <vector>
#include <string>

class INPUT;

enum KeyState : bool {
    Down = 0,
    Up = 1
};

class KeyboardInput {
private:
    void sendKeystrokes(std::vector<INPUT> &inputs);
    INPUT KeyboardInput::CreateInputFromChar(char character, KeyState isKeyUpInput);
    INPUT KeyboardInput::CreateInputFromKey(char key, KeyState isKeyUpInput);
    bool KeyboardInput::IsKeyExtended(char keyCode);
public:
    void sendText(std::string text);
    void deleteWord();
};