#pragma once
#include <Windows.h>

class WindowManager {
    private:
        HWND hwnd;
public:
    WindowManager(HWND hwnd) : hwnd {hwnd} {}
    void move(int x, int y, int w, int h);
    void setVisibility(bool visibile);
};