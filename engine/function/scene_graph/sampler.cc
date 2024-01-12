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

Sampler::Sampler(std::shared_ptr<Gpu> gpu,
                 const tinygltf::Sampler& model_sampler)
    : Component{model_sampler.name} {
  vk::Filter mag_filter;
  switch (model_sampler.minFilter) {
    case TINYGLTF_TEXTURE_FILTER_NEAREST:
      mag_filter = vk::Filter::eNearest;
      break;
    case TINYGLTF_TEXTURE_FILTER_LINEAR:
      mag_filter = vk::Filter::eLinear;
      break;
    default:
      mag_filter = vk::Filter::eNearest;
  }

  vk::Filter min_filter;
  switch (model_sampler.minFilter) {
    case TINYGLTF_TEXTURE_FILTER_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
      min_filter = vk::Filter::eNearest;
      break;
    case TINYGLTF_TEXTURE_FILTER_LINEAR:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
      min_filter = vk::Filter::eLinear;
      break;
    default:
      min_filter = vk::Filter::eNearest;
  }

  vk::SamplerMipmapMode mipmap_mode;
  switch (model_sampler.minFilter) {
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
      mipmap_mode = vk::SamplerMipmapMode::eNearest;
      break;
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
      mipmap_mode = vk::SamplerMipmapMode::eLinear;
      break;
    default:
      mipmap_mode = vk::SamplerMipmapMode::eNearest;
  }

  vk::SamplerAddressMode address_mode_u;
  switch (model_sampler.wrapS) {
    case TINYGLTF_TEXTURE_WRAP_REPEAT:
      address_mode_u = vk::SamplerAddressMode::eRepeat;
      break;
    case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
      address_mode_u = vk::SamplerAddressMode::eClampToEdge;
      break;
    case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
      address_mode_u = vk::SamplerAddressMode::eMirroredRepeat;
      break;
    default:
      address_mode_u = vk::SamplerAddressMode::eRepeat;
  }

  vk::SamplerAddressMode address_mode_v;
  switch (model_sampler.wrapT) {
    case TINYGLTF_TEXTURE_WRAP_REPEAT:
      address_mode_v = vk::SamplerAddressMode::eRepeat;
      break;
    case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
      address_mode_v = vk::SamplerAddressMode::eClampToEdge;
      break;
    case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
      address_mode_v = vk::SamplerAddressMode::eMirroredRepeat;
      break;
    default:
      address_mode_v = vk::SamplerAddressMode::eRepeat;
  }

  vk::SamplerCreateInfo sampler_ci{{},          mag_filter,     min_filter,
                                   mipmap_mode, address_mode_u, address_mode_v};

  sampler_ = gpu->CreateSampler(sampler_ci, GetName());
}

std::type_index Sampler::GetType() { return typeid(Sampler); }
}  // namespace sg

}  // namespace luka
