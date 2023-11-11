#include "Window.hpp"

#if defined(RENDER_DX12)
#include "WinWindow.hpp"
#elif defined(RENDER_OPENGL)
#include "SDLWindow.hpp"
#endif

namespace Engine {

Window* Window::create(const WindowProps& props) {
#if defined(RENDER_DX12)
    return new WinWindow(props);
#elif defined(RENDER_OPENGL)
    return new SDLWindow(props);
#endif
}

}  // namespace Engine
