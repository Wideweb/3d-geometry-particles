cmake_minimum_required(VERSION 3.13)

project(GameApp VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_UNITY_BUILD ON)

set(SOURCE_EXE 
    src/main.cpp
    src/AppLayer.cpp
    src/Geometry.cpp
    src/GeometryParticle.cpp
)	

add_executable(${PROJECT_NAME} ${SOURCE_EXE})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE Engine)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${OUTPUT_DIRECTORY})