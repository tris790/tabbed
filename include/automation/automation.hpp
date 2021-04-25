#pragma once
#include <automation/focus_changed_listener.hpp>
#include <functional>

class IUIAutomation;

class Automation {
private:
    FocusChangedListener focusListener;
    IUIAutomation *uiAutomation;
    void initializeUIAutomation();
    void uninitializeUIAutomation();
    void registerOnFocusCallback();
    void unregisterOnFocusCallback();

public: 
    Automation(std::function<void(IUIAutomationElement*)> onFocusChangedEventHandler) 
        : focusListener {onFocusChangedEventHandler}
    {
        this->initializeUIAutomation();
        this->registerOnFocusCallback();
    };
    ~Automation() { unregisterOnFocusCallback(); };
    Automation(Automation &automation);
    void getCurrentWidget();
};