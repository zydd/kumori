#ifndef DESKTOP_WINDOWS_H
#define DESKTOP_WINDOWS_H

#include <Windows.h>
#include <dwmapi.h>

void play_pause();
void exclude_from_peek(HWND handle);
void ignore_show_desktop(HWND handle);
HWND get_wallpaper_window();

#endif // DESKTOP_WINDOWS_H
