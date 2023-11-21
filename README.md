# luka engine

## Features

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
```

## Use

### Terminal
```shell
# Config
cmake --preset=Base

# Build
cmake --build --preset=[Debug/Release/RelWithDebInfo]

# Run
build/engine/[Debug/Release/RelWithDebInfo]/luka_engine
```

### VSCode
Click Run and Debug in the sidebar, and select Windows Debug or macOS Debug. Click Start Debugging or press F5.
