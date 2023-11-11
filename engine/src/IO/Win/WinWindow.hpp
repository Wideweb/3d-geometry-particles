#pragma once

#include "Window.hpp"

#define NOMINMAX
#include <windows.h>

namespace Engine {

class WinWindow : public Window {
private:
    WindowProps m_Props;

    HINSTANCE m_AppInstance;
    HWND      m_Window;

    EventCallbackFn<MouseEvent&>  m_mouseEventCallback;
    EventCallbackFn<WindowEvent&> m_windowEventCallback;
    EventCallbackFn<void*>        m_nativeEventCallback;
    MouseEvent                    m_MouseEvent{};

public:
    WinWindow(const WindowProps& props);
    virtual ~WinWindow();

    virtual int       getWidth() const override;
    virtual int       getHeight() const override;
    virtual glm::vec2 getSize() const override;

    virtual void getDrawableSize(int& width, int& height) const override;

    virtual void setMouseEventCallback(const EventCallbackFn<MouseEvent&>& callback) override;

    virtual void setWindowEventCallback(const EventCallbackFn<WindowEvent&>& callback) override;

    virtual void setNativeEventCallback(const EventCallbackFn<void*>& callback) override;

    virtual void        readInput() override;
    virtual void        swapBuffers() override;
    virtual void        shutDown() override;
    virtual void*       getNaviteWindow() const override;
    virtual void*       getContext() const override;
    virtual MouseEvent& getMouseEvent() override;

    LRESULT msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    void onResize();
};

}  // namespace Engine
