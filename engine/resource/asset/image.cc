// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/image.h"

#include "core/log.h"

namespace luka {

namespace ast {

Image::Image(std::vector<u8>&& data, std::vector<vk::Extent3D>&& mipmap_extents,
             vk::Format format, u32 level_count, u32 layer_count,
             u32 face_count,
             std::vector<std::vector<std::vector<u64>>>&& offsets,
             const std::string& name)
    : data_{std::move(data)},
      mipmap_extents_{std::move(mipmap_extents)},
      format_{format},
      level_count_{level_count},
      layer_count_{layer_count},
      face_count_{face_count},
      offsets_{std::move(offsets)},
      name_{name} {}

Image::Image(tinygltf::Image& tinygltf_image)
    : data_{std::move(tinygltf_image.image)},
      mipmap_extents_{{static_cast<u32>(tinygltf_image.width),
                       static_cast<u32>(tinygltf_image.height), 1}},
      level_count_{1},
      layer_count_{1},
      face_count_{1},
      name_{!tinygltf_image.name.empty() ? tinygltf_image.name
                                         : tinygltf_image.uri} {
  if (tinygltf_image.component == 4 && tinygltf_image.bits == 8) {
    format_ = vk::Format::eR8G8B8A8Unorm;
  } else {
    THROW("Unsupport image format.");
  }

  tinygltf_image.image.clear();
}

const std::vector<u8>& Image::Image::GetDate() const { return data_; }

const std::vector<vk::Extent3D>& Image::GetMipmapExtents() const {
  return mipmap_extents_;
}

vk::Format Image::GetFormat() const { return format_; }

u32 Image::GetLevelCount() const { return level_count_; }

u32 Image::GetLayerCount() const { return layer_count_; }

u32 Image::GetFaceCount() const { return face_count_; }

const std::vector<std::vector<std::vector<u64>>>& Image::GetOffsets() const {
  return offsets_;
}

const std::string& Image::GetName() const { return name_; }

}  // namespace ast

}  // namespace luka
