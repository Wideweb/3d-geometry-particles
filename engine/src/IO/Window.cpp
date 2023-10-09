#include "Window.hpp"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include "WinWindow.hpp"
#elif __APPLE__ || __linux__ || __unix__ || defined(_POSIX_VERSION)
#include "SDLWindow.hpp"
#endif

namespace Engine {

Window *Window::create(const WindowProps &props) {

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    return new WinWindow(props);
#elif __APPLE__ || __linux__ || __unix__ || defined(_POSIX_VERSION)
    return new SDLWindow(props);
#endif

}

} // namespace Engine
