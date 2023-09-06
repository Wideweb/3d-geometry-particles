cmake_minimum_required(VERSION 3.13)

project(Engine VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(ENV{CONAN_SYSREQUIRES_MODE} enabled)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_PREFIX_PATH})

set(SOURCE_LIB 
    vendor/glad/glad.c
    vendor/imgui_impl_opengl3.cpp
    vendor/imgui_impl_sdl.cpp
    src/Core/Time.cpp
    src/Core/File.cpp
    src/Core/Math.cpp
    src/IO/Window.cpp
    src/IO/Input.cpp
    src/IO/SDL/SDLWindow.cpp
    src/IO/SDL/SDLInput.cpp
    src/Engine/Application.cpp
    src/Engine/Layer.cpp
    src/Engine/CameraController.cpp
    src/Render3D/Camera.cpp
    src/Render3D/Models/Material.cpp
    src/Render3D/Models/Mesh.cpp
    src/Render3D/Models/Model.cpp
    src/Render3D/Renderers/MasterRenderer.cpp
    src/Render3D/GfxObjects/GfxUtils.cpp
    src/Render3D/GfxObjects/GfxImage.cpp
    src/Render3D/GfxObjects/Texture.cpp
    src/Render3D/GfxObjects/Renderbuffer.cpp
    src/Render3D/GfxObjects/Framebuffer.cpp
    src/Render3D/GfxObjects/Shader.cpp
    src/Render3D/ModelLoader.cpp
    src/Render3D/ModelFactory.cpp
    src/Render3D/Viewport.cpp
    src/Render3D/TextureLoader.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCE_LIB})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
                                         ${CMAKE_CURRENT_SOURCE_DIR}/src/Core
                                         ${CMAKE_CURRENT_SOURCE_DIR}/src/IO
                                         ${CMAKE_CURRENT_SOURCE_DIR}/src/IO/SDL
                                         ${CMAKE_CURRENT_SOURCE_DIR}/src/Render3D
                                         ${CMAKE_CURRENT_SOURCE_DIR}/src/Render3D/GfxObjects
                                         ${CMAKE_CURRENT_SOURCE_DIR}/src/Render3D/Models
                                         ${CMAKE_CURRENT_SOURCE_DIR}/src/Render3D/Renderers
                                         ${CMAKE_CURRENT_SOURCE_DIR}/src/Render3D/Utils
                                         ${CMAKE_CURRENT_SOURCE_DIR}/src/Sound
                                         ${CMAKE_CURRENT_SOURCE_DIR}/src/Sound/SDL
                                         ${CMAKE_CURRENT_SOURCE_DIR}/src/Engine
                                         ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan.cmake"
                EXPECTED_HASH SHA256=396e16d0f5eabdc6a14afddbcfff62a54a7ee75c6da23f32f7a31bc85db23484
                TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_configure(REQUIRES sdl/2.0.14 assimp/5.0.1 glm/0.9.9.8 zlib/1.2.12
                      GENERATORS cmake_find_package)

conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE .
                    BUILD missing
                    REMOTE conancenter
                    SETTINGS ${settings})

find_package(SDL2 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)

find_package(glm REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)

find_package(assimp REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)

# target_link_libraries(${PROJECT_NAME} PRIVATE -lprofiler)
# target_link_libraries(${PROJECT_NAME} PRIVATE -ltcmalloc)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/Render3D/shaders DESTINATION ${OUTPUT_DIRECTORY})