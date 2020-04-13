
#include <windows.h>
#include <stdio.h>
#include <UIAutomation.h>
#include <iostream>
#include <comdef.h>
#include <vector>
#include <Wingdi.h>
#include <regex>
#include <Psapi.h>
#include "sympell.h"

#define ENABLE_HOTKEY_ID 1
#define TAB_HOTKEY_ID 2

#ifndef UNICODE
#define UNICODE
#endif

#define WINDOW_HEIGHT 18

void *current;
IUIAutomationElement *currentSender;
HHOOK hHook;
bool isHotkeyEnabled = true;
int index = 0;
HWND hwnd;
int windowPid = 0;
std::vector<HWND> labels;
symspell::SymSpell symSpell;
vector<std::unique_ptr<symspell::SuggestItem>> suggestions;
std::unordered_set<std::string> programBlacklist = {"code.exe"};

void Log(const char *fmt, ...)
{
    return;
    va_list args;
    va_start(args, fmt);
    printf(fmt, args);
    va_end(args);
}

bool GetProcessName(int pid, char *buffer)
{
    HANDLE Handle = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE,
        pid);
    if (Handle)
    {
        if (GetModuleBaseName(Handle, 0, (LPSTR)buffer, MAX_PATH))
        {
            CloseHandle(Handle);
            return true;
        }
        else
        {
            CloseHandle(Handle);
            return false;
        }
    }
}

bool IsProcessBlacklisted(int pid)
{
    char buffer[MAX_PATH];
    if (GetProcessName(pid, buffer))
    {
        for (auto &c : buffer)
        {
            c = tolower(c);
        }
        return programBlacklist.find(buffer) != programBlacklist.end();
    }
    return true;
}

void PredictSymSpell(const char *input)
{
    symSpell.Lookup(input, symspell::Verbosity::Top, 2, false, suggestions);
}

void CreateLabels()
{
    for (auto &lbl : labels)
    {
        DestroyWindow(lbl);
    }
    labels.clear();
    int xOffset = 0;

    for (int i = 0; i < suggestions.size(); i++)
    {
        int x, w, y, h;
        y = 0;
        h = WINDOW_HEIGHT;
        w = 60;
        x = xOffset;
        xOffset += w;

        bool isCurrent = index == i;

        HWND lbl = CreateWindow("static", "",
                                WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                x, y, w, h,
                                hwnd, (HMENU)(501),
                                (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
        labels.push_back(lbl);
    }
}

void SetLabels()
{
    for (int i = 0; i < suggestions.size(); i++)
    {
        auto currentWord = suggestions[i]->term;
        SetWindowText(labels[i], (LPCSTR)currentWord);
    }
}

void PlaceWindow()
{
    RECT boundingBox;
    currentSender->get_CurrentBoundingRectangle(&boundingBox);
    SetWindowPos(hwnd, HWND_TOPMOST, boundingBox.left, boundingBox.top - WINDOW_HEIGHT, 500, WINDOW_HEIGHT, 0);
    ShowWindow(hwnd, SW_SHOW);
}

void GetCurrentTextAndProcess()
{
    void *patternObj;

    if (!currentSender->GetCurrentPatternAs(UIA_ValuePatternId, IID_IUIAutomationValuePattern, &patternObj))
    {
        current = patternObj;
        Log("value pattern\n");
        BSTR bs;
        static_cast<IUIAutomationValuePattern *>(patternObj)->get_CurrentValue(&bs);
        const std::string strval = _bstr_t(bs);

        std::regex re("\\b(\\w+)$");
        std::smatch match;

        if (std::regex_search(strval, match, re))
        {
            suggestions.clear();
            PredictSymSpell(match.str(1).c_str());
            CreateLabels();
            SetLabels();
        }
        currentSender->SetFocus();
    }
    else if (!currentSender->GetCurrentPatternAs(UIA_TextPatternId, IID_IUIAutomationTextPattern, &patternObj))
    {
        current = patternObj;
        Log("text pattern\n");
        BSTR bs;
        IUIAutomationTextRange *range;
        static_cast<IUIAutomationTextPattern *>(patternObj)->get_DocumentRange(&range);
        range->GetText(-1, &bs);
        wchar_t *str = _bstr_t(bs, false);
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

        BOOL isPassword;
        sender->get_CurrentIsPassword(&isPassword);
        int curPid = 0;
        sender->get_CurrentProcessId(&curPid);
        if (ctrlType == UIA_EditControlTypeId && !isPassword && !IsProcessBlacklisted(curPid))
        {

            Log("Found textbox\n");
            GetCurrentTextAndProcess();
            PlaceWindow();
        }
        else if (curPid != windowPid)
        {
            currentSender = NULL;
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
    if (isHotkeyEnabled && currentSender)
    {
        GetCurrentTextAndProcess();
    }
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
            SendText(suggestions[index++]->term);
            if (index == suggestions.size())
            {
                index = 0;
            }
        }
    }
}

void InitializeSymspell()
{
    symSpell.CreateDictionaryEntry("game", 1);
    symSpell.CreateDictionaryEntry("gaming", 1);
    symSpell.CreateDictionaryEntry("gem", 1);
    symSpell.CreateDictionaryEntry("get", 1);
    symSpell.CreateDictionaryEntry("hard", 1);
    symSpell.CreateDictionaryEntry("hello", 1);
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

    InitializeSymspell();
}

void Exit(IUIAutomation *uiAutomationPtr, FocusChangedEventHandler *focusChangedEventHandlerPtr)
{
    UnRegisterEventHandlers(uiAutomationPtr, focusChangedEventHandlerPtr);
    Cleanup(uiAutomationPtr, focusChangedEventHandlerPtr);
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
    CreateLabels();
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
