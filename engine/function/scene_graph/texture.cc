// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/texture.h"

namespace luka {

namespace sg {

Texture::Texture(Image* image, Sampler* sampler, const std::string& name)
    : Component{name}, image_{image}, sampler_{sampler} {}

std::type_index Texture::GetType() { return typeid(Texture); }

}  // namespace sg

}  // namespace luka