#include <automation/window_manager.hpp>

void WindowManager::move(int x, int y, int w, int h)
{
    SetWindowPos(this->hwnd, HWND_TOPMOST, x, y, w, h, 0);
}

void WindowManager::setVisibility(bool visibile)
{
    ShowWindow(this->hwnd, visibile ? SW_SHOW : SW_HIDE);
}