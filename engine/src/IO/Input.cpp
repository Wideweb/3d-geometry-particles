#include "Input.hpp"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include "WinInput.hpp"
#elif __APPLE__ || __linux__ || __unix__ || defined(_POSIX_VERSION)
#include "SDLInput.hpp"
#endif

namespace Engine {

Input* Input::create() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    return new WinInput();
#elif __APPLE__ || __linux__ || __unix__ || defined(_POSIX_VERSION)
    return new SDLInput();
#endif
}

}  // namespace Engine
