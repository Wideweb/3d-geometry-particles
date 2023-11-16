cmake_minimum_required(VERSION 3.13)

project(GameApp VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_UNITY_BUILD ON)

set(SOURCE_EXE 
    src/main.cpp
    src/AppLayer.cpp
    src/gfx/CascadeShadows/CascadeShadowEffect.cpp
    src/gfx/GeometryParticles/GeometryParticleEffect.cpp
    src/gfx/GeometryParticles/Geometry.cpp
    src/gfx/GeometryParticles/GeometryParticle.cpp
    src/gfx/Skybox/SkyboxEffect.cpp
    src/gfx/SkyboxNoise/SkyboxNoiseEffect.cpp
    src/gfx/FrustrumCulling/FrustrumCullingEffect.cpp
    src/gfx/FrustrumCullingGPU/FrustrumCullingGPUEffect.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCE_EXE})

set(INCLUDE_DIRS 
    src
    src/gfx
    src/gfx/CascadeShadows
    src/gfx/GeometryParticles
    src/gfx/Skybox
    src/gfx/SkyboxNoise
    src/gfx/FrustrumCulling
    src/gfx/FrustrumCullingGPU
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE Engine)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${OUTPUT_DIRECTORY})