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
                                 std::shared_ptr<SceneGraph> scene_graph,
                                 const vk::raii::RenderPass& render_pass,
                                 u32 frame_count)
    : Subpass{asset, gpu, scene_graph, render_pass, frame_count} {
  CreateDrawElements();
}

void GeometrySubpass::CreateDrawElements() {
  const sg::Map& object{scene_graph_->GetObject()};
  const sg::Scene* scene{object.GetScene()};
  const std::vector<sg::Node*>& nodes{scene->GetNodes()};

  std::queue<const sg::Node*> all_nodes;
  std::unordered_map<const sg::Node*, glm::mat4> node_model_matrix;

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

    // Model matrix.
    glm::mat4 model_matrix{cur_node->GetModelMarix()};
    const sg::Node* parent_node{cur_node->GetParent()};
    while (parent_node) {
      model_matrix *= parent_node->GetModelMarix();
      parent_node = parent_node->GetParent();
    }

    // Primitives.
    const sg::Mesh* mesh{cur_node->GetMesh()};
    if (!mesh) {
      continue;
    }
    const std::vector<sg::Primitive>& primitives{mesh->GetPrimitives()};

    // Draw elements.
    for (const sg::Primitive& primitive : primitives) {
      DrawElement draw_element{CreateDrawElement(model_matrix, primitive)};
      draw_elements_.push_back(std::move(draw_element));
    }
  }
}

DrawElement GeometrySubpass::CreateDrawElement(const glm::mat4& model_matrix,
                                               const sg::Primitive& primitive) {
  DrawElement draw_element;

  // Primitive info.
  const auto& vertex_attributes{primitive.vertex_attributes};
  const auto& index_attribute{primitive.index_attribute};
  bool has_index{primitive.has_index};
  const sg::Material* material{primitive.material};

  // Shaders.
  std::vector<std::string> shader_processes;
  for (const auto& vertex_buffer_attribute : vertex_attributes) {
    std::string name{vertex_buffer_attribute.first};
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);
    shader_processes.push_back("DHAS_" + name + "_BUFFER");
  }

  const std::map<std::string, sg::Texture*>& textures{material->GetTextures()};
  for (const auto& texture : textures) {
    std::string name{texture.first};
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);
    shader_processes.push_back("DHAS_" + name);
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
    vert_iter = spirv_shaders_.emplace(vert_hash_value, spirv_vert).first;
  }

  u64 fragment_hash_value{
      asset_->GetAssetInfo().fragment.GetHashValue(shader_processes)};
  auto frag_iter{spirv_shaders_.find(fragment_hash_value)};
  if (frag_iter != spirv_shaders_.end()) {
    spirv_frag = frag_iter->second;
  } else {
    spirv_frag = SPIRV{asset_->GetAssetInfo().fragment, shader_processes,
                       vk::ShaderStageFlagBits::eFragment};
    frag_iter = spirv_shaders_.emplace(fragment_hash_value, spirv_frag).first;
  }

  // Pipeline layout.
  std::vector<const SPIRV*> spirv_shaders{&(vert_iter->second),
                                          &(frag_iter->second)};

  std::unordered_map<std::string, ShaderResource> name_shader_resources;

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

  std::unordered_map<u32, std::vector<ShaderResource>> set_shader_resources;

  std::vector<vk::PushConstantRange> push_constant_ranges;

  for (const auto& name_shader_resource : name_shader_resources) {
    const auto& shader_resource{name_shader_resource.second};

    if (shader_resource.type == ShaderResourceType::kUniformBuffer ||
        shader_resource.type == ShaderResourceType::kCombinedImageSampler) {
      auto it{set_shader_resources.find(shader_resource.set)};
      if (it != set_shader_resources.end()) {
        it->second.push_back(shader_resource);
      } else {
        set_shader_resources.emplace(
            shader_resource.set, std::vector<ShaderResource>{shader_resource});
      }
    } else if (shader_resource.type ==
               ShaderResourceType::kPushConstantBuffer) {
      push_constant_ranges.emplace_back(
          shader_resource.stage, shader_resource.offset, shader_resource.size);
    }
  }

  std::vector<vk::DescriptorSetLayout> set_layouts;

  for (const auto& set_shader_resource : set_shader_resources) {
    u32 set{set_shader_resource.first};
    const auto& shader_resources{set_shader_resource.second};

    std::vector<vk::DescriptorSetLayoutBinding> layout_bindings;
    for (const auto& shader_resource : shader_resources) {
      vk::DescriptorType descriptor_type;

      if (shader_resource.type == ShaderResourceType::kUniformBuffer) {
        descriptor_type = vk::DescriptorType::eUniformBuffer;
      } else if (shader_resource.type ==
                 ShaderResourceType::kCombinedImageSampler) {
        descriptor_type = vk::DescriptorType::eCombinedImageSampler;
      } else {
        continue;
      }

      vk::DescriptorSetLayoutBinding layout_binding{
          shader_resource.binding, descriptor_type, shader_resource.array_size,
          shader_resource.stage};

      layout_bindings.push_back(layout_binding);
    }

    vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_ci{{},
                                                               layout_bindings};

    const vk::raii::DescriptorSetLayout& descriptor_set_layout{
        gpu_->RequestDescriptorSetLayout(descriptor_set_layout_ci)};

    set_layouts.push_back(*descriptor_set_layout);
  }

  vk::PipelineLayoutCreateInfo pipeline_layout_ci{
      {}, set_layouts, push_constant_ranges};

  const vk::raii::PipelineLayout& pipeline_layout{
      gpu_->RequestPipelineLayout(pipeline_layout_ci)};

  draw_element.pipeline_layout = *pipeline_layout;

  // Pipeline.
  std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_cis;
  for (const auto* spirv_shader : spirv_shaders) {
    const std::vector<u32>& spirv{spirv_shader->GetSpirv()};

    vk::ShaderModuleCreateInfo shader_module_ci{
        {}, spirv.size() * 4, spirv.data()};
    const vk::raii::ShaderModule& shader_module{
        gpu_->RequestShaderModule(shader_module_ci)};

    vk::PipelineShaderStageCreateInfo shader_stage_ci{
        {}, spirv_shader->GetStage(), *shader_module, "main", nullptr};

    shader_stage_cis.push_back(std::move(shader_stage_ci));
  }

  std::vector<vk::VertexInputBindingDescription>
      vertex_input_binding_descriptions;
  std::vector<vk::VertexInputAttributeDescription>
      vertex_input_attribute_descriptions;

  for (const auto& vertex_buffer_attribute : vertex_attributes) {
    std::string name{vertex_buffer_attribute.first};
    const sg::VertexAttribute& vertex_attribute{vertex_buffer_attribute.second};

    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    auto it{name_shader_resources.find(name)};
    if (it == name_shader_resources.end()) {
      continue;
    }

    const auto& shader_resource{it->second};

    vk::VertexInputBindingDescription vertex_input_binding_description{
        shader_resource.location, vertex_attribute.stride};
    vertex_input_binding_descriptions.push_back(
        std::move(vertex_input_binding_description));

    vk::VertexInputAttributeDescription vertex_input_attribute_description{
        shader_resource.location, shader_resource.location,
        vertex_attribute.format, vertex_attribute.offset};
    vertex_input_attribute_descriptions.push_back(
        std::move(vertex_input_attribute_description));

    draw_element.location_vertex_attributes.emplace(shader_resource.location,
                                                    &vertex_attribute);
    if (draw_element.vertex_count == 0) {
      draw_element.vertex_count = vertex_attribute.count;
    }
  }

  if (has_index) {
    draw_element.has_index = true;
    draw_element.index_attribute = &index_attribute;
  }

  vk::PipelineVertexInputStateCreateInfo vertex_input_state_ci{
      {},
      vertex_input_binding_descriptions,
      vertex_input_attribute_descriptions};

  vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_ci{
      {}, vk::PrimitiveTopology::eTriangleList};

  vk::PipelineViewportStateCreateInfo viewport_state_ci{
      {}, 1, nullptr, 1, nullptr};

  vk::PipelineRasterizationStateCreateInfo rasterization_state_ci{
      {},
      VK_FALSE,
      VK_FALSE,
      vk::PolygonMode::eFill,
      vk::CullModeFlagBits::eBack,
      vk::FrontFace::eClockwise,
      VK_FALSE,
      0.0F,
      0.0F,
      0.0F,
      1.0F};

  vk::PipelineMultisampleStateCreateInfo multisample_state_ci{
      {}, vk::SampleCountFlagBits::e1};

  vk::PipelineDepthStencilStateCreateInfo depth_stencil_state_ci{
      {}, VK_TRUE, VK_TRUE, vk::CompareOp::eLess, VK_FALSE, VK_FALSE,
  };

  vk::ColorComponentFlags color_component_flags{
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};
  vk::PipelineColorBlendAttachmentState color_blend_attachment_state{
      VK_FALSE,          vk::BlendFactor::eZero, vk::BlendFactor::eZero,
      vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero,
      vk::BlendOp::eAdd, color_component_flags};

  vk::PipelineColorBlendStateCreateInfo color_blend_state_ci{
      {},
      VK_FALSE,
      vk::LogicOp::eCopy,
      color_blend_attachment_state,
      {{0.0F, 0.0F, 0.0F, 0.0F}}};

  std::array<vk::DynamicState, 2> dynamic_states{vk::DynamicState::eViewport,
                                                 vk::DynamicState::eScissor};
  vk::PipelineDynamicStateCreateInfo dynamic_state_create_info{{},
                                                               dynamic_states};

  vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info{
      {},
      shader_stage_cis,
      &vertex_input_state_ci,
      &input_assembly_state_ci,
      nullptr,
      &viewport_state_ci,
      &rasterization_state_ci,
      &multisample_state_ci,
      &depth_stencil_state_ci,
      &color_blend_state_ci,
      &dynamic_state_create_info,
      *pipeline_layout,
      render_pass_};

  const vk::raii::Pipeline& pipeline{
      gpu_->RequestPipeline(graphics_pipeline_create_info)};
  draw_element.pipeline = *pipeline;

  // Descriptor set.
  for (u32 i{0}; i < frame_count_; ++i) {
    vk::DescriptorSetAllocateInfo descriptor_set_allocate_info{{}, set_layouts};
    vk::raii::DescriptorSets descriptor_sets{
        gpu_->AllocateDescriptorSets(descriptor_set_allocate_info)};

    std::vector<vk::WriteDescriptorSet> write_descriptor_sets;

    // 1. Combined image samplers.
    for (const auto& texture : textures) {
      const std::string& name{texture.first};

      auto it{name_shader_resources.find(name)};
      if (it != name_shader_resources.end()) {
        const ShaderResource& shader_resource{it->second};

        sg::Texture* tex{texture.second};

        const vk::raii::ImageView& image_view{tex->GetImage()->GetImageView()};
        const vk::raii::Sampler& sampler{tex->GetSampler()->GetSampler()};

        vk::DescriptorImageInfo descriptor_image_info{
            *sampler, *image_view, vk::ImageLayout::eShaderReadOnlyOptimal};

        vk::WriteDescriptorSet write_descriptor_set{
            *(descriptor_sets[shader_resource.set]), shader_resource.binding, 0,
            vk::DescriptorType::eCombinedImageSampler, descriptor_image_info};

        write_descriptor_sets.push_back(std::move(write_descriptor_set));
      }
    }

    // 2. Uniform buffers.
    for (const auto& set_shader_resource : set_shader_resources) {
      u32 set{set_shader_resource.first};
      const auto& shader_resources{set_shader_resource.second};
      for (const auto& shader_resource : shader_resources) {
        if (shader_resource.type == ShaderResourceType::kUniformBuffer) {
          if (shader_resource.name == "GlobalUniform") {
            vk::DescriptorBufferInfo descriptor_buffer_info{
                *(global_uniform_buffers_[i]), 0, sizeof(GlobalUniform)};

            vk::WriteDescriptorSet write_descriptor_set{
                *(descriptor_sets[shader_resource.set]),
                shader_resource.binding,
                0,
                vk::DescriptorType::eUniformBuffer,
                nullptr,
                descriptor_buffer_info};

            write_descriptor_sets.push_back(std::move(write_descriptor_set));
          } else if (shader_resource.name == "DrawElementUniform") {
            DrawElementUniform draw_element_uniform{model_matrix};

            vk::BufferCreateInfo uniform_buffer_ci{
                {},
                sizeof(DrawElementUniform),
                vk::BufferUsageFlagBits::eUniformBuffer,
                vk::SharingMode::eExclusive};

            gpu::Buffer draw_element_uniform_buffer{
                gpu_->CreateBuffer(uniform_buffer_ci, &draw_element_uniform,
                                   false, "draw_element_uniform")};

            vk::DescriptorBufferInfo descriptor_buffer_info{
                *draw_element_uniform_buffer, 0, sizeof(DrawElementUniform)};

            vk::WriteDescriptorSet write_descriptor_set{
                *(descriptor_sets[shader_resource.set]),
                shader_resource.binding,
                0,
                vk::DescriptorType::eUniformBuffer,
                nullptr,
                descriptor_buffer_info};

            draw_element.draw_element_uniforms.push_back(
                std::move(draw_element_uniform));
            draw_element.draw_element_uniform_buffers.push_back(
                std::move(draw_element_uniform_buffer));

            write_descriptor_sets.push_back(std::move(write_descriptor_set));
          }
        }
      }
    }

    gpu_->UpdateDescriptorSets(write_descriptor_sets);
    draw_element.descriptor_sets.push_back(std::move(descriptor_sets));
  }

  return draw_element;
}

}  // namespace rd

}  // namespace luka
