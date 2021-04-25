#pragma once
#include <string>

class IUIAutomationElement;

class UIElementManager {
private:
    IUIAutomationElement* currentUIElement;
    bool isTextbox;
public:
    void setCurrentUIElement(IUIAutomationElement* newUIElement);
    std::wstring getCurrentText();
    void setCurrentText(std::wstring str);
};