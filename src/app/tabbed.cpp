#include <wchar.h>
#include <cstdio>

#include <corrector/corrector.hpp>
#include <automation/automation.hpp>
#include <automation/ui_element_manager.hpp>
#include <string>

#include <Windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,                           // Optional window styles.
        CLASS_NAME,                  // Window class
        L"Learn to Program Windows", // Window text
        WS_OVERLAPPEDWINDOW,         // Window style
        CW_USEDEFAULT,               // x
        CW_USEDEFAULT,               // y
        CW_USEDEFAULT,               // width
        CW_USEDEFAULT,               // height
        NULL,                        // Parent window
        NULL,                        // Menu
        hInstance,                   // Instance handle
        NULL                         // Additional application data
    );

    if (hwnd == NULL)
        return 0;

    ShowWindow(hwnd, nCmdShow);
    auto corrector = Corrector{};
    auto text = L"My string";
    auto output = corrector.autocomplete(text);
    OutputDebugString(output.c_str());
    auto uiElementManager = UIElementManager{};

    auto test = [&](IUIAutomationElement *e) {
        uiElementManager.setCurrentText(L"test");
        OutputDebugString(L"Focus changed\n");
    };

    auto automator = Automation{test};

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

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

        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hwnd, &ps);
    }
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}