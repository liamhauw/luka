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
             std::vector<std::vector<std::vector<u64>>>&& offsets)
    : data_{data},
      format_{format},
      mipmaps_{mipmap},
      layer_count_{layer_count},
      face_count_{face_count},
      offsets_{offsets} {}

}  // namespace ast

}  // namespace luka
