# Luka engine

## Platform
- Windows
- macOS

## Prerequisites
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
