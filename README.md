# luka engine

## Features
- Modular

## Prerequisites
- Windows/macOS
- Git
- CMake
- Ninja
- MSVC/Clang/GCC
- Vulkan SDK
- VSCode (optional)

## Use

```shell
# Clone
git clone --recurse-submodules https://github.com/liamhauw/luka.git

# Pull 
git pull --recurse-submodules

# Config
cmake --preset=Base

# Build
cmake --build --preset=[Debug/Release/RelWithDebInfo]

# Run
build/engine/[Debug/Release/RelWithDebInfo]/luka_engine
```

## Development

### Debug
Click Run and Debug in the sidebar, and select Windows Debug or macOS Debug. Click Start Debugging or press F5.

### Update submodule
```shell
git submodule update --remote
```

## Third party
- [glfw](https://github.com/glfw/glfw)
- [glm](https://github.com/g-truc/glm)
- [imgui](https://github.com/ocornut/imgui)
- [json](https://github.com/nlohmann/json)
- [ktx](https://github.com/KhronosGroup/KTX-Software)
- [spdlog](https://github.com/gabime/spdlog)
- [stb](https://github.com/nothings/stb)
- [tinygltf](https://github.com/syoyo/tinygltf)
- [vma](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)