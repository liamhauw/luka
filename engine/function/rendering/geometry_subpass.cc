// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/geometry_subpass.h"

namespace luka {

namespace rd {

GeometrySubpass::GeometrySubpass(std::shared_ptr<Asset> asset,
                                 std::shared_ptr<Gpu> gpu,
                                 std::shared_ptr<SceneGraph> scene_graph)
    : Subpass{asset, gpu, scene_graph} {
  CreateDrawElements();
}

void GeometrySubpass::CreatePipeline() {}

void GeometrySubpass::CreateDrawElements() {
  const sg::Map& object{scene_graph_->GetObject()};
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

      const std::map<std::string, gpu::Buffer>& vertex_buffers{
          primitive.vertex_buffers};
      const gpu::Buffer& index_buffer{primitive.index_buffer};

      const std::map<std::string, sg::VertexAttribute>& vertex_attributes{
          primitive.vertex_attributes};
      u64 vertex_count{primitive.vertex_count};
      bool has_index{primitive.has_index};
      const sg::IndexAttribute& index_attribute{primitive.index_attribute};

      const sg::Material* material{primitive.material};
      const std::map<std::string, sg::Texture*>& textures{
          material->GetTextures()};

      std::vector<std::string> shader_processes;
      for (const auto& vertex_buffer : vertex_buffers) {
        std::string name{vertex_buffer.first};
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);
        shader_processes.push_back("DHAS_" + name + "_BUFFER");
      }

      for (const auto& texture : textures) {
        std::string name{texture.first};
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);
        shader_processes.push_back("DHAS_" + name + "_TEXTURE");
      }

      SPIRV spirv_vert;
      SPIRV spirv_frag;

      u64 vert_hash_value{
          asset_->GetAssetInfo().vertex.GetHashValue(shader_processes)};
      auto vert_iter{spirv_shaders_.find(vert_hash_value)};
      if (vert_iter != spirv_shaders_.end()) {
        spirv_vert = vert_iter->second;
      } else {
        spirv_vert = SPIRV{asset_->GetAssetInfo().vertex, shader_processes,
                           vk::ShaderStageFlagBits::eVertex};
        spirv_shaders_.insert(std::make_pair(vert_hash_value, spirv_vert));
      }

      u64 fragment_hash_value{
          asset_->GetAssetInfo().fragment.GetHashValue(shader_processes)};
      auto frag_iter{spirv_shaders_.find(fragment_hash_value)};
      if (frag_iter != spirv_shaders_.end()) {
        spirv_frag = frag_iter->second;
      } else {
        spirv_frag = SPIRV{asset_->GetAssetInfo().fragment, shader_processes,
                           vk::ShaderStageFlagBits::eFragment};
        spirv_shaders_.insert(std::make_pair(fragment_hash_value, spirv_frag));
      }

      std::vector<const SPIRV*> spirv_shaders{&spirv_vert, &spirv_frag};

      std::unordered_map<std::string, ShaderResource> name_shader_resources;
      std::unordered_map<u32, std::vector<ShaderResource>> set_shader_resources;

      for (const auto* spirv_shader : spirv_shaders) {
        const auto& shader_resources{spirv_shader->GetShaderResources()};
        for (const auto& shader_resource : shader_resources) {
          const std::string& name{shader_resource.name};

          auto it{name_shader_resources.find(name)};
          if (it != name_shader_resources.end()) {
            it->second.stage |= shader_resource.stage;
          } else {
            name_shader_resources.emplace(name, shader_resource);
          }
        }
      }

      for (const auto& name_shader_resource : name_shader_resources) {
        const auto& shader_resource{name_shader_resource.second};

        auto it{set_shader_resources.find(shader_resource.set)};
        if (it != set_shader_resources.end()) {
          it->second.push_back(shader_resource);
        } else {
          set_shader_resources.emplace(
              shader_resource.set,
              std::vector<ShaderResource>{shader_resource});
        }
      }

      for (const auto& set_shader_resource : set_shader_resources) {
        u32 set{set_shader_resource.first};
        const auto& shader_resources{set_shader_resource.second};

        std::vector<vk::DescriptorSetLayoutBinding> layout_bindings;
        for (const auto& shader_resource : shader_resources) {
          vk::DescriptorType descriptor_type;

          if (shader_resource.type == ShaderResourceType::kUniformBuffer) {
            descriptor_type = vk::DescriptorType::eUniformBuffer;
          } else if (shader_resource.type ==
                     ShaderResourceType::kSampledImage) {
            descriptor_type = vk::DescriptorType::eSampledImage;
          } else {
            continue;
          }

          vk::DescriptorSetLayoutBinding layout_binding{
              shader_resource.binding, descriptor_type,
              shader_resource.array_size, shader_resource.stage};

          layout_bindings.push_back(layout_binding);
        }

        vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_ci{
            {}, layout_bindings};

        const vk::raii::DescriptorSetLayout& descriptor_set_layout{
            gpu_->RequestDescriptorSetLayout(descriptor_set_layout_ci)};
      }

      draw_elements_.push_back(std::move(draw_element));
    }
  }
}

}  // namespace rd

}  // namespace luka
