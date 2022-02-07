#include "win.h"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <glad/wgl.h>
#include <glad/gl.h>
#include "nmutil/gl.h"

#define WND_CLASS_NAME "window_class"

static HDC g_hdc = NULL;             // global device context
static HGLRC g_hrc = NULL;           // global rendering context
static HINSTANCE g_instance = NULL; // application instance
static HWND g_hwnd = NULL;           // handle of our window
static bool g_isActive = true;     // false if window is minimized
static bool g_should_stop = false;
/** Current window size. */
static uint32_t size_x;
static uint32_t size_y;
/** Mouse position. */
static int32_t mouse_x;
static int32_t mouse_y;
/** Keyboard state. */
static bool shift_down = false;
/** Events of the last frame. */
static messages g_messages{};

extern LRESULT ImGui_ImplWin32_WndProcHandler(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/// Windows Procedure Event Handler.
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam)) {
        return true;
    }

    switch (message) {
        case WM_ACTIVATE: {
            // window is being activated or deactivated
            if (!HIWORD(wParam)) {
                // program_id was restored or maximized
                g_isActive = true;
            } else {
                // program_id was minimized
                g_isActive = false;
            }
            return 0;
        }
        case WM_SYSCOMMAND: {
            // user chooses command from the Window menu or maximize, minimize,
            // restore, or close buttons

            switch (wParam) {
                case SC_SCREENSAVE:   // screensaver trying to start
                case SC_MONITORPOWER: // monitor going to powersave mode
                    // returning 0 prevents either from happening
                    return 0;
                default:
                    break;
            }
            break;
        }
        case WM_CLOSE: {
            // window is being closed

            // send WM_QUIT to message queue
            PostQuitMessage(0);

            return 0;
        }
        case WM_SIZE: {
            // size has changed

            // update perspective with new width and height
            size_x = GET_X_LPARAM(lParam);
            size_y = GET_Y_LPARAM(lParam);

            nmutil::log(
                    nmutil::LOG_TRACE, "size change %u %u\n", size_x, size_y);

            return 0;
        }
        case WM_CHAR:
            // keydown event with a char

            switch (toupper(wParam)) {
                case VK_ESCAPE: {
                    // send WM_QUIT to message queue
                    PostQuitMessage(0);
                    return 0;
                }
                default:
                    break;
            }
            break;
        case WM_KEYDOWN:
            switch (wParam) {
                case VK_F1:
                    g_messages.f1_down = true;
                    return 0;
                case VK_F2:
                    g_messages.f2_down = true;
                    return 0;
                case VK_F3:
                    g_messages.f3_down = true;
                    return 0;
                case VK_F4:
                    g_messages.f4_down = true;
                    return 0;
                case VK_RETURN:
                    g_messages.enter_down = true;
                    return 0;
                case VK_SHIFT:
                    shift_down = true;
                    return 0;
                default:
                    break;
            }
            break;
        case WM_KEYUP:
            switch (wParam) {
                case VK_SHIFT:
                    shift_down = false;
                    return 0;
                case VK_SNAPSHOT:
                    g_messages.prtsc_up = true;
                    return 0;
                default:
                    break;
            }
            break;
        case WM_LBUTTONDOWN:
            g_messages.lmb_down = true;
            nmutil::log(nmutil::LOG_TRACE, "LMB down\n");
            return 0;
        case WM_MBUTTONDOWN:
            g_messages.mmb_down = true;
            nmutil::log(nmutil::LOG_TRACE, "MMB down\n");
            return 0;
        case WM_RBUTTONDOWN:
            g_messages.rmb_down = true;
            nmutil::log(nmutil::LOG_TRACE, "RMB down\n");
            return 0;
        case WM_LBUTTONUP:
            g_messages.lmb_up = true;
            nmutil::log(nmutil::LOG_TRACE, "LMB up\n");
            return 0;
        case WM_MBUTTONUP:
            g_messages.mmb_up = true;
            nmutil::log(nmutil::LOG_TRACE, "MMB up\n");
            return 0;
        case WM_RBUTTONUP:
            g_messages.rmb_up = true;
            nmutil::log(nmutil::LOG_TRACE, "RMB up\n");
            return 0;
        case WM_MOUSEMOVE: {
            mouse_x = GET_X_LPARAM(lParam);
            mouse_y = GET_Y_LPARAM(lParam);
            return 0;
        }
        case WM_MOUSEWHEEL: {
            g_messages.wheel_delta = GET_WHEEL_DELTA_WPARAM(wParam);
            return 0;
        }
        default:
            break;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

/// Loads WGL extensions. Global instance handle should be set.
static nm_ret load_wgl_extensions()
{
    BOOL ret;
    int32_t reti;

    // register window class class
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = g_instance;
    wc.lpszClassName = "wgl_extension_loader_class";

    if (!RegisterClassA(&wc)) {
        nmutil::log(
                nmutil::LOG_ERROR,
                "unable to register temporary window class\n");

        return NM_FAIL;
    }

    // create window temporary window
    HWND hwnd = CreateWindowExA(
            0,
            wc.lpszClassName,              // class name
            "wgl_extension_loader_window", // window name
            0,                             // window style
            CW_USEDEFAULT, CW_USEDEFAULT,  // position
            CW_USEDEFAULT, CW_USEDEFAULT,  // dimension
            NULL,                // handle to parent
            NULL,                // handle to menu
            g_instance,         // application instance
            NULL);               // no extra params
    if (!hwnd) {
        nmutil::log(nmutil::LOG_ERROR, "unable to create temporary window\n");

        return NM_FAIL;
    }

    // get a device context
    HDC dc = GetDC(hwnd);
    if (!dc) {
        nmutil::log(
                nmutil::LOG_ERROR,
                "unable to create temporary device context\n");

        DestroyWindow(hwnd);

        return NM_FAIL;
    }

    // specifiy an arbitray PFD with OpenGL capabilities
    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR); // size of structure
    pfd.nVersion = 1;                          // default version
    pfd.dwFlags = PFD_SUPPORT_OPENGL;          // OpenGL support

    // choose best matching pixel format
    int32_t pixel_format_idx = ChoosePixelFormat(dc, &pfd);
    if (pixel_format_idx == 0) {
        nmutil::log(
                nmutil::LOG_ERROR, "cannot find an appropriate pixel format\n");

        ReleaseDC(hwnd, dc);
        DestroyWindow(hwnd);

        return NM_FAIL;
    }

    reti = DescribePixelFormat(
            dc, pixel_format_idx, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    if (reti == 0) {
        nmutil::log(nmutil::LOG_WARN, "unable to describe pixel format\n");
    } else {
        // todo optionally check or list properties
    }

    // set pixel format to device context
    ret = SetPixelFormat(dc, pixel_format_idx, &pfd);
    if (ret == FALSE) {
        nmutil::log(nmutil::LOG_ERROR, "unable to set pixel format\n");

        ReleaseDC(hwnd, dc);
        DestroyWindow(hwnd);

        return NM_FAIL;
    }

    // create temporary (helper) opengl context
    HGLRC rc = wglCreateContext(dc);
    if (rc == NULL) {
        nmutil::log(
                nmutil::LOG_ERROR,
                "failed to create the initial rendering context\n");

        ReleaseDC(hwnd, dc);
        DestroyWindow(hwnd);

        return NM_FAIL;
    }

    // enable temporary context
    ret = wglMakeCurrent(dc, rc);

    // load wgl extensions with glad
    int32_t wgl_version = gladLoaderLoadWGL(dc);
    if (wgl_version == 0) {
        nmutil::log(nmutil::LOG_ERROR, "glad WGL loader failed\n");

        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(rc);
        ReleaseDC(hwnd, dc);
        DestroyWindow(hwnd);

        return NM_FAIL;
    }

    nmutil::log(
            nmutil::LOG_INFO, "WGL version %d.%d\n",
            GLAD_VERSION_MAJOR(wgl_version), GLAD_VERSION_MINOR(wgl_version));

    // check whether required extensions are supported
    // todo perhaps pass this in as a list
    bool has_support =
            GLAD_WGL_EXT_swap_control && // wglSwapIntervalEXT
            GLAD_WGL_ARB_pixel_format && // wglChoosePixelFormatARB
            GLAD_WGL_ARB_create_context; // wglCreateContextAttribsARB

    if (!has_support) {
        nmutil::log(
                nmutil::LOG_ERROR,
                "required opengl extensions not supported\n");

        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(rc);
        ReleaseDC(hwnd, dc);
        DestroyWindow(hwnd);

        return NM_FAIL;
    }

    // cleanup the window and everything related to it, as the context and
    // pixelformat are not the ones we actually want

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(rc);
    ReleaseDC(hwnd, dc);
    DestroyWindow(hwnd);
    UnregisterClassA(wc.lpszClassName, g_instance);

    return NM_SUCCESS;
}

nm_ret setup_window(const char *title, int width, int height)
{
    nm_ret ret;
    BOOL retb;
    int32_t reti;

    size_x = width;
    size_y = height;

    // todo could build in support for code paths that fail but are not
    //  necessarily critical, e.g. create a less ideal pixel format

    // get instance handle
    g_instance = GetModuleHandle(NULL);

    // have to load wgl extensions before being able to make calls below
    ret = load_wgl_extensions();
    if (ret == NM_FAIL) return NM_FAIL;

    // window class
    WNDCLASSA wc;
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g_instance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); // default icon
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);   // default arrow
    wc.hbrBackground = NULL;                    // no background
    wc.lpszMenuName = NULL;                     // no menu
    wc.lpszClassName = WND_CLASS_NAME;

    // register the windows class
    if (!RegisterClassA(&wc)) {
        nmutil::log(nmutil::LOG_ERROR, "unable to register the window class\n");

        return NM_FAIL;
    }

    g_hwnd = CreateWindowExA(
            0,                            // optional window style
            WND_CLASS_NAME,               // class name
            title,                        // app name
            WS_OVERLAPPEDWINDOW,          // window style
            CW_USEDEFAULT, CW_USEDEFAULT, // position
            (LONG) width, (LONG) height,  // dimension
            NULL,                         // handle to parent
            NULL,                         // handle to menu
            g_instance,                   // application instance
            NULL);                        // no extra params
    if (!g_hwnd) {
        nmutil::log(nmutil::LOG_ERROR, "unable to create window\n");

        return NM_FAIL;
    }

    g_hdc = GetDC(g_hwnd);
    if (!g_hdc) {
        nmutil::log(nmutil::LOG_ERROR, "unable to create device context\n");

        DestroyWindow(g_hwnd);

        return NM_FAIL;
    }

    // desired pixel format attributes
    const int32_t i_pixel_format_attrib_list[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,       // PFD_DRAW_TO_WINDOW
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,       // PFD_SUPPORT_OPENGL
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,        // PFD_DOUBLEBUFFER
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB, // PFD_TYPE_RGBA
            WGL_COLOR_BITS_ARB, 32, // 32 color bits
            WGL_DEPTH_BITS_ARB, 24, // 32 depth bits
            // require that the driver supports the pixel format
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            // MSAA16
            WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
            WGL_SAMPLES_ARB, 16,
            0 // end
    };

    int32_t pixel_format_idx;
    UINT pixel_format_count;

    // request at most one format
    retb = wglChoosePixelFormatARB(
            g_hdc, i_pixel_format_attrib_list, NULL, 1, &pixel_format_idx,
            &pixel_format_count);
    // function failed, no formats returned, or format is invalid
    if (retb == FALSE || pixel_format_idx == 0 || pixel_format_count == 0) {
        nmutil::log(
                nmutil::LOG_ERROR, "cannot find an appropriate pixel format\n");

        ReleaseDC(g_hwnd, g_hdc);
        DestroyWindow(g_hwnd);

        return NM_FAIL;
    }

    // set actual pixel format to device context
    PIXELFORMATDESCRIPTOR pfd{};
    reti = DescribePixelFormat(
            g_hdc, pixel_format_idx, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    if (reti == 0) {
        nmutil::log(nmutil::LOG_WARN, "unable to describe pixel format\n");
    } else {
        // todo optionally check or list properties
    }

    retb = SetPixelFormat(g_hdc, pixel_format_idx, &pfd);
    if (retb == FALSE) {
        nmutil::log(nmutil::LOG_ERROR, "unable to set pixel format\n");

        ReleaseDC(g_hwnd, g_hdc);
        DestroyWindow(g_hwnd);

        return NM_FAIL;
    }

    const uint32_t OPENGL_MAJOR = 4;
    const uint32_t OPENGL_MINOR = 3;

    int32_t attributes[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, OPENGL_MAJOR,
            WGL_CONTEXT_MINOR_VERSION_ARB, OPENGL_MINOR,
            0 // end
    };

    // create the actual opengl rendering context
    g_hrc = wglCreateContextAttribsARB(g_hdc, NULL, attributes);
    if (!g_hrc) {
        nmutil::log(
                nmutil::LOG_ERROR,
                "unable to create OpenGL rendering context\n");

        ReleaseDC(g_hwnd, g_hdc);
        DestroyWindow(g_hwnd);

        return NM_FAIL;
    }

    // now make actual rendering context the active one
    wglMakeCurrent(g_hdc, g_hrc);

    // load gl functions with glad
    int32_t version = gladLoaderLoadGL();
    if (version == 0) {
        nmutil::log(nmutil::LOG_ERROR, "glad loader failed\n");

        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(g_hrc);
        ReleaseDC(g_hwnd, g_hdc);
        DestroyWindow(g_hwnd);

        return NM_FAIL;
    }

    uint32_t opengl_major = GLAD_VERSION_MAJOR(version);
    uint32_t opengl_minor = GLAD_VERSION_MINOR(version);

    nmutil::log(
            nmutil::LOG_INFO, "GL version %d.%d\n", opengl_major,
            opengl_minor);

    if (opengl_major < OPENGL_MAJOR ||
        (opengl_major >= OPENGL_MAJOR && opengl_minor < OPENGL_MINOR)) {
        nmutil::log(nmutil::LOG_ERROR, "opengl version too low\n");

        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(g_hrc);
        ReleaseDC(g_hwnd, g_hdc);
        DestroyWindow(g_hwnd);

        return NM_FAIL;
    }

    // show the window in the foreground, and set the keyboard focus to it
    ShowWindow(g_hwnd, SW_SHOW);
    SetForegroundWindow(g_hwnd);
    SetFocus(g_hwnd);

    // disable vsync
    wglSwapIntervalEXT(0);

    load_gl_constants();

    return NM_SUCCESS;
}

void cleanup_window()
{
    // release rendering context
    if (!wglMakeCurrent(NULL, NULL)) {
        nmutil::log(LOG_WARN, "could not release rendering context\n");
    }
    // delete rendering context
    if (!wglDeleteContext(g_hrc)) {
        nmutil::log(LOG_WARN, "could not delete rendering context\n");
    }
    g_hrc = NULL;

    // release device context
    if (!ReleaseDC(g_hwnd, g_hdc)) {
        nmutil::log(LOG_WARN, "could not release device context\n");
    }
    g_hdc = NULL;

    // destroy the window
    if (!DestroyWindow(g_hwnd)) {
        nmutil::log(LOG_WARN, "could not destroy window\n");
    }
    g_hwnd = NULL;

    // unregister window class
    if (!UnregisterClass(WND_CLASS_NAME, g_instance)) {
        nmutil::log(LOG_WARN, "could not unregister window class\n");
    }

    // unload glad, symmetric call to gladLoaderLoadGL
    gladLoaderUnloadGL();
}

void swap_buffers()
{
    SwapBuffers(g_hdc);
}

void poll_events()
{
    MSG msg;

    // clear previous messages
    g_messages = {0};

    // empty out the message queue
    while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            // quit the application on quit message
            g_should_stop = true;
        }
        // translate and dispatch to event queue
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void *get_hwnd()
{ return g_hwnd; }

bool is_active()
{ return g_isActive; }

bool should_stop()
{ return g_should_stop; }

uint32_t get_size_x()
{ return size_x; }

uint32_t get_size_y()
{ return size_y; }

int32_t get_mouse_x()
{ return mouse_x; }

int32_t get_mouse_y()
{ return mouse_y; }

messages get_messages()
{ return g_messages; }

bool get_shift_down()
{ return shift_down; }