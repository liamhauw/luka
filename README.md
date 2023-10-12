# luka engine

## Features
- Asset
  - glTF scene format
- Rendering
  - Physically based rendering
  - Bindless resources
  - Automation pipeline layout generation
  - Improving load times with a pipeline cache
  - Task-based multi-threading
  - Asynchronously load resources
  - Draw in parallel threads
  - Using frame graph to drive rendering and automate resource management and layout transitions
  - Topological sort


## Prerequisites
- Windows/macOS
- Git
- CMake
- Ninja
- MSVC/Clang/GCC
- Vulkan SDK
- VSCode (optional)

## Clone
```shell
git clone --recurse-submodules https://github.com/liamhauw/luka.git
cd luka
```

## Build

```shell
# Debug
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Release
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Run
```shell
build/engine/luka_engine
```
