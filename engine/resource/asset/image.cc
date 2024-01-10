// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/image.h"

namespace luka {

namespace ast {

Image::Image(std::vector<u8>&& data, vk::Format format,
             std::vector<Mipmap>&& mipmap, u32 layer_count, u32 face_count,
             std::vector<std::vector<std::vector<u64>>>&& offsets,
             const std::string& name)
    : data_{std::move(data)},
      format_{format},
      mipmaps_{std::move(mipmap)},
      layer_count_{layer_count},
      face_count_{face_count},
      offsets_{std::move(offsets)},
      name_{name} {}

const std::vector<u8>& Image::Image::GetDate() const { return data_; }

vk::Format Image::GetFormat() const { return format_; }

const std::vector<Mipmap>& Image::GetMipmaps() const { return mipmaps_; }

u32 Image::GetLayerCount() const { return layer_count_; }

u32 Image::GetFaceCount() const { return face_count_; }

const std::vector<std::vector<std::vector<u64>>>& Image::GetOffsets() const {
  return offsets_;
}

const std::string& Image::GetName() const { return name_; }

}  // namespace ast

}  // namespace luka
