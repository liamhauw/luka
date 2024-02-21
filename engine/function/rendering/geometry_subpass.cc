// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/geometry_subpass.h"

#include <spirv_glsl.hpp>

namespace luka {

namespace rd {

GeometrySubpass::GeometrySubpass(std::shared_ptr<Asset> asset,
                                 std::shared_ptr<Gpu> gpu,
                                 std::shared_ptr<SceneGraph> scene_graph)
    : Subpass{asset, gpu, scene_graph} {
  CreatePipeline();
}

void GeometrySubpass::CreatePipeline() {
  std::vector<u32> vert_spirv{asset_->GetAssetInfo().vertex.CompileToSpirv()};
  std::vector<u32> frag_spirv{asset_->GetAssetInfo().fragment.CompileToSpirv()};

  spirv_cross::CompilerGLSL vert_compiler{std::move(vert_spirv)};
  spirv_cross::CompilerGLSL frag_compiler{std::move(frag_spirv)};

   spirv_cross::ShaderResources vert_resources{
      vert_compiler.get_shader_resources()};
  spirv_cross::ShaderResources frag_resources{
      frag_compiler.get_shader_resources()};

  for (auto& i : frag_resources.sampled_images) {
    unsigned set =
        frag_compiler.get_decoration(i.id, spv::DecorationDescriptorSet);
    unsigned binding =
        frag_compiler.get_decoration(i.id, spv::DecorationBinding);

  }
}

void GeometrySubpass::CreateDrawElements() {}

}  // namespace rd

}  // namespace luka
