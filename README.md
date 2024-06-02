# luka
A graphics engine with modular components including platform, core, base, resource, function, editor, and rendering.

## Features
- Following the RAII principle
- Task-based multi-threading using enkiTS
- Support for glTF scenes
- Configurability of multiple types of punctual lights
- User-defined shaders
- Driving rendering with frame graph
- Asynchronously loading asset
- Improving load time with spirv cache and pipeline cache
- Automating pipeline layout generation with SPIRV-Cross
- Bindless rendering with descriptor indexing
- Recording commands on multiple threads
- User interface using imgui

## Prerequisites
Engine supports for Windows and macOS and requires the following dependencies to be installed:
- VSCode (with CMake Language Support, C/C++ and clangd extensions)
- Git
- CMake
- Ninja
- LLVM (with Visual Studio on Windows / Xcode on macOS)
- Vulkan SDK

These dependencies can be installed manually or through package managers.

On Windows if you have [Scoop](https://scoop.sh/) installed, you can easily install some prerequisites using the following command:
```shell
scoop install vscode git cmake ninja llvm vulkan
```
Then you only need to install VSCode extensions and Visual Studio.

On macOS if you have [Homebrew](https://brew.sh/) installed, you can easily install some prerequisites using the following command:
```shell
brew install visual-studio-code git cmake ninja llvm
```
Then you only need to install VSCode extensions, Xcode and Vulkan SDK.

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
./luka_engine
```

## Development
### Debug
Click Run and Debug in the sidebar, and select Windows/macOS Debug/RelWithDebInfo. Click Start Debugging or press F5.

### Update submodule
```shell
git submodule sync
git pull --recurse-submodules

git submodule update --remote
```

## Third party
- [enkiTS](https://github.com/dougbinks/enkiTS)
- [glfw](https://github.com/glfw/glfw)
- [glm](https://github.com/g-truc/glm)
- [glslang](https://github.com/KhronosGroup/glslang)
- [imgui](https://github.com/ocornut/imgui)
- [json](https://github.com/nlohmann/json)
- [spdlog](https://github.com/gabime/spdlog)
- [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross)
- [stb](https://github.com/nothings/stb)
- [tinygltf](https://github.com/syoyo/tinygltf)
- [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
