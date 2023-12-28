// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

namespace luka {

namespace ast {

struct Mipmap {
  u32 level{0};
  vk::Extent3D extent{0, 0, 0};
};

class Image {
 public:
  Image() = default;
  Image(std::vector<u8>&& data, vk::Format format, std::vector<Mipmap>&& mipmap,
        u32 layer_count, u32 face_count,
        std::vector<std::vector<std::vector<u64>>>&& offsets,
        const std::string& name);

  const std::vector<u8>& GetDate() const;
  vk::Format GetFormat() const;
  const std::vector<Mipmap>& GetMipmaps() const;
  u32 GetLayerCount() const;
  u32 GetFaceCount() const;
  const std::vector<std::vector<std::vector<u64>>>& GetOffsets() const;
  const std::string& GetName() const;

 private:
  std::vector<u8> data_;
  vk::Format format_;
  std::vector<Mipmap> mipmaps_;
  u32 layer_count_;
  u32 face_count_;

  // offsets[level_index][layer_index][face_index]
  std::vector<std::vector<std::vector<u64>>> offsets_;

  std::string name_;
};

}  // namespace ast

}  // namespace luka
