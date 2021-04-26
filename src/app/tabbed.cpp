#include <wchar.h>
#include <cstdio>

#include <corrector/corrector.hpp>
#include <automation/automation.hpp>
#include <automation/ui_element_manager.hpp>
#include <gui/gui.hpp>
#include <string>

#include <Windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    if (SUCCEEDED(CoInitialize(NULL)))
    {
        {
            Gui app;
            if (SUCCEEDED(app.initialize()))
            {
                app.runMessageLoop();
            }
        }
        CoUninitialize();
    }
}