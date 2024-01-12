// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <tiny_gltf.h>

namespace luka {

namespace ast {

class Image {
 public:
  Image() = default;

  Image(std::vector<u8>&& data, std::vector<vk::Extent3D>&& mipmap_extents,
        vk::Format format, u32 level_count, u32 layer_count, u32 face_count,
        std::vector<std::vector<std::vector<u64>>>&& offsets,
        const std::string& name = {});

  Image(tinygltf::Image& tinygltf_image);

  const std::vector<u8>& GetDate() const;
  const std::vector<vk::Extent3D>& GetMipmapExtents() const;
  vk::Format GetFormat() const;
  u32 GetLevelCount() const;
  u32 GetLayerCount() const;
  u32 GetFaceCount() const;
  const std::vector<std::vector<std::vector<u64>>>& GetOffsets() const;
  const std::string& GetName() const;

 private:
  std::vector<u8> data_;
  std::vector<vk::Extent3D> mipmap_extents_;
  vk::Format format_;
  u32 level_count_;
  u32 layer_count_;
  u32 face_count_;
  // offsets[level_index][layer_index][face_index]
  std::vector<std::vector<std::vector<u64>>> offsets_;

  std::string name_;
};

}  // namespace ast

}  // namespace luka
