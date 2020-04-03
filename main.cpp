#include <windows.h>
#include <stdio.h>
#include <UIAutomation.h>
#include <iostream>
#include <comdef.h>
#include <vector>

void *current;
IUIAutomationElement *currentSender;

class FocusChangedEventHandler : public IUIAutomationFocusChangedEventHandler
{
private:
    LONG _refCount;

public:
    FocusChangedEventHandler() : _refCount(1) {}

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return InterlockedIncrement(&_refCount);
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG ret = InterlockedDecrement(&_refCount);
        if (ret == 0)
        {
            delete this;
            return 0;
        }
        return ret;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppInterface)
    {
        if (riid == __uuidof(IUnknown))
            *ppInterface = static_cast<IUIAutomationFocusChangedEventHandler *>(this);
        else if (riid == __uuidof(IUIAutomationFocusChangedEventHandler))
            *ppInterface = static_cast<IUIAutomationFocusChangedEventHandler *>(this);
        else
        {
            *ppInterface = NULL;
            return E_NOINTERFACE;
        }
        this->AddRef();
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE HandleFocusChangedEvent(IUIAutomationElement *sender)
    {
        CONTROLTYPEID ctrlType;
        sender->get_CurrentControlType(&ctrlType);

        RECT boundingBox;
        sender->get_CurrentBoundingRectangle(&boundingBox);

        BOOL isPassword;
        sender->get_CurrentIsPassword(&isPassword);

        if (ctrlType == UIA_EditControlTypeId && !isPassword)
        {
            printf("reg %p\n", sender);
            currentSender = sender;

            void *patternObj;
            printf("Found textbox\n");

            if (!sender->GetCurrentPatternAs(UIA_ValuePatternId, IID_IUIAutomationValuePattern, &patternObj))
            {
                current = patternObj;
                printf("value pattern\n");
                BSTR bs;
                static_cast<IUIAutomationValuePattern *>(patternObj)->get_CurrentValue(&bs);
                wchar_t *str = _bstr_t(bs, false);
                printf(">> You are focused in a textbox (%S) [%d, %d]\n", str, boundingBox.left, boundingBox.top);
            }
            else if (!sender->GetCurrentPatternAs(UIA_TextPatternId, IID_IUIAutomationTextPattern, &patternObj))
            {
                current = patternObj;
                printf("text pattern\n");
                BSTR bs;
                IUIAutomationTextRange *range;
                static_cast<IUIAutomationTextPattern *>(patternObj)->get_DocumentRange(&range);
                range->GetText(-1, &bs);
                wchar_t *str = _bstr_t(bs, false);
                printf(">> You are focused in a textbox (%S) [%d, %d]\n", str, boundingBox.left, boundingBox.top);
            }
        }
        return S_OK;
    }
};

HRESULT AddFocusHandler(IUIAutomation *uiAutomationPtr)
{
    FocusChangedEventHandler *pFocusHandler = new FocusChangedEventHandler();
    if (!pFocusHandler)
    {
        return E_OUTOFMEMORY;
    }
    IUIAutomationFocusChangedEventHandler *pHandler;
    pFocusHandler->QueryInterface(IID_IUIAutomationFocusChangedEventHandler, (void **)&pHandler);
    HRESULT hr = uiAutomationPtr->AddFocusChangedEventHandler(NULL, pHandler);
    pFocusHandler->Release();
    return hr;
}

HRESULT RegisterEventHandlers(IUIAutomation *uiAutomationPtr, FocusChangedEventHandler *focusChangedEventHandlerPtr)
{
    printf("Adding Event Handlers.\n");
    return uiAutomationPtr->AddFocusChangedEventHandler(NULL, (IUIAutomationFocusChangedEventHandler *)focusChangedEventHandlerPtr);
}

HRESULT UnRegisterEventHandlers(IUIAutomation *uiAutomationPtr, FocusChangedEventHandler *focusChangedEventHandlerPtr)
{
    printf("Removing Event Handlers.\n");
    return uiAutomationPtr->RemoveFocusChangedEventHandler((IUIAutomationFocusChangedEventHandler *)focusChangedEventHandlerPtr);
}

void Cleanup(IUIAutomation *uiAutomationPtr, FocusChangedEventHandler *focusChangedEventHandlerPtr)
{
    if (focusChangedEventHandlerPtr != NULL)
        focusChangedEventHandlerPtr->Release();

    if (uiAutomationPtr != NULL)
        uiAutomationPtr->Release();

    CoUninitialize();
}

bool IsKeyExtended(char keyCode)
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

INPUT CreateDownInputFromKey(char key)
{
    INPUT down;
    down.type = INPUT_KEYBOARD;
    down.ki.wVk = key;
    down.ki.wScan = MapVirtualKey(key, 0) & 0xFFU;
    down.ki.dwFlags = IsKeyExtended(key) ? KEYEVENTF_EXTENDEDKEY : 0;
    down.ki.time = 0;
    down.ki.dwExtraInfo = 0;
    return down;
}

INPUT CreateUpInputFromKey(char key)
{
    INPUT up;
    up.type = INPUT_KEYBOARD;
    up.ki.wVk = key;
    up.ki.wScan = MapVirtualKey(key, 0) & 0xFFU;
    up.ki.dwFlags = IsKeyExtended(key) ? KEYEVENTF_KEYUP | KEYEVENTF_EXTENDEDKEY : KEYEVENTF_KEYUP;
    up.ki.time = 0;
    up.ki.dwExtraInfo = 0;
    return up;
}

INPUT CreateDownInputFromChar(char character)
{
    INPUT down;
    down.type = INPUT_KEYBOARD;
    down.ki.wVk = 0;
    down.ki.wScan = character;
    down.ki.dwFlags = KEYEVENTF_UNICODE;
    down.ki.time = 0;
    down.ki.dwExtraInfo = 0;
    if ((character & 0xFF00) == 0xE000)
    {
        down.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    }
    return down;
}

INPUT CreateUpInputFromChar(char character)
{
    INPUT up;
    up.type = INPUT_KEYBOARD;
    up.ki.wVk = 0;
    up.ki.wScan = character;
    up.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_UNICODE;
    up.ki.time = 0;
    up.ki.dwExtraInfo = 0;
    if ((character & 0xFF00) == 0xE000)
    {
        up.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    }
    return up;
}

void SendKeystrokes(std::vector<INPUT> inputs)
{
    SendInput(inputs.size(), (LPINPUT)inputs.data(), sizeof(INPUT));
}

void DeleteWord()
{
    std::vector<INPUT> inputs;
    inputs.push_back(CreateDownInputFromKey(VK_CONTROL));

    inputs.push_back(CreateDownInputFromKey(VK_SHIFT));

    inputs.push_back(CreateDownInputFromKey(VK_LEFT));
    inputs.push_back(CreateUpInputFromKey(VK_LEFT));

    inputs.push_back(CreateDownInputFromKey(VK_DELETE));
    inputs.push_back(CreateUpInputFromKey(VK_DELETE));

    inputs.push_back(CreateUpInputFromKey(VK_CONTROL));
    inputs.push_back(CreateUpInputFromKey(VK_SHIFT));

    SendKeystrokes(inputs);
}

void SendText(char *text)
{
    int len = strlen(text);
    std::vector<INPUT> inputs;
    for (int i = 0; i < len; i++)
    {
        inputs.push_back(CreateDownInputFromChar(text[i]));
        inputs.push_back(CreateUpInputFromChar(text[i]));
    }
    SendKeystrokes(inputs);
}

void SetTextBoxValue(LPCTSTR str)
{
    BSTR string;
    static_cast<IUIAutomationValuePattern *>(current)->get_CurrentValue(&string);
    BSTR newString = SysAllocString((_bstr_t)string + (_bstr_t)str);
    static_cast<IUIAutomationValuePattern *>(current)->SetValue(newString);
    SysFreeString(string);
}

int main()
{
    RegisterHotKey(
        NULL,
        1,
        0x4000,
        VK_TAB);

    HRESULT hr;
    FocusChangedEventHandler *focusChangedEventHandlerPtr = NULL;

    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    IUIAutomation *uiAutomationPtr = NULL;
    hr = CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation), (void **)&uiAutomationPtr);
    if (FAILED(hr) || uiAutomationPtr == NULL)
    {
        Cleanup(uiAutomationPtr, focusChangedEventHandlerPtr);
        return 1;
    }

    focusChangedEventHandlerPtr = new FocusChangedEventHandler();
    if (focusChangedEventHandlerPtr == NULL)
    {
        Cleanup(uiAutomationPtr, focusChangedEventHandlerPtr);
        return 1;
    }

    if (FAILED(RegisterEventHandlers(uiAutomationPtr, focusChangedEventHandlerPtr)))
    {
        Cleanup(uiAutomationPtr, focusChangedEventHandlerPtr);
        return 1;
    }

    printf("Press any key to remove event handler and exit\n");
    MSG msg = {0};
    std::vector<char *> words = {"hacked", "hello", "ninja"};
    int index = 0;
    while (GetMessage(&msg, NULL, 0, 0) != 0)
    {
        if (msg.message == WM_HOTKEY)
        {
            DeleteWord();
            SendText(words[index++]);
            if (index == words.size())
                index = 0;
        }
    }
    getchar();

    UnRegisterEventHandlers(uiAutomationPtr, focusChangedEventHandlerPtr);

    Cleanup(uiAutomationPtr, focusChangedEventHandlerPtr);
}