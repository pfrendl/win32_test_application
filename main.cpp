#include <stdio.h>
#include <time.h>

#include "win32_definitions.h"
#include "random.h"
#include "graphics.h"
#include "collision_detection.h"
#include "physics.h"

#define internal static
#define local_persist static
#define global_variable static


// TODO(Peti): This is a global for now.
global_variable bool global_running;
global_variable win32_offscreen_buffer global_back_buffer;
global_variable win32_input_data global_input_data;


win32_window_dimensions
win32_get_window_dimensions(HWND window) {
    win32_window_dimensions result;

    RECT client_rect;
    GetClientRect(window, &client_rect);
    result.width = client_rect.right - client_rect.left;
    result.height = client_rect.bottom - client_rect.top;

    return result;
}


internal void
win32_resize_DIB_section(win32_offscreen_buffer *buffer, int width, int height) {
    // TODO(Peti): Bulletproof this.
    // Maybe don't free first, free after, then free first if that fails.

    if (buffer->memory) {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width = width;
    buffer->height = height;
    buffer->bytes_per_pixel = 4;

    // NOTE(Peti): When biHeight is negative, bitmap is treated as top-down, not bottom-up,
    // meaning that the first pixel in the bitmap is the top-left pixel
    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = -buffer->height;
    buffer->stride = buffer->width * buffer->bytes_per_pixel;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 8 * buffer->bytes_per_pixel;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    int bitmap_memory_size = buffer->width * buffer->height * buffer->bytes_per_pixel;
    buffer->memory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);

    // TODO(Peti): Probably clear this to black.
}


internal void
win32_display_buffer_in_window(win32_offscreen_buffer *buffer, HDC device_context,
                               int window_width, int window_height) {
    // TODO(Peti): Aspect ratio correction
    StretchDIBits(device_context,
                  /*
                  x, y, width, height,
                  x, y, width, height,
                  */
                  0, 0, window_width, window_height,
                  0, 0, buffer->width, buffer->height,
                  buffer->memory,
                  &buffer->info,
                  DIB_RGB_COLORS, SRCCOPY);
}


LRESULT CALLBACK
win32_main_window_callback(
    HWND   window,
    UINT   message,
    WPARAM w_param,
    LPARAM l_param
) {
    LRESULT result = 0;

    switch(message) {
        case WM_CLOSE: {
            // TODO(Peti): Handle this with a message to the user?
            global_running = false;
        } break;

        case WM_ACTIVATEAPP: {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        case WM_DESTROY: {
            // TODO(Peti): handle this as an error - recreate window?
            global_running = false;
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP: {
            uint32_t vk_code = w_param;
            bool was_down = (l_param & (1 << 30)) != 0;
            bool is_down = (l_param & (1 << 31)) == 0;
            if(is_down != was_down) {
                switch(vk_code) {
                    case 'W': {global_input_data.W = is_down;} break;
                    case 'A': {global_input_data.A = is_down;} break;
                    case 'S': {global_input_data.S = is_down;} break;
                    case 'D': {global_input_data.D = is_down;} break;
                }
            }
        } break;
        
        case WM_MOUSEWHEEL: {
            int travel_distance = GET_WHEEL_DELTA_WPARAM(w_param) / 120;
            global_input_data.zoom *= (1 + 0.05 * travel_distance);
        } break;
        
        case WM_LBUTTONDOWN: {
            global_input_data.left_button_down = true;
            global_input_data.mouse_x = GET_X_LPARAM(l_param);
            global_input_data.mouse_y = GET_Y_LPARAM(l_param);
        } break;

        case WM_LBUTTONUP: {
            global_input_data.left_button_down = false;
        } break;
        
        case WM_MOUSEMOVE: {
            if(global_input_data.left_button_down) {
                int x = GET_X_LPARAM(l_param);
                int y = GET_Y_LPARAM(l_param);
                int dx = x - global_input_data.mouse_x;
                int dy = y - global_input_data.mouse_y;
                global_input_data.mouse_x = x;
                global_input_data.mouse_y = y;
                global_input_data.cam_pos.v[0] -= dx / global_input_data.zoom;
                global_input_data.cam_pos.v[1] -= dy / global_input_data.zoom;
            }
        } break;

        case WM_PAINT: {
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(window, &paint);
            int x = paint.rcPaint.left;
            int y = paint.rcPaint.top;
            int width = paint.rcPaint.right - paint.rcPaint.left;
            int height = paint.rcPaint.bottom - paint.rcPaint.top;

            win32_window_dimensions dimensions = win32_get_window_dimensions(window);
            win32_display_buffer_in_window(&global_back_buffer, device_context,
                                           dimensions.width, dimensions.height);
            EndPaint(window, &paint);
        } break;
        
        case WM_EXITSIZEMOVE: {
            global_input_data.last_tick = clock();
        } break;

        default: {
            // OutputDebugStringA("default\n");
            result = DefWindowProc(window, message, w_param, l_param);
        } break;
    }

    return result;
}


int CALLBACK
WinMain(
    HINSTANCE instance,
    HINSTANCE prev_instance,
    LPSTR command_line,
    int show_code
) {
    WNDCLASSA window_class = {};

    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = win32_main_window_callback;
    window_class.hInstance = instance;
    // window_class.hIcon = ;
    window_class.hCursor = LoadCursorA(NULL, IDC_ARROW);
    window_class.lpszClassName = "RaytracerWindowClass";
    
    Memory memory = m_create(1024 * 1024 * 50);
    
    char buff[128];
    
    time_t t;
    srand((unsigned)time(&t));
    
    double firmness = 10.0;
    double map_radius = 1.0;
    double velocity_dampening = 0.75;
    constexpr int circle_count = 1000;
    Vec2 positions[circle_count];
    Vec2 velocities[circle_count];
    double radii[circle_count];
    double weights[circle_count];
    for(int i = 0; i < circle_count; ++i) {
        positions[i] = random_normal({0, 0}, 0.2);
        velocities[i] = {0, 0};
        radii[i] = random_uniform(0.003, 0.01);
        weights[i] = radii[i] * radii[i] * M_PI;
    }
    int x_bound_idxs[circle_count];
    int y_bound_idxs[circle_count];
    for(int i = 0; i < circle_count; ++i) {
        x_bound_idxs[i] = i;
        y_bound_idxs[i] = i;
    }

    if(RegisterClass(&window_class)) {
        HWND window =
            CreateWindowExA(
                0,
                window_class.lpszClassName,
                "Raytracing",
                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                1440,
                900,
                0,
                0,
                instance,
                0);
        if(window) {
            win32_window_dimensions dimensions = win32_get_window_dimensions(window);
            win32_resize_DIB_section(&global_back_buffer, dimensions.width, dimensions.height);
            
            global_input_data.last_tick = clock();
            
            global_running = true;
            while (global_running)
            {
                MSG message;
                while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
                    if (message.message == WM_QUIT) {
                        global_running = false;
                    }
                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                }
                
                clock_t current_tick = clock();
                double delta_t = ((double) (current_tick - global_input_data.last_tick)) / CLOCKS_PER_SEC;
                global_input_data.last_tick = current_tick;

                BoundingBox *bboxes = (BoundingBox *)m_alloc(&memory, sizeof(BoundingBox) * circle_count);
                for(int i = 0; i < circle_count; ++i) {
                    Vec2 *origin = positions + i;
                    double radius = radii[i];
                    bboxes[i] = {{origin->v[0] - radius, origin->v[1] - radius}, {origin->v[0] + radius, origin->v[1] + radius}};
                }
                
                IndexPairArray collisions = sweep_and_prune(
                    bboxes,
                    x_bound_idxs,
                    y_bound_idxs,
                    circle_count,
                    &memory
                );
                
                narrow_phase(positions, radii, &collisions);
                
                Vec2 *forces = calculate_forces(positions, radii, circle_count, &collisions, firmness, map_radius, &memory);
                
                apply_forces(positions, velocities, weights, forces, circle_count, velocity_dampening, delta_t);
                
                render_scene(&global_back_buffer, &(global_input_data.cam_pos),
                                    global_input_data.zoom, positions, radii, circle_count, &collisions, &memory);
                
                sprintf(buff, "MB mem used: %f\n", (double)memory.ptr / 1024 / 1024);
                OutputDebugStringA(buff);

                m_free(&memory);

                HDC device_context = GetDC(window);
                win32_window_dimensions dimensions = win32_get_window_dimensions(window);
                win32_display_buffer_in_window(&global_back_buffer, device_context,
                                                                dimensions.width, dimensions.height);
                ReleaseDC(window, device_context);
            }
        }
        else {
            // TODO(Peti): Logging
        }
    }
    else {
        // TODO(Peti): Logging
    }

    return 0;
}
