#include "CrossPlatformRender.hpp"

#define RENDER_DX12

#if defined(RENDER_DX12)
#include "AdapterDx.hpp"
#elif defined(RENDER_OPENGL)
#include "AdapterOpenGL.hpp"
#endif

namespace Engine {

CrossPlatformRender* CrossPlatformRender::create(void* window, uint32_t width, uint32_t height) {
#if defined(RENDER_DX12)
    return new DxPlatformRenderWrapper(window, width, height);
#elif defined(RENDER_OPENGL)
    return new OpenGLPlatformRenderWrapper(window, width, height);
#endif
}

}  // namespace Engine
