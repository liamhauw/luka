# luka engine

## Features
- Bindless rendering with descriptor indexing
- Automating pipeline layout generation with spirv-cross
- Improving load time with pipeline cache
- Asynchronously load asset using a task-based multi-threading library

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
# Prepare source code
git clone --recurse-submodules https://github.com/liamhauw/luka.git
cd luka

# Build
cmake --preset=Base
cmake --build --preset=Release
cmake --install build --config Release

# Run
cd bin
luka_engine
```

## Development

### Debug
Click Run and Debug in the sidebar, and select Windows/macOS Debug/RelWithDebInfo. Click Start Debugging or press F5.

### Update submodule
```shell
git pull --recurse-submodules
git submodule update --remote
```

## Third party
- [enkits](https://github.com/dougbinks/enkiTS)
- [glfw](https://github.com/glfw/glfw)
- [glm](https://github.com/g-truc/glm)
- [glslang](https://github.com/KhronosGroup/glslang)
- [imgui](https://github.com/ocornut/imgui)
- [json](https://github.com/nlohmann/json)
- [spdlog](https://github.com/gabime/spdlog)
- [spirv-cross](https://github.com/KhronosGroup/SPIRV-Cross)
- [stb](https://github.com/nothings/stb)
- [tinygltf](https://github.com/syoyo/tinygltf)
- [vma](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
