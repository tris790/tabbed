#pragma once
#include <string>
#include <automation/keyboard_input.hpp>

class IUIAutomationElement;

class UIElementManager {
private:
    IUIAutomationElement* currentUIElement;
    KeyboardInput keyboard;
    bool isTextbox;
public:
    void setCurrentUIElement(IUIAutomationElement* newUIElement);
    std::wstring getCurrentText();
    void setCurrentText(std::wstring str);
};