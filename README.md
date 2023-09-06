## Building

The following set of tools is required:
- gcc
- conan
- cmake

build steps:
- cd 3d-geometry-particles
- mkdir build
- cd build
- cmake .. -DCMAKE_BUILD_TYPE=Debug
- cmake --build .

run: ./GameApp