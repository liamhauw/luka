// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/swapchain_subpass.h"

#include "core/log.h"

namespace luka {

namespace rd {

SwapchainSupass::SwapchainSupass(std::shared_ptr<Asset> asset,
                                 std::shared_ptr<Camera> camera,
                                 std::shared_ptr<Gpu> gpu,
                                 std::shared_ptr<SceneGraph> scene_graph,
                                 const vk::raii::RenderPass& render_pass,
                                 u32 frame_count)
    : Subpass{asset, camera, gpu, scene_graph, render_pass, frame_count} {
  CreateBindlessDescriptorSets();
  CreateDrawElements();
}

void SwapchainSupass::PushConstants(
    const vk::raii::CommandBuffer& command_buffer,
    vk::PipelineLayout pipeline_layout) {
  const glm::mat4& view{camera_->GetViewMatrix()};
  const glm::mat4& projection{camera_->GetProjectionMatrix()};
  const glm::vec3& camera_position{camera_->GetPosition()};

  PushConstantUniform push_constant_uniform{projection * view, camera_position};

  command_buffer.pushConstants<PushConstantUniform>(
      pipeline_layout, vk::ShaderStageFlagBits::eVertex, 0,
      push_constant_uniform);
}

void SwapchainSupass::CreateBindlessDescriptorSets() {
  std::vector<vk::DescriptorSetLayoutBinding> descriptor_set_layout_bindings{
      {0, vk::DescriptorType::eCombinedImageSampler, 1024,
       vk::ShaderStageFlagBits::eAll}};

  std::vector<vk::DescriptorBindingFlags> descriptor_binding_flags(
      1, vk::DescriptorBindingFlagBits::ePartiallyBound);

  vk::DescriptorSetLayoutBindingFlagsCreateInfo
      descriptor_set_layout_binding_flags_ci{descriptor_binding_flags};

  vk::DescriptorSetLayoutCreateInfo bindless_descriptor_set_layout_ci{
      vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
      descriptor_set_layout_bindings, &descriptor_set_layout_binding_flags_ci};

  bindless_descriptor_set_layout_ =
      *(RequestDescriptorSetLayout(bindless_descriptor_set_layout_ci));

  vk::DescriptorSetAllocateInfo bindless_descriptor_set_allocate_info{
      nullptr, bindless_descriptor_set_layout_};
  bindless_descriptor_sets_ = gpu_->AllocateBindlessDescriptorSets(
      bindless_descriptor_set_allocate_info);
}

void SwapchainSupass::CreateDrawElements() {
  const ast::sc::Map& object{scene_graph_->GetObjectLuka()};
  const ast::sc::Scene* scene{object.GetScene()};
  const std::vector<ast::sc::Node*>& nodes{scene->GetNodes()};

  std::queue<const ast::sc::Node*> all_nodes;
  std::unordered_map<const ast::sc::Node*, glm::mat4> node_model_matrix;

  for (const ast::sc::Node* node : nodes) {
    all_nodes.push(node);
  }

  while (!all_nodes.empty()) {
    const ast::sc::Node* cur_node{all_nodes.front()};
    all_nodes.pop();

    const std::vector<ast::sc::Node*>& cur_node_children{cur_node->GetChildren()};
    for (const ast::sc::Node* cur_node_child : cur_node_children) {
      all_nodes.push(cur_node_child);
    }

    // Model matrix.
    glm::mat4 model_matrix{cur_node->GetModelMarix()};
    const ast::sc::Node* parent_node{cur_node->GetParent()};
    while (parent_node) {
      model_matrix *= parent_node->GetModelMarix();
      parent_node = parent_node->GetParent();
    }

    // Primitives.
    const ast::sc::Mesh* mesh{cur_node->GetMesh()};
    if (!mesh) {
      continue;
    }
    const std::vector<ast::sc::Primitive>& primitives{mesh->GetPrimitives()};

    // Draw elements.
    for (const ast::sc::Primitive& primitive : primitives) {
      DrawElement draw_element{CreateDrawElement(model_matrix, primitive)};
      draw_elements_.push_back(std::move(draw_element));
    }
  }
}

DrawElement SwapchainSupass::CreateDrawElement(const glm::mat4& model_matrix,
                                               const ast::sc::Primitive& primitive) {
  DrawElement draw_element;

  // Primitive info.
  const auto& vertex_attributes{primitive.vertex_attributes};
  const auto& index_attribute{primitive.index_attribute};
  bool has_index{primitive.has_index};
  const ast::sc::Material* material{primitive.material};

  // Shaders.
  std::vector<std::string> shader_processes;
  const std::map<std::string, ast::sc::Texture*>& textures{material->GetTextures()};
  for (std::string wanted_texture : wanted_textures_) {
    auto it{textures.find(wanted_texture)};
    if (it != textures.end()) {
      std::transform(wanted_texture.begin(), wanted_texture.end(),
                     wanted_texture.begin(), ::toupper);
      shader_processes.push_back("DHAS_" + wanted_texture);
    }
  }

  bool has_position_buffer{false};
  for (const auto& vertex_buffer_attribute : vertex_attributes) {
    std::string name{vertex_buffer_attribute.first};
    if (name == "POSITION") {
      has_position_buffer = true;
    }
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);
    shader_processes.push_back("DHAS_" + name + "_BUFFER");
  }
  if (!has_position_buffer) {
    THROW("There is no position buffer.");
  }

  const SPIRV& vert_spirv{RequesetSpirv(asset_->GetVertexShader(), shader_processes,
                                        vk::ShaderStageFlagBits::eVertex)};
  const SPIRV& frag_spirv{RequesetSpirv(asset_->GetFragmentShader(), shader_processes,
                                        vk::ShaderStageFlagBits::eFragment)};

  // Pipeline layout.
  std::vector<const SPIRV*> spirv_shaders{&vert_spirv, &frag_spirv};

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

    if (set == 0) {
      continue;
    }

    const auto& shader_resources{set_shader_resource.second};

    std::vector<vk::DescriptorSetLayoutBinding> layout_bindings;
    for (const auto& shader_resource : shader_resources) {
      vk::DescriptorType descriptor_type;

      if (shader_resource.type == ShaderResourceType::kUniformBuffer) {
        descriptor_type = vk::DescriptorType::eUniformBuffer;
      } else {
        continue;
      }

      vk::DescriptorSetLayoutBinding layout_binding{
          shader_resource.binding, descriptor_type, shader_resource.array_size,
          shader_resource.stage};

      layout_bindings.push_back(layout_binding);
    }

    if (layout_bindings.empty()) {
      continue;
    }

    vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_ci{{},
                                                               layout_bindings};

    const vk::raii::DescriptorSetLayout& descriptor_set_layout{
        RequestDescriptorSetLayout(descriptor_set_layout_ci)};

    set_layouts.push_back(*descriptor_set_layout);
  }

  std::vector<vk::DescriptorSetLayout> set_layouts_with_bindless{
      bindless_descriptor_set_layout_};
  for (vk::DescriptorSetLayout set_layout : set_layouts) {
    set_layouts_with_bindless.push_back(set_layout);
  }

  vk::PipelineLayoutCreateInfo pipeline_layout_ci{
      {}, set_layouts_with_bindless, push_constant_ranges};

  const vk::raii::PipelineLayout& pipeline_layout{
      RequestPipelineLayout(pipeline_layout_ci)};

  draw_element.pipeline_layout = *pipeline_layout;

  // Pipeline.
  std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_cis;
  u64 pipeline_hash_value{0};
  for (const auto* spirv_shader : spirv_shaders) {
    u64 shader_module_hash_value{spirv_shader->GetHashValue()};
    HashCombine(pipeline_hash_value, shader_module_hash_value);

    const std::vector<u32>& spirv{spirv_shader->GetSpirv()};

    vk::ShaderModuleCreateInfo shader_module_ci{
        {}, spirv.size() * 4, spirv.data()};
    const vk::raii::ShaderModule& shader_module{
        RequestShaderModule(shader_module_ci, shader_module_hash_value)};

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
    const ast::sc::VertexAttribute& vertex_attribute{vertex_buffer_attribute.second};

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
      vk::FrontFace::eCounterClockwise,
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
      RequestPipeline(graphics_pipeline_create_info, pipeline_hash_value)};
  draw_element.pipeline = *pipeline;

  // Descriptor set.
  for (u32 i{0}; i < frame_count_; ++i) {
    vk::DescriptorSetAllocateInfo descriptor_set_allocate_info{{}, set_layouts};
    vk::raii::DescriptorSets descriptor_sets{
        gpu_->AllocateNormalDescriptorSets(descriptor_set_allocate_info)};

    std::vector<vk::WriteDescriptorSet> write_descriptor_sets;
    std::vector<vk::DescriptorImageInfo> image_infos;
    std::vector<vk::DescriptorBufferInfo> buffer_infos;

    // 1. Combined image samplers.
    glm::uvec4 image_indices{0};
    u32 idx{0};
    for (const auto& wanted_texture : wanted_textures_) {
      auto it{name_shader_resources.find("global_images")};
      auto it1{textures.find(wanted_texture)};
      if (it != name_shader_resources.end() && it1 != textures.end()) {
        const ShaderResource& shader_resource{it->second};

        ast::sc::Texture* tex{it1->second};

        const vk::raii::ImageView& image_view{tex->GetImage()->GetImageView()};
        const vk::raii::Sampler& sampler{tex->GetSampler()->GetSampler()};

        vk::DescriptorImageInfo descriptor_image_info{
            *sampler, *image_view, vk::ImageLayout::eShaderReadOnlyOptimal};
        image_infos.push_back(std::move(descriptor_image_info));

        image_indices[idx] = global_image_index_++;
        vk::WriteDescriptorSet write_descriptor_set{
            *(bindless_descriptor_sets_[0]), shader_resource.binding,
            image_indices[idx], vk::DescriptorType::eCombinedImageSampler,
            image_infos.back()};
        ++idx;
        write_descriptor_sets.push_back(write_descriptor_set);
      }
    }

    // 2. Uniform buffers.
    for (const auto& set_shader_resource : set_shader_resources) {
      u32 set{set_shader_resource.first};
      const auto& shader_resources{set_shader_resource.second};
      for (const auto& shader_resource : shader_resources) {
        if (shader_resource.type == ShaderResourceType::kUniformBuffer) {
          if (shader_resource.name == "DrawElementUniform") {
            DrawElementUniform draw_element_uniform{
                model_matrix, material->GetBaseColorFactor(), image_indices};

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
            buffer_infos.push_back(std::move(descriptor_buffer_info));

            vk::WriteDescriptorSet write_descriptor_set{
                *(descriptor_sets[shader_resource.set - 1]),
                shader_resource.binding,
                0,
                vk::DescriptorType::eUniformBuffer,
                nullptr,
                buffer_infos.back()};

            draw_element.draw_element_uniforms.push_back(
                std::move(draw_element_uniform));
            draw_element.draw_element_uniform_buffers.push_back(
                std::move(draw_element_uniform_buffer));

            write_descriptor_sets.push_back(write_descriptor_set);
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
