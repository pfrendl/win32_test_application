#pragma once

#include <windows.h>
#include <windowsx.h>

#include "datatypes.h"


struct win32_offscreen_buffer {
    BITMAPINFO info;
    void *memory;
    int width;
    int height;
    int stride;
    int bytes_per_pixel;
};


struct win32_window_dimensions {
    int width;
    int height;
};


struct win32_input_data {
    bool W;
    bool A;
    bool S;
    bool D;
    bool left_button_down;
    int mouse_x;
    int mouse_y;
    float zoom = 400;
    Vec2 cam_pos = {0, 0};
};
