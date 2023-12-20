// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/scene_graph/sampler.h"

namespace luka {

namespace sg {
Sampler::Sampler(vk::raii::Sampler&& sampler, const std::string& name)
    : Component{name}, sampler_{std::move(sampler)} {}

std::type_index Sampler::GetType() { return typeid(Sampler); }
}  // namespace sg

}  // namespace luka
