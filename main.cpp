
#include <windows.h>
#include <stdio.h>
#include <UIAutomation.h>
#include <iostream>
#include <comdef.h>
#include <vector>
#include <Wingdi.h>

#define ENABLE_HOTKEY_ID 1
#define TAB_HOTKEY_ID 2

#ifndef UNICODE
#define UNICODE
#endif

void *current;
IUIAutomationElement *currentSender;
HHOOK hHook;
bool isHotkeyEnabled = true;
std::vector<std::string> suggestions = {"hacked", "hello", "ninja", "bob"};
int index = 0;
HWND hwnd;
int windowPid = 0;
std::vector<HWND> labels;
#define WINDOW_HEIGHT 18

void Log(const char *fmt, ...)
{
    return;
    va_list args;
    va_start(args, fmt);
    printf(fmt, args);
    va_end(args);
}

void CreateGui()
{
    for (auto &lbl : labels)
    {
        DestroyWindow(lbl);
    }
    labels.clear();
    int xOffset = 0;
    for (int i = 0; i < suggestions.size(); i++)
    {
        auto currentWord = &suggestions[i];
        int x, w, y, h;
        y = 0;
        h = WINDOW_HEIGHT;
        w = 10 * currentWord->size();
        x = xOffset;
        xOffset += w;

        bool isCurrent = index == i;

        HWND lbl = CreateWindow("static", "ST_U",
                                WS_CHILD | WS_VISIBLE | WS_TABSTOP | (isCurrent ? SS_OWNERDRAW : 0),
                                x, y, w, h,
                                hwnd, (HMENU)(501),
                                (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
        SetWindowText(lbl, (LPCSTR)currentWord->c_str());
        auto color = isCurrent ? RGB(209, 209, 209) : RGB(234, 234, 234);
        labels.push_back(lbl);
    }
}

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
        int curPid = 0;
        sender->get_CurrentProcessId(&curPid);
        if (ctrlType == UIA_EditControlTypeId && !isPassword)
        {
            currentSender = sender;

            void *patternObj;
            Log("Found textbox\n");

            if (!sender->GetCurrentPatternAs(UIA_ValuePatternId, IID_IUIAutomationValuePattern, &patternObj))
            {
                current = patternObj;
                Log("value pattern\n");
                BSTR bs;
                static_cast<IUIAutomationValuePattern *>(patternObj)->get_CurrentValue(&bs);
                wchar_t *str = _bstr_t(bs, false);
                Log(">> You are focused in a textbox (%S) [%ld, %ld]\n", str, boundingBox.left, boundingBox.top);

                // MoveWindow(
                //     hwnd,
                //     boundingBox.left,
                //     boundingBox.top - 50,
                //     500,
                //     50,
                //     true);
                // ShowWindow(hwnd, SW_SHOW);
                // SetWindowLongPtr(hwnd, GWL_STYLE, WS_SYSMENU); //3d argument=style
                SetWindowPos(hwnd, HWND_TOPMOST, boundingBox.left, boundingBox.top - WINDOW_HEIGHT, 500, WINDOW_HEIGHT, SWP_SHOWWINDOW);
            }
            else if (!sender->GetCurrentPatternAs(UIA_TextPatternId, IID_IUIAutomationTextPattern, &patternObj))
            {
                current = patternObj;
                Log("text pattern\n");
                BSTR bs;
                IUIAutomationTextRange *range;
                static_cast<IUIAutomationTextPattern *>(patternObj)->get_DocumentRange(&range);
                range->GetText(-1, &bs);
                wchar_t *str = _bstr_t(bs, false);
                Log(">> You are focused in a textbox (%S) [%ld, %ld]\n", str, boundingBox.left, boundingBox.top);
            }
        }
        else if (curPid != windowPid)
        {
            ShowWindow(hwnd, SW_HIDE);
        }
        return S_OK;
    }
};

HRESULT RegisterEventHandlers(IUIAutomation *uiAutomationPtr, FocusChangedEventHandler *focusChangedEventHandlerPtr)
{
    Log("Adding Event Handlers.\n");
    return uiAutomationPtr->AddFocusChangedEventHandler(NULL, (IUIAutomationFocusChangedEventHandler *)focusChangedEventHandlerPtr);
}

HRESULT UnRegisterEventHandlers(IUIAutomation *uiAutomationPtr, FocusChangedEventHandler *focusChangedEventHandlerPtr)
{
    Log("Removing Event Handlers.\n");
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
    down.ki.wVk = (WORD)key;
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
    up.ki.wVk = (WORD)key;
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
    down.ki.wScan = (WORD)character;
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
    up.ki.wScan = (WORD)character;
    up.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_UNICODE;
    up.ki.time = 0;
    up.ki.dwExtraInfo = 0;
    if ((character & 0xFF00) == 0xE000)
    {
        up.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    }
    return up;
}

bool SendKeystrokes(std::vector<INPUT> inputs)
{
    return SendInput(inputs.size(), (LPINPUT)inputs.data(), sizeof(INPUT)) == inputs.size();
}

void DeleteWord()
{
    std::vector<INPUT> selectWordInput;
    selectWordInput.push_back(CreateDownInputFromKey(VK_CONTROL));
    selectWordInput.push_back(CreateDownInputFromKey(VK_SHIFT));

    selectWordInput.push_back(CreateDownInputFromKey(VK_LEFT));
    selectWordInput.push_back(CreateUpInputFromKey(VK_LEFT));

    selectWordInput.push_back(CreateUpInputFromKey(VK_CONTROL));
    selectWordInput.push_back(CreateUpInputFromKey(VK_SHIFT));
    SendKeystrokes(selectWordInput);

    std::vector<INPUT> deleteWordInput;
    deleteWordInput.push_back(CreateDownInputFromKey(VK_DELETE));
    deleteWordInput.push_back(CreateUpInputFromKey(VK_DELETE));
    SendKeystrokes(deleteWordInput);
}

void SendText(std::string text)
{
    std::vector<INPUT> inputs;
    for (int i = 0; i < text.size(); i++)
    {
        inputs.push_back(CreateDownInputFromChar(text[i]));
        inputs.push_back(CreateUpInputFromChar(text[i]));
    }
    SendKeystrokes(inputs);
}

void SetTextBoxValue(LPCTSTR str)
{
    BSTR string;
    if (FAILED(static_cast<IUIAutomationValuePattern *>(current)->get_CurrentValue(&string)))
    {
        Log("Error while getting the texbox value pattern\n");
        return;
    }
    BSTR newString = SysAllocString((_bstr_t)string + (_bstr_t)str);
    if (FAILED(static_cast<IUIAutomationValuePattern *>(current)->SetValue(newString)))
    {
        Log("Error while setting the textbox value");
    }
    SysFreeString(string);
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;
    if (p->vkCode == VK_SPACE)
    {
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void MessagePump(MSG *msg)
{
    if (msg->message == WM_HOTKEY)
    {
        if (msg->wParam == ENABLE_HOTKEY_ID)
        {
            isHotkeyEnabled = !isHotkeyEnabled;
            if (isHotkeyEnabled)
            {
                RegisterHotKey(NULL, TAB_HOTKEY_ID, MOD_NOREPEAT, VK_TAB);
            }
            else
            {
                UnregisterHotKey(NULL, TAB_HOTKEY_ID);
            }

            Log("Tabed is %s\n", isHotkeyEnabled ? "online" : "offline");
        }
        else if (isHotkeyEnabled && msg->wParam == TAB_HOTKEY_ID)
        {

            DeleteWord();
            SendText(suggestions[index++]);
            if (index == suggestions.size())
            {
                index = 0;
            }
            CreateGui();
        }
    }
}

int Init(IUIAutomation *uiAutomationPtr, FocusChangedEventHandler *focusChangedEventHandlerPtr)
{
    RegisterHotKey(NULL, ENABLE_HOTKEY_ID, MOD_CONTROL | MOD_NOREPEAT, VK_F7);
    RegisterHotKey(NULL, TAB_HOTKEY_ID, MOD_NOREPEAT, VK_TAB);
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);

    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    HRESULT hr = CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation), (void **)&uiAutomationPtr);
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

    Log("Press any key to remove event handler and exit\n");
    MSG msg = {0};
}

void Exit(IUIAutomation *uiAutomationPtr, FocusChangedEventHandler *focusChangedEventHandlerPtr)
{
    UnRegisterEventHandlers(uiAutomationPtr, focusChangedEventHandlerPtr);
    Cleanup(uiAutomationPtr, focusChangedEventHandlerPtr);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{

    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "Test";

    RegisterClass(&wc);

    // Create the window.
    hwnd = CreateWindowEx(
        0,      // Optional window styles.
        "Test", // Window class
        "",     // Window text
        0,      // Window style

        // Size and position
        0,
        0,
        500,
        WINDOW_HEIGHT,

        NULL,      // Parent window
        NULL,      // Menu
        hInstance, // Instance handle
        NULL       // Additional application data
    );
    SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~(WS_BORDER | WS_DLGFRAME | WS_THICKFRAME));
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_DLGMODALFRAME);
    windowPid = GetCurrentProcessId();

    if (hwnd == NULL)
    {
        return 0;
    }
    IUIAutomation *uiAutomationPtr = NULL;
    FocusChangedEventHandler *focusChangedEventHandlerPtr = NULL;
    Init(uiAutomationPtr, focusChangedEventHandlerPtr);
    CreateGui();
    ShowWindow(hwnd, SW_SHOW);

    // Run the message loop.
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        MessagePump(&msg);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    Exit(uiAutomationPtr, focusChangedEventHandlerPtr);
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // All painting occurs here, between BeginPaint and EndPaint.
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(hwnd, &ps);
    }
    case WM_CTLCOLORSTATIC:
    {
        // if (true || GetWindowLong((HWND)lParam, GWL_ID) == 0)
        // {
        //     HDC hDC = (HDC)wParam;

        //     SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
        //     SetTextColor(hDC, RGB(0, 0xFF, 0));
        //     SetBkMode(hDC, TRANSPARENT);

        //     return (INT_PTR)CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
        // }
    }

        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}