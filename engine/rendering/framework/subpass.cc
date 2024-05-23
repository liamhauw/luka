// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
#include "resource/asset/scene_component/material.h"
// clang-format on

#include "rendering/framework/subpass.h"

#ifdef _MSC_VER
#undef MemoryBarrier
#endif
#include <utility>
#include <vulkan/vulkan_hash.hpp>

#include "core/log.h"
#include "core/util.h"
#include "resource/config/generated/root_path.h"

namespace luka::fw {

Subpass::Subpass(
    std::shared_ptr<Gpu> gpu, std::shared_ptr<Asset> asset,
    std::shared_ptr<Camera> camera, u32 frame_count, vk::RenderPass render_pass,
    const std::vector<std::vector<vk::raii::ImageView>>& attachment_image_views,
    u32 color_attachment_count, const std::vector<ast::Subpass>& ast_subpasses,
    u32 subpass_index, const std::vector<ScenePrimitive>& scene_primitives,
    std::vector<std::unordered_map<std::string, vk::ImageView>>&
        shared_image_views)
    : gpu_{std::move(gpu)},
      asset_{std::move(asset)},
      camera_{std::move(camera)},
      frame_count_{frame_count},
      render_pass_{render_pass},
      attachment_image_views_{&attachment_image_views},
      color_attachment_count_{color_attachment_count},
      ast_subpasses_{&ast_subpasses},
      subpass_index_{subpass_index},
      scene_primitives_{&scene_primitives},
      shared_image_views_{&shared_image_views},
      ast_subpass_{&(*ast_subpasses_)[subpass_index_]},
      name_{ast_subpass_->name},
      shaders_{&(ast_subpass_->shaders)},
      scene_{ast_subpass_->scene},
      lights_{&(ast_subpass_->lights)},
      has_scene_{!scene_.empty()},
      has_light_{!lights_->empty()},
      subpass_uniforms_(frame_count_),
      subpass_uniform_buffers_(frame_count_) {
  CreateDrawElements();
}

void Subpass::Resize(const std::vector<std::vector<vk::raii::ImageView>>&
                         attachment_image_views) {
  if (!need_resize_) {
    return;
  }

  attachment_image_views_ = &attachment_image_views;
  punctual_lights_.clear();
  subpass_desciptor_set_updated_ = false;
  bindless_sampler_index_ = 0;
  bindless_image_index_ = 0;

  CreateDrawElements();
}

void Subpass::Update(u32 frame_index) {
  if (has_subpass_descriptor_set_) {
    const glm::mat4& view{camera_->GetViewMatrix()};
    const glm::mat4& projection{camera_->GetProjectionMatrix()};
    const glm::vec3& camera_position{camera_->GetPosition()};

    glm::mat4 pv{projection * view};
    glm::mat4 inverse_pv{glm::inverse(pv)};

    subpass_uniforms_[frame_index] =
        SubpassUniform{pv, inverse_pv, glm::vec4{camera_position, 1.0F}};

    if (has_light_) {
      memcpy(subpass_uniforms_[frame_index].punctual_lights,
             punctual_lights_.data(),
             sizeof(ast::PunctualLight) * punctual_lights_.size());
    }

    void* mapped{subpass_uniform_buffers_[frame_index].Map()};
    memcpy(mapped,
           reinterpret_cast<const void*>(&(subpass_uniforms_[frame_index])),
           sizeof(SubpassUniform));
  }
}

const std::string& Subpass::GetName() const { return name_; }

const std::vector<DrawElement>& Subpass::GetDrawElements() const {
  return draw_elements_;
}

bool Subpass::HasPushConstant() const { return has_push_constant_; }

void Subpass::PushConstants(const vk::raii::CommandBuffer& command_buffer,
                            vk::PipelineLayout pipeline_layout) const {}

bool Subpass::HasSubpassDescriptorSet() const {
  return has_subpass_descriptor_set_;
}

u32 Subpass::GetSubpassDescriptorSetIndex() const {
  return subpass_descriptor_set_index_;
}

const vk::raii::DescriptorSet& Subpass::GetSubpassDescriptorSet(
    u32 frame_index) const {
  return subpass_descriptor_sets_[frame_index];
}

bool Subpass::HasBindlessDescriptorSet() const {
  return has_bindless_descriptor_set_;
}

u32 Subpass::GetBindlessDescriptorSetIndex() const {
  return bindless_descriptor_set_index_;
}

const vk::raii::DescriptorSet& Subpass::GetBindlessDescriptorSet() const {
  return bindless_descriptor_set_;
}

u32 Subpass::GetDrawElementDescriptorSetIndex() const {
  return draw_element_descriptor_set_index_;
}

void Subpass::CreateDrawElements() {
  draw_elements_.clear();

  if (!has_scene_) {
    draw_elements_.push_back(CreateDrawElement());
  } else {
    const std::string& ast_subpass_scene{ast_subpass_->scene};
    bool is_transparent{false};
    if (ast_subpass_scene == "transparency") {
      is_transparent = true;
    }

    for (const auto& scene_primitive : *scene_primitives_) {
      if ((!is_transparent &&
           scene_primitive.primitive->material->GetAlphaMode() !=
               ast::sc::AlphaMode::kBlend) ||
          (is_transparent &&
           scene_primitive.primitive->material->GetAlphaMode() ==
               ast::sc::AlphaMode::kBlend)) {
        draw_elements_.push_back(CreateDrawElement(scene_primitive));
      }
    }
  }
}

DrawElement Subpass::CreateDrawElement(const ScenePrimitive& scene_primitivce) {
  DrawElement draw_element{};
  draw_element.has_scene = has_scene_;

  // Parse shader resources.
  std::vector<const SPIRV*> spirvs;
  std::unordered_map<std::string, ShaderResource> name_shader_resources;
  std::unordered_map<u32, std::vector<ShaderResource>> set_shader_resources;
  std::vector<u32> sorted_sets;
  std::vector<vk::PushConstantRange> push_constant_ranges;
  ParseShaderResources(*(scene_primitivce.primitive), spirvs,
                       name_shader_resources, set_shader_resources, sorted_sets,
                       push_constant_ranges);

  // Create pipeline resources.
  CreatePipelineResources(
      scene_primitivce.model, scene_primitivce.inverse_model,
      *(scene_primitivce.primitive), name_shader_resources,
      set_shader_resources, sorted_sets, push_constant_ranges, draw_element);

  // Pipeline.
  CreatePipeline(*(scene_primitivce.primitive), spirvs, name_shader_resources,
                 draw_element);

  return draw_element;
}

void Subpass::ParseShaderResources(
    const ast::sc::Primitive& primitive, std::vector<const SPIRV*>& spirvs,
    std::unordered_map<std::string, ShaderResource>& name_shader_resources,
    std::unordered_map<u32, std::vector<ShaderResource>>& set_shader_resources,
    std::vector<u32>& sorted_sets,
    std::vector<vk::PushConstantRange>& push_constant_ranges) {
  std::vector<std::string> shader_processes;

  // Common.
  shader_processes.emplace_back("DPI 3.14159265359");

  std::string opaque_alpha{
      std::to_string(static_cast<u32>(ast::sc::AlphaMode::kOpaque))};
  opaque_alpha = "DOPAQUE_ALPHA " + opaque_alpha;
  shader_processes.push_back(opaque_alpha);

  std::string mask_alpha{
      std::to_string(static_cast<u32>(ast::sc::AlphaMode::kMask))};
  mask_alpha = "DMASK_ALPHA " + mask_alpha;
  shader_processes.push_back(mask_alpha);

  std::string blend_alpha{
      std::to_string(static_cast<u32>(ast::sc::AlphaMode::kBlend))};
  blend_alpha = "DBLEND_ALPHA " + blend_alpha;
  shader_processes.push_back(blend_alpha);

  std::string punctual_light_max_count{
      std::to_string(ast::kPunctualLightMaxCount)};
  punctual_light_max_count =
      "DPUNCTUAL_LIGHT_MAX_COUNT " + punctual_light_max_count;
  shader_processes.push_back(punctual_light_max_count);

  // Scene.
  if (has_scene_) {
    const std::map<std::string, ast::sc::Texture*>& textures{
        primitive.material->GetTextures()};
    for (u32 i{}; i < wanted_textures_.size(); ++i) {
      std::string wanted_texture{wanted_textures_[i]};
      auto it{textures.find(wanted_texture)};
      if (it != textures.end()) {
        std::transform(wanted_texture.begin(), wanted_texture.end(),
                       wanted_texture.begin(), ::toupper);
        shader_processes.push_back("DHAS_" + wanted_texture);
      }
    }

    bool has_position_buffer{};
    bool has_normal_buffer{};
    for (const auto& vertex_buffer_attribute : primitive.vertex_attributes) {
      std::string name{vertex_buffer_attribute.first};
      if (name == "POSITION") {
        has_position_buffer = true;
        continue;
      }
      if (name == "NORMAL") {
        has_normal_buffer = true;
        continue;
      }
      std::transform(name.begin(), name.end(), name.begin(), ::toupper);
      shader_processes.push_back("DHAS_" + name + "_BUFFER");
    }
    if (!has_position_buffer || !has_normal_buffer) {
      THROW("There is no position or/and normal buffer.");
    }

    if (primitive.material->GetAlphaMode() == ast::sc::AlphaMode::kMask) {
      shader_processes.emplace_back("DHAS_MASK_ALPHA");
    }
  }

  // Light.
  if (has_light_) {
    std::string directional_light{
        std::to_string(static_cast<u32>(ast::PunctualLightType::kDirectional))};
    directional_light = "DDIRECTIONAL_LIGHT " + directional_light;
    shader_processes.push_back(directional_light);

    std::string point_light{
        std::to_string(static_cast<u32>(ast::PunctualLightType::kPoint))};
    point_light = "DPOINT_LIGHT " + point_light;
    shader_processes.push_back(point_light);

    std::string spot_light{
        std::to_string(static_cast<u32>(ast::PunctualLightType::kSpot))};
    spot_light = "DSPOT_LIGHT " + spot_light;
    shader_processes.push_back(spot_light);

    for (const auto& light : *lights_) {
      const auto& pls{asset_->GetLight(light).GetPunctualLights()};
      for (const auto& pl : pls) {
        punctual_lights_.push_back(pl);
      }
      if (punctual_lights_.size() == ast::kPunctualLightMaxCount) {
        break;
      }
    }
    std::string punctual_light_count{std::to_string(punctual_lights_.size())};
    punctual_light_count = "DPUNCTUAL_LIGHT_COUNT " + punctual_light_count;
    shader_processes.push_back(punctual_light_count);
  }

  auto vi{shaders_->find(vk::ShaderStageFlagBits::eVertex)};
  if (vi == shaders_->end()) {
    THROW("There is no vertex shader");
  }
  auto fi{shaders_->find(vk::ShaderStageFlagBits::eFragment)};
  if (fi == shaders_->end()) {
    THROW("There is no fragment shader");
  }
  const SPIRV& vert_spirv{RequestSpirv(asset_->GetShader(vi->second),
                                       shader_processes,
                                       vk::ShaderStageFlagBits::eVertex)};
  const SPIRV& frag_spirv{RequestSpirv(asset_->GetShader(fi->second),
                                       shader_processes,
                                       vk::ShaderStageFlagBits::eFragment)};

  spirvs = {&vert_spirv, &frag_spirv};

  for (const auto* spirv : spirvs) {
    const auto& shader_resources{spirv->GetShaderResources()};
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
  if (!sorted_sets.empty() && (sorted_sets.back() != sorted_sets.size() - 1)) {
    THROW("Descriptor sets is not continuous.");
  }
}

void Subpass::CreatePipelineResources(
    const glm::mat4& model_matrix, const glm::mat4& inverse_model_matrix,
    const ast::sc::Primitive& primitive,
    const std::unordered_map<std::string, ShaderResource>&
        name_shader_resources,
    const std::unordered_map<u32, std::vector<ShaderResource>>&
        set_shader_resources,
    const std::vector<u32>& sorted_sets,
    const std::vector<vk::PushConstantRange>& push_constant_ranges,
    DrawElement& draw_element) {
  std::vector<vk::WriteDescriptorSet> write_descriptor_sets;
  std::vector<vk::DescriptorImageInfo> sampler_infos;
  std::vector<vk::DescriptorImageInfo> image_infos;
  std::vector<vk::DescriptorBufferInfo> buffer_infos;

  /**
   * Why we reserve these vectors:
   *
   * When we add an element to std::vector, if it exceeds the size of the
   * current container, std::vector will automatically perform a memory
   * reallocation, copying the element from the old location to the new one, but
   * the pointer stored in write_descriptor_sets will not change, causing an
   * error in the update to descriptors.
   */
  sampler_infos.reserve(kSamplerInfoMaxCount);
  image_infos.reserve(kImageInfoMaxCount);
  buffer_infos.reserve(kBufferInfoMaxCount);

  glm::uvec4 sampler_indices_0{};
  glm::uvec4 sampler_indices_1{};
  glm::uvec4 image_indices_0{};
  glm::uvec4 image_indices_1{};

  std::vector<vk::DescriptorSetLayout> set_layouts;

  for (u32 set : sorted_sets) {
    const vk::raii::DescriptorSetLayout* descriptor_set_layout{};

    const auto& shader_resources{set_shader_resources.at(set)};
    std::string resource_name{ToLower(shader_resources[0].name)};

    if (resource_name.find("subpass") != std::string::npos) {
      // Create descriptor set.
      if (!has_subpass_descriptor_set_) {
        has_subpass_descriptor_set_ = true;
        subpass_descriptor_set_index_ = set;

        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        for (const auto& shader_resource : shader_resources) {
          vk::DescriptorType descriptor_type{};

          if (shader_resource.type == ShaderResourceType::kUniformBuffer) {
            descriptor_type = vk::DescriptorType::eUniformBuffer;
          } else if (shader_resource.type ==
                     ShaderResourceType::kInputAttachment) {
            descriptor_type = vk::DescriptorType::eInputAttachment;
          } else {
            THROW("Unsupport descriptor type");
          }

          vk::DescriptorSetLayoutBinding binding{shader_resource.binding,
                                                 descriptor_type, 1,
                                                 shader_resource.stage};

          bindings.push_back(binding);
        }

        if (bindings.empty()) {
          continue;
        }

        vk::DescriptorSetLayoutCreateInfo subpass_descriptor_set_layout_ci{
            {}, bindings};

        subpass_descriptor_set_layout_ = &(RequestDescriptorSetLayout(
            subpass_descriptor_set_layout_ci, name_ + "_subpass"));

        std::vector<vk::DescriptorSetLayout> subpass_descriptor_set_layouts(
            frame_count_, **subpass_descriptor_set_layout_);
        vk::DescriptorSetAllocateInfo subpass_descriptor_set_allocate_info{
            nullptr, subpass_descriptor_set_layouts};
        subpass_descriptor_sets_ = gpu_->AllocateNormalDescriptorSets(
            subpass_descriptor_set_allocate_info, name_ + "_subpass");
      }

      // Update descriptor sets.
      if (!subpass_desciptor_set_updated_) {
        subpass_desciptor_set_updated_ = true;
        for (const auto& shader_resource : shader_resources) {
          if (shader_resource.type == ShaderResourceType::kUniformBuffer) {
            if (shader_resource.name == "Subpass") {
              for (u32 i{}; i < frame_count_; ++i) {
                SubpassUniform subpass_uniform{};

                vk::BufferCreateInfo uniform_buffer_ci{
                    {},
                    sizeof(SubpassUniform),
                    vk::BufferUsageFlagBits::eUniformBuffer,
                    vk::SharingMode::eExclusive};

                gpu::Buffer subpass_uniform_buffer{
                    gpu_->CreateBuffer(uniform_buffer_ci, &subpass_uniform,
                                       true, "subpass_uniform")};

                vk::DescriptorBufferInfo descriptor_buffer_info{
                    *subpass_uniform_buffer, 0, sizeof(SubpassUniform)};

                buffer_infos.push_back(descriptor_buffer_info);
                vk::WriteDescriptorSet write_descriptor_set{
                    *subpass_descriptor_sets_[i],
                    shader_resource.binding,
                    0,
                    vk::DescriptorType::eUniformBuffer,
                    nullptr,
                    buffer_infos.back()};

                subpass_uniforms_[i] = subpass_uniform;
                subpass_uniform_buffers_[i] = std::move(subpass_uniform_buffer);

                write_descriptor_sets.push_back(write_descriptor_set);
              }
            }

          } else if (shader_resource.type ==
                     ShaderResourceType::kInputAttachment) {
            need_resize_ = true;
            for (u32 i{}; i < frame_count_; ++i) {
              vk::ImageView image_view{
                  *(*attachment_image_views_)[i][shader_resource
                                                     .input_attachment_index]};
              vk::DescriptorImageInfo descriptor_image_info{
                  nullptr, image_view, vk::ImageLayout::eShaderReadOnlyOptimal};
              image_infos.push_back(descriptor_image_info);

              vk::WriteDescriptorSet write_descriptor_set{
                  *subpass_descriptor_sets_[i], shader_resource.binding, 0,
                  vk::DescriptorType::eInputAttachment, image_infos.back()};

              write_descriptor_sets.push_back(write_descriptor_set);
            }
          }
        }
      }

      descriptor_set_layout = subpass_descriptor_set_layout_;

    } else if (resource_name.find("bindless") != std::string::npos) {
      if (!has_bindless_descriptor_set_) {
        // Create descriptor set layout.
        has_bindless_descriptor_set_ = true;
        bindless_descriptor_set_index_ = set;

        std::vector<vk::DescriptorSetLayoutBinding> bindings{
            {0, vk::DescriptorType::eSampler, kBindlessSamplerMaxCount,
             vk::ShaderStageFlagBits::eAll},
            {1, vk::DescriptorType::eSampledImage, kBindlessImageMaxCount,
             vk::ShaderStageFlagBits::eAll}};

        std::vector<vk::DescriptorBindingFlags> binding_flags(
            2, vk::DescriptorBindingFlagBits::ePartiallyBound);

        vk::DescriptorSetLayoutBindingFlagsCreateInfo binding_flags_ci{
            binding_flags};

        vk::DescriptorSetLayoutCreateInfo bindless_descriptor_set_layout_ci{
            vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
            bindings, &binding_flags_ci};
        bindless_descriptor_set_layout_ = &(RequestDescriptorSetLayout(
            bindless_descriptor_set_layout_ci, name_ + "_bindless"));

        // Allocate descriptor set.
        vk::DescriptorSetAllocateInfo bindless_descriptor_set_allocate_info{
            nullptr, **bindless_descriptor_set_layout_};
        bindless_descriptor_set_ = gpu_->AllocateBindlessDescriptorSet(
            bindless_descriptor_set_allocate_info, name_ + "_bindless");
      }

      // Update descriptor set.
      if (has_scene_) {
        const std::map<std::string, ast::sc::Texture*>& textures{
            primitive.material->GetTextures()};

        i32 idx{};
        for (const auto& wanted_texture : wanted_textures_) {
          auto bindless_samplers_it{
              name_shader_resources.find("bindless_samplers")};
          auto bindless_images_it{
              name_shader_resources.find("bindless_images")};
          auto wanted_texture_it{textures.find(wanted_texture)};

          if (bindless_samplers_it != name_shader_resources.end() &&
              bindless_images_it != name_shader_resources.end() &&
              wanted_texture_it != textures.end()) {
            const ShaderResource& bindless_samplers_shader_resource{
                bindless_samplers_it->second};
            const ShaderResource& bindless_images_shader_resource{
                bindless_images_it->second};
            ast::sc::Texture* tex{wanted_texture_it->second};

            const ast::sc::Sampler* ast_sampler{tex->GetSampler()};
            u64 sampler_hash_value{};
            HashCombine(sampler_hash_value, ast_sampler);
            auto it2{sampler_indices_.find(sampler_hash_value)};

            u32 cur_sampler_index{};
            if (it2 != sampler_indices_.end()) {
              cur_sampler_index = it2->second;
            } else {
              const vk::raii::Sampler& sampler{ast_sampler->GetSampler()};

              vk::DescriptorImageInfo descriptor_sampler_info{*sampler};
              sampler_infos.push_back(descriptor_sampler_info);

              cur_sampler_index = bindless_sampler_index_++;

              if (cur_sampler_index >= kBindlessImageMaxCount) {
                THROW(
                    "cur_sampler_index ({}) exceeds kBindlessImageMaxCount "
                    "({})",
                    cur_sampler_index, kBindlessImageMaxCount);
              }

              vk::WriteDescriptorSet write_descriptor_set{
                  *(bindless_descriptor_set_),
                  bindless_samplers_shader_resource.binding, cur_sampler_index,
                  vk::DescriptorType::eSampler, sampler_infos.back()};

              write_descriptor_sets.push_back(write_descriptor_set);

              sampler_indices_.emplace(sampler_hash_value, cur_sampler_index);
            }

            const ast::sc::Image* ast_image{tex->GetImage()};

            u64 image_hash_value{};
            HashCombine(image_hash_value, ast_image);
            auto it3{image_indices_.find(image_hash_value)};

            u32 cur_image_index{};
            if (it3 != image_indices_.end()) {
              cur_image_index = it3->second;
            } else {
              const vk::raii::ImageView& image_view{ast_image->GetImageView()};

              vk::DescriptorImageInfo descriptor_image_info{
                  nullptr, *image_view,
                  vk::ImageLayout::eShaderReadOnlyOptimal};
              image_infos.push_back(descriptor_image_info);

              cur_image_index = bindless_image_index_++;

              if (cur_image_index >= kBindlessImageMaxCount) {
                THROW(
                    "cur_image_index ({}) exceeds kBindlessImageMaxCount ({})",
                    cur_image_index, kBindlessImageMaxCount);
              }

              vk::WriteDescriptorSet write_descriptor_set{
                  *bindless_descriptor_set_,
                  bindless_images_shader_resource.binding, cur_image_index,
                  vk::DescriptorType::eSampledImage, image_infos.back()};

              write_descriptor_sets.push_back(write_descriptor_set);

              image_indices_.emplace(image_hash_value, cur_image_index);
            }

            if (idx < 4) {
              sampler_indices_0[idx] = cur_sampler_index;
              image_indices_0[idx] = cur_image_index;
            } else {
              sampler_indices_1[idx - 4] = cur_sampler_index;
              image_indices_1[idx - 4] = cur_image_index;
            }
          }
          ++idx;
        }
      }

      descriptor_set_layout = bindless_descriptor_set_layout_;
    } else {
      // Create descriptor set layout.
      draw_element_descriptor_set_index_ =
          std::min(draw_element_descriptor_set_index_, set);
      std::vector<vk::DescriptorSetLayoutBinding> bindings;
      for (const auto& shader_resource : shader_resources) {
        vk::DescriptorType descriptor_type{};

        if (shader_resource.type == ShaderResourceType::kUniformBuffer) {
          descriptor_type = vk::DescriptorType::eUniformBuffer;
        } else if (shader_resource.type ==
                   ShaderResourceType::kCombinedImageSampler) {
          descriptor_type = vk::DescriptorType::eCombinedImageSampler;
        } else {
          THROW("Unsupport descriptor type");
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

      // Allocate descriptor sets.
      draw_element.has_descriptor_set = true;
      for (u32 i{}; i < frame_count_; ++i) {
        vk::DescriptorSetAllocateInfo draw_element_descriptor_set_allocate_info{
            nullptr, **draw_element_descriptor_set_layout};

        vk::raii::DescriptorSets draw_element_descriptor_sets{
            gpu_->AllocateNormalDescriptorSets(
                draw_element_descriptor_set_allocate_info,
                name_ + "_draw_element")};

        draw_element.descriptor_sets.push_back(
            std::move(draw_element_descriptor_sets));
      }

      // Update descriptor sets.
      for (const auto& shader_resource : shader_resources) {
        if (shader_resource.type == ShaderResourceType::kUniformBuffer) {
          if (shader_resource.name == "DrawElement") {
            for (u32 i{}; i < frame_count_; ++i) {
              DrawElementUniform draw_element_uniform{
                  model_matrix,
                  inverse_model_matrix,
                  sampler_indices_0,
                  sampler_indices_1,
                  image_indices_0,
                  image_indices_1,
                  primitive.material->GetBaseColorFactor(),
                  primitive.material->GetMetallicFactor(),
                  primitive.material->GetRoughnessFactor(),
                  primitive.material->GetNormalScale(),
                  primitive.material->GetOcclusionStrength(),
                  glm::vec4{primitive.material->GetEmissiveFactor(), 1.0F},
                  static_cast<u32>(primitive.material->GetAlphaMode()),
                  primitive.material->GetAlphaCutoff()};

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
              buffer_infos.push_back(descriptor_buffer_info);

              vk::WriteDescriptorSet write_descriptor_set{
                  *(draw_element
                        .descriptor_sets[i]
                                        [shader_resource.set -
                                         draw_element_descriptor_set_index_]),
                  shader_resource.binding,
                  0,
                  vk::DescriptorType::eUniformBuffer,
                  nullptr,
                  buffer_infos.back()};

              draw_element.uniforms.push_back(draw_element_uniform);
              draw_element.uniform_buffers.push_back(
                  std::move(draw_element_uniform_buffer));

              write_descriptor_sets.push_back(write_descriptor_set);
            }
          }
        } else if (shader_resource.type ==
                   ShaderResourceType::kCombinedImageSampler) {
          need_resize_ = true;
          for (u32 i{}; i < frame_count_; ++i) {
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
            image_infos.push_back(descriptor_image_info);

            vk::WriteDescriptorSet write_descriptor_set{
                *(draw_element
                      .descriptor_sets[i][shader_resource.set -
                                          draw_element_descriptor_set_index_]),
                shader_resource.binding, 0,
                vk::DescriptorType::eCombinedImageSampler, image_infos.back()};

            write_descriptor_sets.push_back(write_descriptor_set);
          }
        }
      }
    }

    set_layouts.push_back(**descriptor_set_layout);
  }

  if (sampler_infos.size() >= kSamplerInfoMaxCount) {
    THROW("The size of sampler_infos ({}) exceeds kSamplerInfoMaxCount ({})",
          sampler_infos.size(), kSamplerInfoMaxCount);
  }
  if (image_infos.size() >= kImageInfoMaxCount) {
    THROW("The size of sampler_infos ({}) exceeds kSamplerInfoMaxCount ({})",
          image_infos.size(), kImageInfoMaxCount);
  }
  if (buffer_infos.size() >= kBufferInfoMaxCount) {
    THROW("The size of sampler_infos ({}) exceeds kSamplerInfoMaxCount ({})",
          buffer_infos.size(), kBufferInfoMaxCount);
  }

  gpu_->UpdateDescriptorSets(write_descriptor_sets);

  // Pipeline layouts.
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
}

void Subpass::CreatePipeline(
    const ast::sc::Primitive& primitive,
    const std::vector<const SPIRV*>& spirvs,
    const std::unordered_map<std::string, ShaderResource>&
        name_shader_resources,
    DrawElement& draw_element) {
  std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_cis;
  u64 pipeline_hash_value{};
  for (const auto* spirv_shader : spirvs) {
    u64 shader_module_hash_value{spirv_shader->GetHashValue()};
    HashCombine(pipeline_hash_value, shader_module_hash_value);

    const std::vector<u32>& spirv{spirv_shader->GetSpirv()};

    vk::ShaderModuleCreateInfo shader_module_ci{
        {}, spirv.size() * 4, spirv.data()};
    const vk::raii::ShaderModule& shader_module{
        RequestShaderModule(shader_module_ci, shader_module_hash_value, name_)};

    vk::PipelineShaderStageCreateInfo shader_stage_ci{
        {}, spirv_shader->GetStage(), *shader_module, "main", nullptr};

    shader_stage_cis.push_back(shader_stage_ci);
  }

  vk::PipelineVertexInputStateCreateInfo vertex_input_state_ci;
  std::vector<vk::VertexInputBindingDescription>
      vertex_input_binding_descriptions;
  std::vector<vk::VertexInputAttributeDescription>
      vertex_input_attribute_descriptions;
  if (has_scene_) {
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
          vertex_input_binding_description);

      vk::VertexInputAttributeDescription vertex_input_attribute_description{
          shader_resource.location, shader_resource.location,
          vertex_attribute.format, vertex_attribute.offset};
      vertex_input_attribute_descriptions.push_back(
          vertex_input_attribute_description);

      vertex_location_attributes.emplace(shader_resource.location,
                                         &vertex_attribute);
      if (draw_element.vertex_count == 0) {
        draw_element.vertex_count = vertex_attribute.count;
      }
    }

    std::vector<u32> locations;
    locations.reserve(vertex_location_attributes.size());
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

  if (!has_scene_ || primitive.material->GetDoubleSided()) {
    rasterization_state_ci.cullMode = vk::CullModeFlagBits::eNone;
  }

  vk::PipelineMultisampleStateCreateInfo multisample_state_ci{
      {}, vk::SampleCountFlagBits::e1};

  vk::PipelineDepthStencilStateCreateInfo depth_stencil_state_ci{
      {}, VK_TRUE, VK_TRUE, vk::CompareOp::eLess, VK_FALSE, VK_FALSE,
  };

  u32 blend_enable{VK_FALSE};
  if (scene_ == "transparency") {
    blend_enable = VK_TRUE;
  }

  vk::ColorComponentFlags color_component_flags{
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};
  std::vector<vk::PipelineColorBlendAttachmentState>
      color_blend_attachment_states(
          color_attachment_count_,
          vk::PipelineColorBlendAttachmentState{
              blend_enable, vk::BlendFactor::eSrcAlpha,
              vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
              vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
              color_component_flags});

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
      **(draw_element.pipeline_layout),
      render_pass_,
      subpass_index_};

  const vk::raii::Pipeline& pipeline{RequestPipeline(
      graphics_pipeline_create_info, pipeline_hash_value, name_)};
  draw_element.pipeline = &pipeline;
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
  std::filesystem::path cache_path{root_path / ".cache" / "engine"};
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
  u64 hash_value{};
  HashCombine(hash_value, descriptor_set_layout_ci.flags);
  for (u32 i{}; i < descriptor_set_layout_ci.bindingCount; ++i) {
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
  u64 hash_value{};
  HashCombine(hash_value, pipeline_layout_ci.flags);
  for (u32 i{}; i < pipeline_layout_ci.setLayoutCount; ++i) {
    HashCombine(hash_value, pipeline_layout_ci.pSetLayouts[i]);
  }
  for (u32 i{}; i < pipeline_layout_ci.pushConstantRangeCount; ++i) {
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
  HashCombine(hash_value, *(graphics_pipeline_ci.pRasterizationState));
  auto it{pipelines_.find(hash_value)};
  if (it != pipelines_.end()) {
    return it->second;
  }

  vk::PipelineCacheCreateInfo pipeline_cache_ci;

  std::filesystem::path root_path{GetPath(LUKA_ROOT_PATH)};
  std::filesystem::path cache_path{root_path / ".cache" / "engine"};
  std::filesystem::path pipeline_cache_file{
      cache_path / ("pipeline_" + std::to_string(hash_value) + ".cache")};
  std::vector<u8> pipeline_cache_data;

  bool has_cache{};
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

}  // namespace luka::fw
