#include "WinWindow.hpp"

#include <WindowsX.h>

#include <cassert>
#include <iostream>
#include <sstream>
#include <string_view>

namespace Engine {

static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    WinWindow* window;

    if (uMsg == WM_NCCREATE) {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        window              = static_cast<WinWindow*>(lpcs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    } else {
        window = reinterpret_cast<WinWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (window != nullptr) {
        return window->msgProc(hwnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

WinWindow::WinWindow(const WindowProps& props) {
    using namespace std;
    using namespace std::string_view_literals;

    m_AppInstance = static_cast<HINSTANCE>(props.appInstance);
    m_Props       = props;

    WNDCLASS wc;
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = windowProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = m_AppInstance;
    wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName  = 0;
    wc.lpszClassName = "MainWnd";

    if (!RegisterClass(&wc)) {
        cerr << "RegisterClass Failed." << endl;
        return;
    }

    // Compute window rectangle dimensions based on requested client area dimensions.
    RECT R = {0, 0, props.width, props.height};
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
    int width  = R.right - R.left;
    int height = R.bottom - R.top;

    m_Window = CreateWindow(
        "MainWnd",            // Window class
        "Engine",             // Window text
        WS_OVERLAPPEDWINDOW,  // Window Style
        CW_USEDEFAULT,        // Window initial horizontal position
        CW_USEDEFAULT,        // Window initial vertical position
        width,                // Window width in device units
        height,               // Window height in device units
        0,                    // Parent window handle
        0,                    // Menu handle
        m_AppInstance,        // Application handle
        this                  // Data
    );

    if (!m_Window) {
        cerr << "CreateWindow Failed." << endl;
        return;
    }

    ShowWindow(m_Window, SW_SHOW);
    UpdateWindow(m_Window);
}

void WinWindow::onResize() {}

WinWindow::~WinWindow() {}

int WinWindow::getWidth() const { return m_Props.width; }

int WinWindow::getHeight() const { return m_Props.height; }

glm::vec2 WinWindow::getSize() const { return glm::vec2(m_Props.width, m_Props.height); };

void WinWindow::setMouseEventCallback(const EventCallbackFn<MouseEvent&>& callback) { m_mouseEventCallback = callback; }

void WinWindow::setWindowEventCallback(const EventCallbackFn<WindowEvent&>& callback) {
    m_windowEventCallback = callback;
}

void WinWindow::setNativeEventCallback(const EventCallbackFn<void*>& callback) { m_nativeEventCallback = callback; }

LRESULT WinWindow::msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        // WM_ACTIVATE is sent when the window is activated or deactivated.
        // We pause the game when the window is deactivated and unpause it
        // when it becomes active.
        case WM_ACTIVATE: {
            if (LOWORD(wParam) == WA_INACTIVE) {
                // mAppPaused = true;
                // mTimer.Stop();
            } else {
                // mAppPaused = false;
                // mTimer.Start();
            }
            return 0;
        }

        // WM_SIZE is sent when the user resizes the window.
        case WM_SIZE: {
            // Save the new client area dimensions.
            m_Props.width  = LOWORD(lParam);
            m_Props.height = HIWORD(lParam);

            WindowEvent event(EventType::WindowResized);
            if (m_windowEventCallback) {
                m_windowEventCallback(event);
            }
            return 0;
        }

        // WM_DESTROY is sent when the window is being destroyed.
        case WM_DESTROY: {
            WindowEvent event(EventType::WindowClosed);
            if (m_windowEventCallback) {
                m_windowEventCallback(event);
            }
            PostQuitMessage(0);
            return 0;
        }

        // The WM_MENUCHAR message is sent when a menu is active and the user presses
        // a key that does not correspond to any mnemonic or accelerator key.
        case WM_MENUCHAR:
            // Don't beep when we alt-enter.
            return MAKELRESULT(0, MNC_CLOSE);

        // Catch this message so to prevent the window from becoming too small.
        case WM_GETMINMAXINFO: {
            ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
            ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
            return 0;
        }

        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN: {
            m_MouseEvent =
                MouseEvent(GET_X_LPARAM(lParam), m_Props.height - GET_Y_LPARAM(lParam), EventType::MouseDown);
            m_mouseEventCallback(m_MouseEvent);
            return 0;
        }
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            m_MouseEvent = MouseEvent(GET_X_LPARAM(lParam), m_Props.height - GET_Y_LPARAM(lParam), EventType::MouseUp);
            m_mouseEventCallback(m_MouseEvent);
            return 0;
        }
        case WM_MOUSEMOVE: {
            m_MouseEvent =
                MouseEvent(GET_X_LPARAM(lParam), m_Props.height - GET_Y_LPARAM(lParam), EventType::MouseMoved);
            m_mouseEventCallback(m_MouseEvent);
            return 0;
        }
        case WM_MOUSEWHEEL: {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);

            m_MouseEvent = MouseEvent(0, delta, EventType::MouseWheel);
            m_mouseEventCallback(m_MouseEvent);
            return 0;
        }
        case WM_MOUSEHWHEEL: {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);

            m_MouseEvent = MouseEvent(delta, 0, EventType::MouseWheel);
            m_mouseEventCallback(m_MouseEvent);
            return 0;
        }
        case WM_KEYUP: {
            if (wParam == VK_ESCAPE) {
                PostQuitMessage(0);
            }

            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void WinWindow::readInput() {
    {
        WindowEvent event(EventType::TextInput, "");
        m_windowEventCallback(event);
    }

    MSG msg = {0};
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void* WinWindow::getNaviteWindow() const { return m_Window; }

void* WinWindow::getContext() const { return nullptr; }

MouseEvent& WinWindow::getMouseEvent() { return m_MouseEvent; }

void WinWindow::getDrawableSize(int& width, int& height) const {
    width  = m_Props.width;
    height = m_Props.height;
}

void WinWindow::swapBuffers() {}

void WinWindow::shutDown() {}

}  // namespace Engine
