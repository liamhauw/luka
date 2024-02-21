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
  CreateDrawElements();
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

  for (auto& i : frag_resources.stage_inputs) {
    // unsigned set =
    //     frag_compiler.get_decoration(i.id, spv::DecorationDescriptorSet);
    // unsigned binding =
    //     frag_compiler.get_decoration(i.id, spv::DecorationBinding);
    u32 location = frag_compiler.get_decoration(i.id, spv::DecorationLocation);
  }
}

void GeometrySubpass::CreateDrawElements() {
  const sg::Map object{scene_graph_->GetObject()};
  const sg::Scene* scene{object.GetScene()};
  const std::vector<sg::Node*>& nodes{scene->GetNodes()};

  std::queue<const sg::Node*> all_nodes;

  for (const sg::Node* node : nodes) {
    all_nodes.push(node);
  }

  while (!all_nodes.empty()) {
    const sg::Node* cur_node{all_nodes.front()};
    all_nodes.pop();

    const std::vector<sg::Node*>& cur_node_children{cur_node->GetChildren()};
    for (const sg::Node* cur_node_child : cur_node_children) {
      all_nodes.push(cur_node_child);
    }

    const sg::Mesh* mesh{cur_node->GetMesh()};
    const std::vector<sg::Primitive>& primitives{mesh->GetPrimitives()};

    const glm::mat4& model{cur_node->GetMarix()};

    for (const sg::Primitive& primitive : primitives) {
      DrawElement draw_element;
      draw_element.model = model;
    }
  }
}

}  // namespace rd

}  // namespace luka
