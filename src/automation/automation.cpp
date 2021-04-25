#include <automation/automation.hpp>
#include <UIAutomation.h>

void Automation::getCurrentWidget()
{
}

void Automation::initializeUIAutomation()
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    HRESULT hr = CoCreateInstance(
        __uuidof(CUIAutomation),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(IUIAutomation),
        reinterpret_cast<void **>(&this->uiAutomation));
}

void Automation::uninitializeUIAutomation()
{
    this->focusListener.Release();

    if (this->uiAutomation != nullptr)
        this->uiAutomation->Release();

    CoUninitialize();
}

void Automation::registerOnFocusCallback()
{
    this->uiAutomation->AddFocusChangedEventHandler(NULL, reinterpret_cast<IUIAutomationFocusChangedEventHandler *>(&this->focusListener));
}

void Automation::unregisterOnFocusCallback()
{
    this->uiAutomation->RemoveFocusChangedEventHandler(reinterpret_cast<IUIAutomationFocusChangedEventHandler *>(&this->focusListener));
}