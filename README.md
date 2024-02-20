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
# Prepare source code
git clone --recurse-submodules https://github.com/liamhauw/luka.git
git pull --recurse-submodules
cd luka
python third_party/shaderc/utils/git-sync-deps

# Build
cmake --preset=Base
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
- [ctpl](https://github.com/vit-vit/CTPL)
- [glfw](https://github.com/glfw/glfw)
- [glm](https://github.com/g-truc/glm)
- [imgui](https://github.com/ocornut/imgui)
- [json](https://github.com/nlohmann/json)
- [shaderc](https://github.com/liamhauw/shaderc)
- [spdlog](https://github.com/gabime/spdlog)
- [spirv-cross](https://github.com/KhronosGroup/SPIRV-Cross)
- [stb](https://github.com/nothings/stb)
- [tinygltf](https://github.com/syoyo/tinygltf)
- [vma](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
