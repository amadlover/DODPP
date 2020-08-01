#pragma once

#include "error.hpp"
#include <Windows.h>

AGE_RESULT game_init (HINSTANCE h_instance, HWND h_wnd);
AGE_RESULT game_process_left_mouse_click (const size_t x, const size_t y);
AGE_RESULT game_update (void);
AGE_RESULT game_submit_present (void);
void game_exit (void);