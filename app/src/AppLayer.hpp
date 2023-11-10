#pragma once

#include "Engine.hpp"

#include "Geometry.hpp"
#include "GeometryParticle.hpp"
#include "CascadeShadow.hpp"

#include <glm/mat4x4.hpp>
#include <vector>
#include <memory>

struct Light
{
    CascadeShadow::Cascade cascades[4];
    glm::vec3 strength;
    float a;
    glm::vec3 direction;
    float b;
};

struct RenderCommonData {
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 viewPos;
    float time;
    glm::vec4 ambientLight;
    Light light;
};

struct RenderItemData {
  glm::mat4 model;
};

struct Material
{
    glm::vec4 diffuseAlbedo;
    glm::vec3 fresnelR0;
    float roughness;
};

struct DepthRenderCommonData {
    glm::mat4 viewProj;
};

struct DepthMap {
  std::shared_ptr<Engine::CrossPlatformDepthStencilTexture> texture;
  std::shared_ptr<Engine::CrossPlatformFramebuffer> framebuffer;
  std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> renderData;
};

class AppLayer : public Engine::Layer {
  private:
    std::shared_ptr<Engine::CrossPlatformShaderProgram> m_ShaderTexture;
    std::shared_ptr<Engine::CrossPlatformRenderPass> m_RenderPassTexture;

    std::shared_ptr<Engine::CrossPlatformShaderProgram> m_LightShader;
    std::shared_ptr<Engine::CrossPlatformRenderPass> m_LightRenderPass;

    std::shared_ptr<Engine::CrossPlatformTexture> m_Texture;
    std::shared_ptr<Engine::CrossPlatformTexture> m_SkyboxCubeTexture;
    std::shared_ptr<Engine::CrossPlatformShaderProgram> m_SkyboxShader;
    std::shared_ptr<Engine::CrossPlatformRenderPass> m_SkyboxRenderPass;
    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> m_SkyboxRenderData;

    std::shared_ptr<Engine::CrossPlatformShaderProgram> m_DepthShader;
    std::shared_ptr<Engine::CrossPlatformRenderPass> m_DepthRenderPass;
    std::array<DepthMap, 4> m_DepthMaps;
    

    std::shared_ptr<Engine::CrossPlatformShaderProgram> m_ScreenTextureShader;
    std::shared_ptr<Engine::CrossPlatformRenderPass> m_ScreenTextureRenderPass;
    std::vector<std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer>> m_ScreenRenderData;

    glm::mat4 m_WorldTransform = glm::mat4(1.0f);

    glm::vec2 m_MousePos;

    // Engine::Shader m_Shader, m_SurfaceShader;

    // Engine::Texture m_SandTexture, m_SkyTexture;

    // std::shared_ptr<Engine::Model> m_SurfaceModel;
    // glm::mat4 m_SurfaceTransform = glm::mat4(1.0f);

    // std::shared_ptr<Engine::Model> m_GeometryModel;
    glm::mat4 m_GeometryTransform = glm::mat4(1.0f);

    // std::shared_ptr<Engine::Model> m_ParticleModel;
    glm::mat4 m_ParticleTransform = glm::mat4(1.0f);

    std::shared_ptr<Geometry> m_Geometry;
    std::vector<GeometryParticle> m_Particles;

    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> m_CommonRenderData;

    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> m_TerrainRenderData;
    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> m_TerrainMaterialRenderData;

    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> m_GeometryRenderData;
    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> m_GeometryMaterialRenderData;

    std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer> m_ParticleMaterialRenderData;
    std::vector<std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer>> m_ParticlesRenderData;

    std::vector<std::shared_ptr<Engine::CrossPlatformShaderProgramDataBuffer>> m_InstancesRenderData;

    float m_Time = 0.0f;

    Engine::Mesh createCube(float left, float right, float bottom, float top, float back, float front);
    Engine::Mesh createCube2();
    Engine::Mesh createPlane(float tileSize, int columns, int rows);
    void initDepthPass();
    void initScreenTexturePass();

  public:
    using Layer::Layer;

    virtual void onAttach() override;
    virtual void onUpdate() override;
    virtual void onDraw() override;
    virtual void onDetach() override;
    virtual void onMouseEvent(Engine::MouseEvent &event) override;
};
