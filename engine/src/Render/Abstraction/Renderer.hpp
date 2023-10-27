#pragma once

#include "Camera.hpp"

class Renderer {
  public:
    void setCamera(Camera* camera);

    void draw();

    void setClearColor(glm::vec4 color);
    
    void setViewport(int width, int height) {
        #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
           return dxRenderer->resize(width, height);
        #elif __APPLE__ || __linux__ || __unix__ || defined(_POSIX_VERSION)
           return 0;
        #endif
    }
    
    void getViewport(int& width, int& height) const;
    
    void clear();

    void registerGeometry(const std::string& geometry, const std::vector<std::string>& subGeometries, const std::vector<Mesh>& subMeshes) {
        #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
           return dxRenderer->registerGeometry(geometry, subGeometries, subMeshes);
        #elif __APPLE__ || __linux__ || __unix__ || defined(_POSIX_VERSION)
           return 0;
        #endif
    }

   void createRenderPass() {
      std::shared_ptr<RenderPass> pass = dxRender->createRenderPass(std::shared_ptr<ShaderProgram> shaderProgram, size_t constantBuffersNum, size_t texturesNum);
   }
};