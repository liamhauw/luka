// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "rendering/graphics/subpass.h"

#undef MemoryBarrier
#include <vulkan/vulkan_hash.hpp>

#include "core/log.h"
#include "core/util.h"
#include "resource/config/generated/root_path.h"

namespace luka {

namespace gs {

Subpass::Subpass(
    std::shared_ptr<Gpu> gpu, std::shared_ptr<Asset> asset,
    std::shared_ptr<Camera> camera, u32 frame_count, vk::RenderPass render_pass,
    const std::vector<std::vector<vk::raii::ImageView>>& attachment_image_views,
    u32 color_attachment_count, const std::vector<ast::Subpass>& ast_subpasses,
    u32 subpass_index,
    std::vector<std::unordered_map<std::string, vk::ImageView>>&
        shared_image_views)
    : gpu_{gpu},
      asset_{asset},
      camera_{camera},
      frame_count_{frame_count},
      render_pass_{render_pass},
      attachment_image_views_{&attachment_image_views},
      color_attachment_count_{color_attachment_count},
      ast_subpasses_{&ast_subpasses},
      subpass_index_{subpass_index},
      shared_image_views_{&shared_image_views},
      ast_subpass_{&(*ast_subpasses_)[subpass_index_]},
      name_{ast_subpass_->name},
      scenes_{&(ast_subpass_->scenes)},
      shaders_{&(ast_subpass_->shaders)},
      has_primitive_{!scenes_->empty()} {
  CreateSubpassAndBindlessDescriptorSets();
  CreateDrawElements();
}

void Subpass::Resize(const std::vector<std::vector<vk::raii::ImageView>>&
                         attachment_image_views) {
  if (!need_resize_) {
    return;
  }
  attachment_image_views_ = &attachment_image_views;
  global_sampler_index_ = 0;
  global_image_index_ = 0;
  CreateDrawElements();
}

void Subpass::Update(u32 frame_index) {
  const glm::mat4& view{camera_->GetViewMatrix()};
  const glm::mat4& projection{camera_->GetProjectionMatrix()};
  const glm::vec3& camera_position{camera_->GetPosition()};

  glm::mat4 pv{projection * view};
  glm::mat4 inverse_vp{glm::inverse(pv)};

  subpass_uniforms_[frame_index] =
      SubpassUniform{pv, inverse_vp, glm::vec4{camera_position, 1.0F},
                     glm::vec4{5.0, 5.0, 5.0, 1.0F}};
  void* mapped{subpass_uniform_buffers_[frame_index].Map()};
  memcpy(mapped,
         reinterpret_cast<const void*>(&(subpass_uniforms_[frame_index])),
         sizeof(SubpassUniform));
}

const std::string& Subpass::GetName() const { return name_; }

const std::vector<DrawElement>& Subpass::GetDrawElements() const {
  return draw_elements_;
}

bool Subpass::HasPushConstant() const { return has_push_constant_; }

void Subpass::PushConstants(const vk::raii::CommandBuffer& command_buffer,
                            vk::PipelineLayout pipeline_layout) const {}

const vk::raii::DescriptorSet& Subpass::GetSubpassDescriptorSet(
    u32 frame_index) const {
  return subpass_descriptor_sets_[frame_index];
}

const vk::raii::DescriptorSet& Subpass::GetBindlessDescriptorSet() const {
  return bindless_descriptor_set_;
}

void Subpass::CreateSubpassAndBindlessDescriptorSets() {
  // Subpass.
  {
    std::vector<vk::DescriptorSetLayoutBinding> bindings{
        {0, vk::DescriptorType::eUniformBuffer, 1,
         vk::ShaderStageFlagBits::eAll}};
    vk::DescriptorSetLayoutCreateInfo subpass_descriptor_set_layout_ci{
        {}, bindings};

    subpass_descriptor_set_layout_ = gpu_->CreateDescriptorSetLayout(
        subpass_descriptor_set_layout_ci, name_ + "_subpass");

    std::vector<vk::DescriptorSetLayout> subpass_descriptor_set_layouts(
        frame_count_, *subpass_descriptor_set_layout_);
    vk::DescriptorSetAllocateInfo subpass_descriptor_set_allocate_info{
        nullptr, subpass_descriptor_set_layouts};
    subpass_descriptor_sets_ = gpu_->AllocateNormalDescriptorSets(
        subpass_descriptor_set_allocate_info, name_ + "_subpass");

    const glm::mat4& view{camera_->GetViewMatrix()};
    const glm::mat4& projection{camera_->GetProjectionMatrix()};
    const glm::vec3& camera_position{camera_->GetPosition()};

    glm::mat4 pv{projection * view};
    glm::mat4 inverse_vp{glm::inverse(pv)};

    for (u32 i{0}; i < frame_count_; ++i) {
      SubpassUniform subpass_uniform{pv, inverse_vp,
                                     glm::vec4{camera_position, 1.0F},
                                     glm::vec4{5.0, 5.0, 5.0, 1.0F}};

      vk::BufferCreateInfo uniform_buffer_ci{
          {},
          sizeof(SubpassUniform),
          vk::BufferUsageFlagBits::eUniformBuffer,
          vk::SharingMode::eExclusive};

      gpu::Buffer subpass_uniform_buffer{gpu_->CreateBuffer(
          uniform_buffer_ci, &subpass_uniform, true, "subpass_uniform")};

      vk::DescriptorBufferInfo descriptor_buffer_info{
          *subpass_uniform_buffer, 0, sizeof(SubpassUniform)};

      vk::WriteDescriptorSet write_descriptor_set{
          *subpass_descriptor_sets_[i],       0,       0,
          vk::DescriptorType::eUniformBuffer, nullptr, descriptor_buffer_info};

      subpass_uniforms_.push_back(std::move(subpass_uniform));
      subpass_uniform_buffers_.push_back(std::move(subpass_uniform_buffer));

      std::vector<vk::WriteDescriptorSet> write_descriptor_sets;

      write_descriptor_sets.push_back(write_descriptor_set);

      gpu_->UpdateDescriptorSets(write_descriptor_sets);
    }
  }

  // Bindless.
  {
    std::vector<vk::DescriptorSetLayoutBinding> bindings{
        {0, vk::DescriptorType::eSampler, 8, vk::ShaderStageFlagBits::eAll},
        {1, vk::DescriptorType::eSampledImage, 32,
         vk::ShaderStageFlagBits::eAll}};

    std::vector<vk::DescriptorBindingFlags> binding_flags(
        2, vk::DescriptorBindingFlagBits::ePartiallyBound);

    vk::DescriptorSetLayoutBindingFlagsCreateInfo binding_flags_ci{
        binding_flags};

    vk::DescriptorSetLayoutCreateInfo bindless_descriptor_set_layout_ci{
        vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool, bindings,
        &binding_flags_ci};
    bindless_descriptor_set_layout_ = gpu_->CreateDescriptorSetLayout(
        bindless_descriptor_set_layout_ci, name_ + "_bindless");

    vk::DescriptorSetAllocateInfo bindless_descriptor_set_allocate_info{
        nullptr, *bindless_descriptor_set_layout_};
    bindless_descriptor_set_ = gpu_->AllocateBindlessDescriptorSet(
        bindless_descriptor_set_allocate_info, name_ + "_bindless");
  }
}

void Subpass::CreateDrawElements() {
  draw_elements_.clear();

  if (!has_primitive_) {
    draw_elements_.push_back(CreateDrawElement());
    return;
  }

  for (u32 scene_index : *scenes_) {
    const ast::sc::Scene* scene{asset_->GetScene(scene_index).GetScene()};
    const std::vector<ast::sc::Node*>& nodes{scene->GetNodes()};

    std::queue<const ast::sc::Node*> all_nodes;
    std::unordered_map<const ast::sc::Node*, glm::mat4> node_model_matrix;

    for (const ast::sc::Node* node : nodes) {
      all_nodes.push(node);
    }

    while (!all_nodes.empty()) {
      const ast::sc::Node* cur_node{all_nodes.front()};
      all_nodes.pop();

      const std::vector<ast::sc::Node*>& cur_node_children{
          cur_node->GetChildren()};
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
      for (u32 i{0}; i < primitives.size(); ++i) {
        DrawElement draw_element{
            CreateDrawElement(model_matrix, primitives[i], i)};
        draw_elements_.push_back(std::move(draw_element));
      }
    }
  }
}

DrawElement Subpass::CreateDrawElement(const glm::mat4& model_matrix,
                                       const ast::sc::Primitive& primitive,
                                       u32 primitive_index) {
  DrawElement draw_element;

  draw_element.has_primitive = has_primitive_;

  // Shaders.
  std::vector<std::string> wanted_textures{"base_color_texture"};
  std::vector<std::string> shader_processes;
  if (has_primitive_) {
    const std::map<std::string, ast::sc::Texture*>& textures{
        primitive.material->GetTextures()};
    for (std::string wanted_texture : wanted_textures) {
      auto it{textures.find(wanted_texture)};
      if (it != textures.end()) {
        std::transform(wanted_texture.begin(), wanted_texture.end(),
                       wanted_texture.begin(), ::toupper);
        shader_processes.push_back("DHAS_" + wanted_texture);
      }
    }

    bool has_position_buffer{false};
    for (const auto& vertex_buffer_attribute : primitive.vertex_attributes) {
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
  }

  auto vi{shaders_->find(vk::ShaderStageFlagBits::eVertex)};
  if (vi == shaders_->end()) {
    THROW("There is no vertex shader");
  }
  auto fi{shaders_->find(vk::ShaderStageFlagBits::eFragment)};
  if (fi == shaders_->end()) {
    THROW("There is no fragment shader");
  }
  const SPIRV& vert_spirv{
      RequestSpirv(asset_->GetShader(vi->second), shader_processes,
                   vk::ShaderStageFlagBits::eVertex),
  };
  const SPIRV& frag_spirv{RequestSpirv(asset_->GetShader(fi->second),
                                       shader_processes,
                                       vk::ShaderStageFlagBits::eFragment)};

  std::vector<const SPIRV*> spirv_shaders{&vert_spirv, &frag_spirv};

  // Pipeline layout.
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
  std::vector<u32> sorted_sets;

  for (const auto& name_shader_resource : name_shader_resources) {
    const auto& shader_resource{name_shader_resource.second};

    if (shader_resource.type == ShaderResourceType::kSampler ||
        shader_resource.type == ShaderResourceType::kCombinedImageSampler ||
        shader_resource.type == ShaderResourceType::kSampledImage ||
        shader_resource.type == ShaderResourceType::kUniformBuffer ||
        shader_resource.type == ShaderResourceType::kInputAttachment) {
      auto it{set_shader_resources.find(shader_resource.set)};
      if (it != set_shader_resources.end()) {
        it->second.push_back(shader_resource);
      } else {
        set_shader_resources.emplace(
            shader_resource.set, std::vector<ShaderResource>{shader_resource});
        sorted_sets.push_back(shader_resource.set);
      }
    } else if (shader_resource.type ==
               ShaderResourceType::kPushConstantBuffer) {
      push_constant_ranges.emplace_back(
          shader_resource.stage, shader_resource.offset, shader_resource.size);
    }
  }

  std::sort(sorted_sets.begin(), sorted_sets.end());

  std::vector<vk::DescriptorSetLayout> set_layouts{
      *subpass_descriptor_set_layout_, *bindless_descriptor_set_layout_};

  for (u32 set : sorted_sets) {
    if (set <= 1) {
      continue;
    }

    const auto& shader_resources{set_shader_resources.at(set)};
    const vk::raii::DescriptorSetLayout* descriptor_set_layout{nullptr};

    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    for (const auto& shader_resource : shader_resources) {
      vk::DescriptorType descriptor_type;

      if (shader_resource.type == ShaderResourceType::kUniformBuffer) {
        descriptor_type = vk::DescriptorType::eUniformBuffer;
      } else if (shader_resource.type == ShaderResourceType::kInputAttachment) {
        descriptor_type = vk::DescriptorType::eInputAttachment;
      } else if (shader_resource.type ==
                 ShaderResourceType::kCombinedImageSampler) {
        descriptor_type = vk::DescriptorType::eCombinedImageSampler;
      } else {
        continue;
      }

      vk::DescriptorSetLayoutBinding binding{
          shader_resource.binding, descriptor_type, 1, shader_resource.stage};

      bindings.push_back(binding);
    }

    if (bindings.empty()) {
      continue;
    }

    vk::DescriptorSetLayoutCreateInfo draw_element_descriptor_set_layout_ci{
        {}, bindings};

    const vk::raii::DescriptorSetLayout* draw_element_descriptor_set_layout =
        &(RequestDescriptorSetLayout(draw_element_descriptor_set_layout_ci,
                                     name_ + "_draw_element"));

    descriptor_set_layout = draw_element_descriptor_set_layout;

    for (u32 i{0}; i < frame_count_; ++i) {
      vk::DescriptorSetAllocateInfo draw_element_descriptor_set_allocate_info{
          nullptr, **draw_element_descriptor_set_layout};

      vk::raii::DescriptorSets draw_element_descriptor_sets{
          gpu_->AllocateNormalDescriptorSets(
              draw_element_descriptor_set_allocate_info,
              name_ + "_draw_element", primitive_index)};

      draw_element.descriptor_sets.push_back(
          std::move(draw_element_descriptor_sets));
    }

    draw_element.has_descriptor_set = true;

    set_layouts.push_back(**descriptor_set_layout);
  }

  vk::PipelineLayoutCreateInfo pipeline_layout_ci;

  if (!set_layouts.empty()) {
    pipeline_layout_ci.setSetLayouts(set_layouts);
  }

  if (!push_constant_ranges.empty()) {
    has_push_constant_ = true;
    pipeline_layout_ci.setPushConstantRanges(push_constant_ranges);
  }

  const vk::raii::PipelineLayout& pipeline_layout{
      RequestPipelineLayout(pipeline_layout_ci, name_)};

  draw_element.pipeline_layout = &pipeline_layout;

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
        RequestShaderModule(shader_module_ci, shader_module_hash_value, name_)};

    vk::PipelineShaderStageCreateInfo shader_stage_ci{
        {}, spirv_shader->GetStage(), *shader_module, "main", nullptr};

    shader_stage_cis.push_back(std::move(shader_stage_ci));
  }

  vk::PipelineVertexInputStateCreateInfo vertex_input_state_ci;
  std::vector<vk::VertexInputBindingDescription>
      vertex_input_binding_descriptions;
  std::vector<vk::VertexInputAttributeDescription>
      vertex_input_attribute_descriptions;
  if (has_primitive_) {
    std::map<u32, const ast::sc::VertexAttribute*> vertex_location_attributes;
    for (const auto& vertex_buffer_attribute : primitive.vertex_attributes) {
      std::string name{vertex_buffer_attribute.first};
      const ast::sc::VertexAttribute& vertex_attribute{
          vertex_buffer_attribute.second};

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

      vertex_location_attributes.emplace(shader_resource.location,
                                         &vertex_attribute);
      if (draw_element.vertex_count == 0) {
        draw_element.vertex_count = vertex_attribute.count;
      }
    }

    std::vector<u32> locations;
    for (const auto& vertex_location_attribute : vertex_location_attributes) {
      locations.push_back(vertex_location_attribute.first);
    }
    std::vector<std::vector<u32>> splited_locations{SplitVector(locations)};

    for (const auto& splited : splited_locations) {
      std::vector<vk::Buffer> buffers;
      std::vector<u64> offsets;
      for (const auto& location : splited) {
        const auto* vertex_attribute{vertex_location_attributes.at(location)};
        buffers.push_back(*(vertex_attribute->buffer));
        offsets.push_back(vertex_attribute->offset);
      }
      draw_element.vertex_infos.push_back(
          DrawElmentVertexInfo{splited.front(), buffers, offsets});
    }

    if (primitive.has_index) {
      draw_element.has_index = true;
      draw_element.index_attribute = &(primitive.index_attribute);
    }
    vertex_input_state_ci.setVertexBindingDescriptions(
        vertex_input_binding_descriptions);
    vertex_input_state_ci.setVertexAttributeDescriptions(
        vertex_input_attribute_descriptions);
  }

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

  if (!has_primitive_) {
    rasterization_state_ci.cullMode = vk::CullModeFlagBits::eNone;
  }

  vk::PipelineMultisampleStateCreateInfo multisample_state_ci{
      {}, vk::SampleCountFlagBits::e1};

  vk::PipelineDepthStencilStateCreateInfo depth_stencil_state_ci{
      {}, VK_TRUE, VK_TRUE, vk::CompareOp::eLess, VK_FALSE, VK_FALSE,
  };

  vk::ColorComponentFlags color_component_flags{
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};
  std::vector<vk::PipelineColorBlendAttachmentState>
      color_blend_attachment_states(
          color_attachment_count_,
          vk::PipelineColorBlendAttachmentState{
              VK_FALSE, vk::BlendFactor::eZero, vk::BlendFactor::eZero,
              vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero,
              vk::BlendOp::eAdd, color_component_flags});

  vk::PipelineColorBlendStateCreateInfo color_blend_state_ci{
      {},
      VK_FALSE,
      vk::LogicOp::eCopy,
      color_blend_attachment_states,
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
      render_pass_,
      subpass_index_};

  const vk::raii::Pipeline& pipeline{RequestPipeline(
      graphics_pipeline_create_info, pipeline_hash_value, name_)};
  draw_element.pipeline = &pipeline;

  // Update descriptor sets.
  std::vector<vk::WriteDescriptorSet> write_descriptor_sets;
  std::vector<vk::DescriptorImageInfo> sampler_infos;
  std::vector<vk::DescriptorImageInfo> image_infos;
  std::vector<vk::DescriptorBufferInfo> buffer_infos;

  image_infos.reserve(100);
  buffer_infos.reserve(100);

  // Bindless.
  glm::uvec4 sampler_indices{0};
  glm::uvec4 image_indices{0};
  if (has_primitive_) {
    const std::map<std::string, ast::sc::Texture*>& textures{
        primitive.material->GetTextures()};
    u32 idx{0};
    for (const auto& wanted_texture : wanted_textures) {
      auto global_samplers_it{name_shader_resources.find("global_samplers")};
      auto global_images_it{name_shader_resources.find("global_images")};
      auto wanted_texture_it{textures.find(wanted_texture)};
      if (global_samplers_it != name_shader_resources.end() &&
          global_images_it != name_shader_resources.end() &&
          wanted_texture_it != textures.end()) {
        const ShaderResource& global_samplers_shader_resource{
            global_samplers_it->second};
        const ShaderResource& global_images_shader_resource{
            global_images_it->second};
        ast::sc::Texture* tex{wanted_texture_it->second};

        const ast::sc::Sampler* ast_sampler{tex->GetSampler()};
        u64 sampler_hash_value{0};
        HashCombine(sampler_hash_value, ast_sampler);
        auto it2{sampler_indices_.find(sampler_hash_value)};
        if (it2 != sampler_indices_.end()) {
          sampler_indices_[idx] = it2->second;
        } else {
          const vk::raii::Sampler& sampler{ast_sampler->GetSampler()};

          vk::DescriptorImageInfo descriptor_sampler_info{*sampler};
          sampler_infos.push_back(std::move(descriptor_sampler_info));

          sampler_indices[idx] = global_sampler_index_++;

          vk::WriteDescriptorSet write_descriptor_set{
              *(bindless_descriptor_set_),
              global_samplers_shader_resource.binding, sampler_indices[idx],
              vk::DescriptorType::eSampler, sampler_infos.back()};

          write_descriptor_sets.push_back(write_descriptor_set);

          sampler_indices_.emplace(sampler_hash_value, sampler_indices[idx]);
        }

        const ast::sc::Image* ast_image{tex->GetImage()};

        u64 image_hash_value{0};
        HashCombine(image_hash_value, ast_image);
        auto it3{image_indices_.find(image_hash_value)};
        if (it3 != image_indices_.end()) {
          image_indices[idx] = it3->second;
        } else {
          const vk::raii::ImageView& image_view{ast_image->GetImageView()};

          vk::DescriptorImageInfo descriptor_image_info{
              nullptr, *image_view, vk::ImageLayout::eShaderReadOnlyOptimal};
          image_infos.push_back(std::move(descriptor_image_info));

          image_indices[idx] = global_image_index_++;

          vk::WriteDescriptorSet write_descriptor_set{
              *bindless_descriptor_set_, global_images_shader_resource.binding,
              image_indices[idx], vk::DescriptorType::eSampledImage,
              image_infos.back()};

          write_descriptor_sets.push_back(write_descriptor_set);

          image_indices_.emplace(image_hash_value, image_indices[idx]);
        }
        ++idx;
      }
    }
  }

  // 2. Draw element.
  for (u32 i{0}; i < frame_count_; ++i) {
    for (const auto& set_shader_resource : set_shader_resources) {
      u32 set{set_shader_resource.first};
      const auto& shader_resources{set_shader_resource.second};
      for (const auto& shader_resource : shader_resources) {
        if (shader_resource.type == ShaderResourceType::kUniformBuffer) {
          if (shader_resource.name == "DrawElementUniform") {
            DrawElementUniform draw_element_uniform{
                model_matrix, primitive.material->GetBaseColorFactor(),
                sampler_indices, image_indices};

            vk::BufferCreateInfo uniform_buffer_ci{
                {},
                sizeof(DrawElementUniform),
                vk::BufferUsageFlagBits::eUniformBuffer,
                vk::SharingMode::eExclusive};

            gpu::Buffer draw_element_uniform_buffer{gpu_->CreateBuffer(
                uniform_buffer_ci, &draw_element_uniform, false,
                "draw_element_uniform", primitive_index)};

            vk::DescriptorBufferInfo descriptor_buffer_info{
                *draw_element_uniform_buffer, 0, sizeof(DrawElementUniform)};
            buffer_infos.push_back(std::move(descriptor_buffer_info));

            vk::WriteDescriptorSet write_descriptor_set{
                *(draw_element.descriptor_sets[i][shader_resource.set - 2]),
                shader_resource.binding,
                0,
                vk::DescriptorType::eUniformBuffer,
                nullptr,
                buffer_infos.back()};

            draw_element.uniforms.push_back(std::move(draw_element_uniform));
            draw_element.uniform_buffers.push_back(
                std::move(draw_element_uniform_buffer));

            write_descriptor_sets.push_back(write_descriptor_set);
          }
        } else if (shader_resource.type ==
                   ShaderResourceType::kInputAttachment) {
          need_resize_ = true;
          vk::ImageView image_view{
              *(*attachment_image_views_)[i][shader_resource
                                                 .input_attachment_index]};
          vk::DescriptorImageInfo descriptor_image_info{
              nullptr, image_view, vk::ImageLayout::eShaderReadOnlyOptimal};
          image_infos.push_back(std::move(descriptor_image_info));

          vk::WriteDescriptorSet write_descriptor_set{
              *(draw_element.descriptor_sets[i][shader_resource.set - 2]),
              shader_resource.binding, 0, vk::DescriptorType::eInputAttachment,
              image_infos.back()};

          write_descriptor_sets.push_back(write_descriptor_set);
        } else if (shader_resource.type ==
                   ShaderResourceType::kCombinedImageSampler) {
          need_resize_ = true;
          vk::ImageView image_view{nullptr};

          auto it{(*shared_image_views_)[i].find(shader_resource.name)};
          if (it != (*shared_image_views_)[i].end()) {
            image_view = it->second;
          } else {
            THROW("Image view is nullptr");
          }

          vk::DescriptorImageInfo descriptor_image_info{
              *(gpu_->GetSampler()), image_view,
              vk::ImageLayout::eShaderReadOnlyOptimal};
          image_infos.push_back(std::move(descriptor_image_info));

          vk::WriteDescriptorSet write_descriptor_set{
              *(draw_element.descriptor_sets[i][shader_resource.set - 2]),
              shader_resource.binding, 0,
              vk::DescriptorType::eCombinedImageSampler, image_infos.back()};

          write_descriptor_sets.push_back(write_descriptor_set);
        }
      }
    }

    gpu_->UpdateDescriptorSets(write_descriptor_sets);
  }

  return draw_element;
}

const SPIRV& Subpass::RequestSpirv(const ast::Shader& shader,
                                   const std::vector<std::string>& processes,
                                   vk::ShaderStageFlagBits shader_stage) {
  u64 hash_value{shader.GetHashValue(processes)};

  auto it{spirv_shaders_.find(hash_value)};
  if (it != spirv_shaders_.end()) {
    return it->second;
  }

  std::filesystem::path root_path{GetPath(LUKA_ROOT_PATH)};
  std::filesystem::path cache_path{root_path / "cache"};
  std::filesystem::path spirv_cache_file{
      cache_path / ("spirv_" + std::to_string(hash_value) + ".cache")};

  std::vector<u32> spirv_cache_data;
  if (std::filesystem::exists(spirv_cache_file)) {
    spirv_cache_data = LoadBinaryU32(spirv_cache_file);
  } else {
    spirv_cache_data = shader.CompileToSpirv(processes);
    if (!std::filesystem::exists(cache_path)) {
      std::filesystem::create_directories(cache_path);
    }
    SaveBinaryU32(spirv_cache_data, spirv_cache_file);
  }

  SPIRV spirv{spirv_cache_data, shader_stage, hash_value};

  auto it1{spirv_shaders_.emplace(hash_value, std::move(spirv))};

  return it1.first->second;
}

const vk::raii::DescriptorSetLayout& Subpass::RequestDescriptorSetLayout(
    const vk::DescriptorSetLayoutCreateInfo& descriptor_set_layout_ci,
    const std::string& name, i32 index) {
  u64 hash_value{0};
  HashCombine(hash_value, descriptor_set_layout_ci.flags);
  for (u32 i{0}; i < descriptor_set_layout_ci.bindingCount; ++i) {
    HashCombine(hash_value, descriptor_set_layout_ci.pBindings[i]);
  }

  auto it{descriptor_set_layouts_.find(hash_value)};
  if (it != descriptor_set_layouts_.end()) {
    return it->second;
  }

  vk::raii::DescriptorSetLayout descriptor_set_layout{
      gpu_->CreateDescriptorSetLayout(descriptor_set_layout_ci, name, index)};

  auto it1{descriptor_set_layouts_.emplace(hash_value,
                                           std::move(descriptor_set_layout))};

  return it1.first->second;
}

const vk::raii::PipelineLayout& Subpass::RequestPipelineLayout(
    const vk::PipelineLayoutCreateInfo& pipeline_layout_ci,
    const std::string& name, i32 index) {
  u64 hash_value{0};
  HashCombine(hash_value, pipeline_layout_ci.flags);
  for (u32 i{0}; i < pipeline_layout_ci.setLayoutCount; ++i) {
    HashCombine(hash_value, pipeline_layout_ci.pSetLayouts[i]);
  }
  for (u32 i{0}; i < pipeline_layout_ci.pushConstantRangeCount; ++i) {
    HashCombine(hash_value, pipeline_layout_ci.pPushConstantRanges[i]);
  }

  auto it{pipeline_layouts_.find(hash_value)};
  if (it != pipeline_layouts_.end()) {
    return it->second;
  }

  vk::raii::PipelineLayout pipeline_layout{
      gpu_->CreatePipelineLayout(pipeline_layout_ci, name, index)};

  auto it1{pipeline_layouts_.emplace(hash_value, std::move(pipeline_layout))};

  return it1.first->second;
}

const vk::raii::ShaderModule& Subpass::RequestShaderModule(
    const vk::ShaderModuleCreateInfo& shader_module_ci, u64 hash_value,
    const std::string& name, i32 index) {
  auto it{shader_modules_.find(hash_value)};
  if (it != shader_modules_.end()) {
    return it->second;
  }

  vk::raii::ShaderModule shader_module{
      gpu_->CreateShaderModule(shader_module_ci, name, index)};

  auto it1{shader_modules_.emplace(hash_value, std::move(shader_module))};

  return it1.first->second;
}

const vk::raii::Pipeline& Subpass::RequestPipeline(
    const vk::GraphicsPipelineCreateInfo& graphics_pipeline_ci, u64 hash_value,
    const std::string& name, i32 index) {
  auto it{pipelines_.find(hash_value)};
  if (it != pipelines_.end()) {
    return it->second;
  }

  vk::PipelineCacheCreateInfo pipeline_cache_ci;

  std::filesystem::path root_path{GetPath(LUKA_ROOT_PATH)};
  std::filesystem::path cache_path{root_path / "cache"};
  std::filesystem::path pipeline_cache_file{
      cache_path / ("pipeline_" + std::to_string(hash_value) + ".cache")};
  std::vector<u8> pipeline_cache_data;

  bool has_cache{false};
  if (std::filesystem::exists(pipeline_cache_file)) {
    pipeline_cache_data = LoadBinaryU8(pipeline_cache_file);

    vk::PipelineCacheHeaderVersionOne* header_version_one{
        reinterpret_cast<vk::PipelineCacheHeaderVersionOne*>(
            pipeline_cache_data.data())};
    vk::PhysicalDeviceProperties physical_device_properties{
        gpu_->GetPhysicalDeviceProperties()};

    if (header_version_one->headerSize > 0 &&
        header_version_one->headerVersion ==
            vk::PipelineCacheHeaderVersion::eOne &&
        header_version_one->vendorID == physical_device_properties.vendorID &&
        header_version_one->deviceID == physical_device_properties.deviceID) {
      has_cache = true;

      pipeline_cache_ci.initialDataSize = pipeline_cache_data.size();
      pipeline_cache_ci.pInitialData = pipeline_cache_data.data();
    }
  }

  vk::raii::PipelineCache pipeline_cache{
      gpu_->CreatePipelineCache(pipeline_cache_ci, name, index)};

  vk::raii::Pipeline pipeline{
      gpu_->CreatePipeline(graphics_pipeline_ci, pipeline_cache, name, index)};

  if (!has_cache) {
    std::vector<u8> pipeline_cache_data{pipeline_cache.getData()};
    if (!std::filesystem::exists(cache_path)) {
      std::filesystem::create_directories(cache_path);
    }
    SaveBinaryU8(pipeline_cache_data, pipeline_cache_file);
  }

  auto it1{pipelines_.emplace(hash_value, std::move(pipeline))};

  return it1.first->second;
}

}  // namespace gs

}  // namespace luka
