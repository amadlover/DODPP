#pragma once

#include "error.hpp"
#include <Windows.h>

AGE_RESULT graphics_common_graphics_init (HINSTANCE h_instance, HWND h_wnd);
AGE_RESULT graphics_init (void);
AGE_RESULT graphics_update_command_buffers (void);
AGE_RESULT graphics_create_transforms_buffer (void);
AGE_RESULT graphics_update_transforms_buffer (void);
AGE_RESULT graphics_submit_present (void);
void graphics_exit (void);