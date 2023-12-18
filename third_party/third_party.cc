// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "tiny_gltf.h"

#define VMA_IMPLEMENTATION
#define VMA_VULKAN_VERSION 1003000
#include "vk_mem_alloc.h"
