cmake_minimum_required(VERSION 3.13)

set(CMAKE_TOOLCHAIN_FILE D:/vcpkg/scripts/buildsystems/vcpkg.cmake)

project(Engine VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(ENV{CONAN_SYSREQUIRES_MODE} enabled)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_PREFIX_PATH})

set(SOURCE_LIB 
    src/Core/Time.cpp
    src/Core/File.cpp
    src/Core/Math.cpp
    src/IO/Window.cpp
    src/IO/Input.cpp
    src/Render/Abstraction/Camera.cpp
    src/Render/Abstraction/CrossPlatformRender.cpp
    src/Engine/Application.cpp
    src/Engine/Layer.cpp
    src/Engine/CameraController.cpp
)

set(INCLUDE_DIRS 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IO
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Render/Abstraction
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Engine
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb
)

if (UNIX)
  list(APPEND SOURCE_LIB
    src/IO/SDL/SDLWindow.cpp
    src/IO/SDL/SDLInput.cpp
    vendor/glad/glad.c
    vendor/imgui-impl/imgui_impl_opengl3.cpp
    vendor/imgui-impl/imgui_impl_sdl.cpp
  )

  list(APPEND INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IO/SDL
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui-impl
  )
endif (UNIX)

if (WIN32)
  list(APPEND SOURCE_LIB
      src/IO/Win/WinWindow.cpp
      src/IO/Win/WinInput.cpp
      src/Render/Dx12Render/DxDescriptorPool.cpp
      src/Render/Dx12Render/DxGeometryRegistry.cpp
      src/Render/Dx12Render/DxRender.cpp
      src/Render/Dx12Render/DxTextureLoader.cpp
      src/Render/Dx12Render/DxUtils.cpp
      src/Render/Dx12Render/Infrastructure/DxDepthStencilTexture.cpp
      src/Render/Dx12Render/Infrastructure/DxFramebuffer.cpp
      src/Render/Dx12Render/Infrastructure/DxRenderPass.cpp
      src/Render/Dx12Render/Infrastructure/DxRenderTexture.cpp
      src/Render/Dx12Render/Infrastructure/DxShaderProgram.cpp
      src/Render/Dx12Render/Infrastructure/DxShaderProgramSlot.cpp
  )

  list(APPEND INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IO/Win
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Render/Adapters/Dx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Render/Dx12Render
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Render/Dx12Render/Frame
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Render/Dx12Render/Infrastructure
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/dx
  )
endif (WIN32)

add_library(${PROJECT_NAME} STATIC ${SOURCE_LIB})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIRS})

if (UNIX)

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

endif(UNIX)

if (WIN32)

  target_link_libraries(${PROJECT_NAME} PRIVATE
      d3d12.lib dxgi.lib dxguid.lib uuid.lib
      dxgi.lib d3dcompiler.lib
      kernel32.lib user32.lib
      comdlg32.lib advapi32.lib shell32.lib
      ole32.lib oleaut32.lib
  )

  # include_directories(C:/Alckevich/vcpkg/installed/x64-windows/include)
  # link_directories(C:/Alckevich/vcpkg/installed/x64-windows/lib)

  target_include_directories(${PROJECT_NAME} PRIVATE ${glm_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} PRIVATE ${glm_LIBRARIES})

  target_include_directories(${PROJECT_NAME} PRIVATE ${assimp_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} PRIVATE ${assimp_LIBRARIES})

endif(WIN32)

# target_link_libraries(${PROJECT_NAME} PRIVATE -lprofiler)
# target_link_libraries(${PROJECT_NAME} PRIVATE -ltcmalloc)

#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/Render3D/shaders DESTINATION ${OUTPUT_DIRECTORY})