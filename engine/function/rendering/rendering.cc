// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/rendering/rendering.h"

#include "context.h"
#include "core/log.h"

namespace luka {

Rendering::Rendering() : asset_{gContext.asset}, gpu_{gContext.gpu} {
  CreateModelResource();
  CreatePipeline();

  CreateGeometry();
  CreateGBuffer();
}

Rendering::~Rendering() { gpu_->WaitIdle(); }

void Rendering::Tick() {
  if (gContext.window->GetIconified()) {
    return;
  }

  if (gContext.window->GetFramebufferResized()) {
    Resize();
    return;
  }
}

std::pair<const vk::raii::Sampler&, const vk::raii::ImageView&>
Rendering::GetViewportImage() const {
  return std::make_pair(std::ref(sampler_), std::ref(color_image_views_[0]));
}

void Rendering::Render(const vk::raii::CommandBuffer& command_buffer) {
  vk::RenderingAttachmentInfo attachment_info{
      {},
      vk::ImageLayout::eAttachmentOptimal,
      {},
      {},
      {},
      vk::AttachmentLoadOp::eClear,
      vk::AttachmentStoreOp::eStore};

  std::vector<vk::RenderingAttachmentInfo> color_attachment_infos;
  attachment_info.clearValue.color = {0.0F, 0.0F, 0.0F, 1.0F};
  for (const auto& color_image_view : color_image_views_) {
    attachment_info.imageView = *color_image_view;
    color_attachment_infos.push_back(attachment_info);
  }

  vk::RenderingInfo rendering_info{
      {}, {{0, 0}, extent_}, 1, 0, color_attachment_infos, nullptr, nullptr};

  command_buffer.beginRendering(rendering_info);

  command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline_);

  command_buffer.setViewport(
      0, vk::Viewport{0.0F, 0.0F, static_cast<float>(extent_.width),
                      static_cast<float>(extent_.height), 0.0F, 1.0F});
  command_buffer.setScissor(0, vk::Rect2D{vk::Offset2D{0, 0}, extent_});

  command_buffer.bindVertexBuffers(0, *vertex_buffer_, {0});
  command_buffer.bindIndexBuffer(*index_buffer_, 0, vk::IndexType::eUint16);

  command_buffer.drawIndexed(6, 1, 0, 0, 0);

  command_buffer.endRendering();
}

void Rendering::Resize() {
  gpu_->WaitIdle();
  color_image_views_.clear();
  color_images_.clear();
  depth_image_view_.clear();
  depth_image_.Clear();
  sampler_.clear();
  CreateGBuffer();
}

void Rendering::CreateModelResource() {
  const tinygltf::Model& model{asset_->GetModel()};

  const vk::raii::CommandBuffer& command_buffer{gpu_->BeginTempCommandBuffer()};

  // Buffers.
  {
    u64 model_buffer_count{model.bufferViews.size()};

    vk::BufferCreateInfo model_buffer_ci{
        {},
        {},
        vk::BufferUsageFlagBits::eVertexBuffer |
            vk::BufferUsageFlagBits::eIndexBuffer |
            vk::BufferUsageFlagBits::eTransferDst,
        vk::SharingMode::eExclusive};

    vk::BufferCreateInfo model_buffer_staging_buffer_ci{
        {}, {}, vk::BufferUsageFlagBits::eTransferSrc};

    for (u64 i{0}; i < model_buffer_count; ++i) {
      const tinygltf::BufferView& buffer_view{model.bufferViews[i]};

      const std::vector<u8>& buffer{(model.buffers)[buffer_view.buffer].data};

      const u8* buffer_data{buffer.data() + buffer_view.byteOffset};
      u64 buffer_size{buffer_view.byteLength};

      model_buffer_ci.size = buffer_size;

      model_buffer_staging_buffer_ci.size = buffer_size;
      Buffer model_buffer_staging_buffer{
          gpu_->CreateBuffer(model_buffer_staging_buffer_ci, buffer_data)};
      model_buffer_staging_buffers_.push_back(
          std::move(model_buffer_staging_buffer));

      Buffer model_buffer{gpu_->CreateBuffer(
          model_buffer_ci, model_buffer_staging_buffers_[i], command_buffer)};

      model_buffers_.push_back(std::move(model_buffer));
    }
  }

  // Images and image views.
  {
    u64 model_image_count{model.images.size()};

    vk::BufferCreateInfo model_image_staging_buffer_ci{
        {}, {}, vk::BufferUsageFlagBits::eTransferSrc};
    vk::ImageCreateInfo model_image_ci{{}, vk::ImageType::e2D, {}, {}, 1, 1};
    vk::ImageViewCreateInfo model_image_view_ci{
        {}, {}, vk::ImageViewType::e2D,
        {}, {}, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};

    for (u64 i{0}; i < model_image_count; ++i) {
      const tinygltf::Image& image{model.images[i]};

      const std::string& name{image.name};
      vk::Format format;

      if (image.component == 4 && image.bits == 8) {
        format = vk::Format::eR8G8B8A8Unorm;
      } else {
        THROW("Unsupport image format.");
      }

      model_image_ci.format = format;
      model_image_ci.extent = vk::Extent3D{static_cast<u32>(image.width),
                                           static_cast<u32>(image.height), 1};
      model_image_ci.usage = vk::ImageUsageFlagBits::eSampled |
                             vk::ImageUsageFlagBits::eTransferDst;

      model_image_staging_buffer_ci.size = image.image.size();
      Buffer model_image_staging_buffer{gpu_->CreateBuffer(
          model_image_staging_buffer_ci, image.image.data())};
      model_image_staging_buffers_.push_back(
          std::move(model_image_staging_buffer));

      Image model_image{gpu_->CreateImage(
          model_image_ci, vk::ImageLayout::eShaderReadOnlyOptimal,
          model_image_staging_buffers_[i], command_buffer, name)};

      model_images_.push_back(std::move(model_image));

      model_image_view_ci.image = *model_images_[i];
      model_image_view_ci.format = format;

      vk::raii::ImageView image_view =
          gpu_->CreateImageView(model_image_view_ci, name);

      model_image_views_.push_back(std::move(image_view));
    }
  }

  // Samplers.
  {
    u64 model_sampler_count{model.samplers.size()};

    vk::SamplerCreateInfo sampler_ci;
    for (u64 i{0}; i < model_sampler_count; ++i) {
      const tinygltf::Sampler& sampler{model.samplers[i]};

      const std::string& name{sampler.name};

      switch (sampler.magFilter) {
        case TINYGLTF_TEXTURE_FILTER_NEAREST:
          sampler_ci.magFilter = vk::Filter::eNearest;
          break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR:
          sampler_ci.magFilter = vk::Filter::eLinear;
          break;
        default:
          sampler_ci.magFilter = vk::Filter::eNearest;
          break;
      }

      switch (sampler.minFilter) {
        case TINYGLTF_TEXTURE_FILTER_NEAREST:
          sampler_ci.minFilter = vk::Filter::eNearest;
          sampler_ci.mipmapMode = vk::SamplerMipmapMode::eNearest;
          break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR:
          sampler_ci.minFilter = vk::Filter::eLinear;
          sampler_ci.mipmapMode = vk::SamplerMipmapMode::eNearest;
          break;
        case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
          sampler_ci.minFilter = vk::Filter::eNearest;
          sampler_ci.mipmapMode = vk::SamplerMipmapMode::eNearest;
          break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
          sampler_ci.minFilter = vk::Filter::eLinear;
          sampler_ci.mipmapMode = vk::SamplerMipmapMode::eNearest;
          break;
        case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
          sampler_ci.minFilter = vk::Filter::eNearest;
          sampler_ci.mipmapMode = vk::SamplerMipmapMode::eLinear;
          break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
          sampler_ci.minFilter = vk::Filter::eLinear;
          sampler_ci.mipmapMode = vk::SamplerMipmapMode::eLinear;
          break;
        default:
          sampler_ci.minFilter = vk::Filter::eNearest;
          sampler_ci.mipmapMode = vk::SamplerMipmapMode::eNearest;
          break;
      }

      switch (sampler.wrapS) {
        case TINYGLTF_TEXTURE_WRAP_REPEAT:
          sampler_ci.addressModeU = vk::SamplerAddressMode::eRepeat;
          break;
        case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
          sampler_ci.addressModeU = vk::SamplerAddressMode::eClampToEdge;
          break;
        case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
          sampler_ci.addressModeU = vk::SamplerAddressMode::eMirroredRepeat;
          break;
        default:
          sampler_ci.addressModeU = vk::SamplerAddressMode::eRepeat;
          break;
      }

      switch (sampler.wrapT) {
        case TINYGLTF_TEXTURE_WRAP_REPEAT:
          sampler_ci.addressModeV = vk::SamplerAddressMode::eRepeat;
          break;
        case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
          sampler_ci.addressModeV = vk::SamplerAddressMode::eClampToEdge;
          break;
        case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
          sampler_ci.addressModeV = vk::SamplerAddressMode::eMirroredRepeat;
          break;
        default:
          sampler_ci.addressModeV = vk::SamplerAddressMode::eRepeat;
          break;
      }

      vk::raii::Sampler model_sampler{gpu_->CreateSampler(sampler_ci, name)};

      model_samplers_.push_back(std::move(model_sampler));
    }
  }

  gpu_->EndTempCommandBuffer(command_buffer);

  model_buffer_staging_buffers_.clear();
  model_image_staging_buffers_.clear();

  // Scene.
  {
    const tinygltf::Scene& scene{model.scenes[model.defaultScene]};
    const std::vector<i32>& scene_nodes{scene.nodes};

    std::map<i32, i32> node_parents;
    std::vector<i32> node_stack;
    std::map<i32, glm::dmat4> node_mat4s;

    for (i32 i{0}; i < scene_nodes.size(); ++i) {
      i32 root_node{scene_nodes[i]};
      node_parents[root_node] = -1;
      node_stack.push_back(root_node);
    }

    while (!node_stack.empty()) {
      i32 node_index{node_stack.back()};
      node_stack.pop_back();
      const tinygltf::Node node{model.nodes[node_index]};
      // Model matrix.
      glm::dmat4 model_mat4{1.0f};
      {
        glm::dmat4 cur_model_mat4{1.0f};

        if (!node.matrix.empty()) {
          cur_model_mat4 = glm::make_mat4(node.matrix.data());
        } else {
          if (!node.translation.empty()) {
            glm::dvec3 translation_vec3{
                glm::make_vec3(node.translation.data())};
            cur_model_mat4 = glm::translate(cur_model_mat4, translation_vec3);
          }

          if (!node.rotation.empty()) {
            glm::dvec4 roatation_vec4{glm::make_vec4(node.rotation.data())};
            glm::dquat rotation{roatation_vec4};
            cur_model_mat4 = cur_model_mat4 * glm::mat4_cast(rotation);
          }

          if (!node.scale.empty()) {
            glm::dvec3 scale_vec3{glm::make_vec3(node.scale.data())};
            cur_model_mat4 = glm::scale(cur_model_mat4, scale_vec3);
          }
        }

        node_mat4s[node_index] = cur_model_mat4;

        model_mat4 = cur_model_mat4;
        i32 node_parent{node_parents[node_index]};
        while (node_parent != -1) {
          model_mat4 = node_mat4s[node_parent] * model_mat4;
          node_parent = node_parents[node_parent];
        }

        for (i32 i{0}; i < node.children.size(); ++i) {
          i32 node_children_index{node.children[i]};
          node_parents[node_children_index] = node_index;
          node_stack.push_back(node_children_index);
        }
      }

      // Mesh.
      const tinygltf::Mesh& mesh{model.meshes[node.mesh]};
      for (u32 i{0}; i < mesh.primitives.size(); ++i) {
        DrawElement draw_element;
        draw_element.material_data.model_mat4 = model_mat4;

        const tinygltf::Primitive& primitive{mesh.primitives[i]};

        const tinygltf::Accessor& indices_accessor{
            model.accessors[primitive.indices]};

        switch (indices_accessor.componentType) {
          case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            draw_element.index_type = vk::IndexType::eUint16;
            break;
          case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            draw_element.index_type = vk::IndexType::eUint32;
          default:
            draw_element.index_type = vk::IndexType::eUint16;
            break;
        }

        draw_element.index_buffer_index = indices_accessor.bufferView;
        draw_element.index_buffer_offset = indices_accessor.byteOffset;
        draw_element.index_count = indices_accessor.count;

        const std::map<std::string, i32>& primitive_attributes{
            primitive.attributes};

        if (primitive_attributes.contains("POSITION")) {
          i32 position_accessor_index{primitive_attributes.at("POSITION")};
          const tinygltf::Accessor& position_accessor{
              model.accessors[position_accessor_index]};

          draw_element.postion_buffer_index = position_accessor.bufferView;
          draw_element.position_buffer_offset = position_accessor.byteOffset;
        } else {
          THROW("Don't have positon data.");
        }

        if (primitive_attributes.contains("TEXCOORD_0")) {
          i32 texcoord0_accessor_index{primitive_attributes.at("TEXCOORD_0")};
          const tinygltf::Accessor& texcoord0_accessor{
              model.accessors[texcoord0_accessor_index]};

          draw_element.texcoord0_buffer_index = texcoord0_accessor.bufferView;
          draw_element.texcoord0_buffer_offset = texcoord0_accessor.byteOffset;
        } else {
          THROW("Don't have texcoord0 data.");
        }

        if (primitive_attributes.contains("NORMAL")) {
          i32 normal_accessor_index{primitive_attributes.at("NORMAL")};
          const tinygltf::Accessor& normal_accessor{
              model.accessors[normal_accessor_index]};

          draw_element.normal_buffer_index = normal_accessor.bufferView;
          draw_element.normal_buffer_offset = normal_accessor.byteOffset;
        } else {
          THROW("Don't have normal data.");
        }

        if (primitive_attributes.contains("TANGENT")) {
          i32 tangent_accessor_index{primitive_attributes.at("TANGENT")};
          const tinygltf::Accessor& tangent_accessor{
              model.accessors[tangent_accessor_index]};

          draw_element.tangent_buffer_index = tangent_accessor.bufferView;
          draw_element.tangent_buffer_offset = tangent_accessor.byteOffset;
        }

        const tinygltf::Material& material{model.materials[primitive.material]};


        draw_elements_.push_back(draw_element);



      }
    }

    
  }
}

void Rendering::CreatePipeline() {
  const std::vector<u8>& vertex_shader_buffer{asset_->GetVertexShaderBuffer()};
  const std::vector<u8>& fragment_shader_buffer{
      asset_->GetFragmentShaderBuffer()};

  std::vector<std::pair<u32, vk::Format>> vertex_input_stride_format{
      {12, vk::Format::eR32G32B32Sfloat},
      {16, vk::Format::eR32G32B32A32Sfloat},
      {12, vk::Format::eR32G32B32Sfloat},
      {8, vk::Format::eR32G32Sfloat}};

  std::vector<vk::DescriptorSetLayoutBinding> descriptor_set_layout_bindings{
      {0, vk::DescriptorType::eUniformBufferDynamic, 1,
       vk::ShaderStageFlagBits::eAll},
      {1, vk::DescriptorType::eUniformBufferDynamic, 1,
       vk::ShaderStageFlagBits::eAll},
      {2, vk::DescriptorType::eCombinedImageSampler, 1,
       vk::ShaderStageFlagBits::eAll},
      {3, vk::DescriptorType::eCombinedImageSampler, 1,
       vk::ShaderStageFlagBits::eAll},
      {4, vk::DescriptorType::eCombinedImageSampler, 1,
       vk::ShaderStageFlagBits::eAll},
      {5, vk::DescriptorType::eCombinedImageSampler, 1,
       vk::ShaderStageFlagBits::eAll},
      {6, vk::DescriptorType::eCombinedImageSampler, 1,
       vk::ShaderStageFlagBits::eAll},
  };

  vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_ci{
      {}, descriptor_set_layout_bindings};
  descriptor_set_layout_ =
      gpu_->CreateDescriptorSetLayout(descriptor_set_layout_ci, "rendering");

  vk::PipelineLayoutCreateInfo pipeline_layout_ci{{}, *descriptor_set_layout_};
  pipeline_layout_ =
      gpu_->CreatePipelineLayout(pipeline_layout_ci, "rendering");

  vk::PipelineRenderingCreateInfo pipeline_rendering_ci{
      {}, color_formats_, depth_format_};

  pipeline_ = gpu_->CreatePipeline(vertex_shader_buffer, fragment_shader_buffer,
                                   vertex_input_stride_format, pipeline_layout_,
                                   pipeline_rendering_ci, "rendering");
}

void Rendering::CreateGeometry() {
  // const std::vector<Vertex> vertices{{{-0.5F, -0.5F}, {1.0F, 1.0F, 0.0F}},
  //                                    {{0.5F, -0.5F}, {0.0F, 1.0F, 1.0F}},
  //                                    {{0.5F, 0.5F}, {1.0F, 0.0F, 1.0F}},
  //                                    {{-0.5F, 0.5F}, {1.0F, 1.0F, 1.0F}}};
  // const std::vector<u16> indices = {0, 2, 1, 2, 0, 3};

  // u64 vertices_size{vertices.size() * sizeof(Vertex)};
  // u64 indices_size{indices.size() * sizeof(u16)};

  // vk::BufferCreateInfo vertex_buffer_ci{
  //     {},
  //     vertices_size,
  //     vk::BufferUsageFlagBits::eVertexBuffer |
  //         vk::BufferUsageFlagBits::eTransferDst,
  //     vk::SharingMode::eExclusive};
  // vk::BufferCreateInfo index_buffer_ci{
  //     {},
  //     indices_size,
  //     vk::BufferUsageFlagBits::eIndexBuffer |
  //         vk::BufferUsageFlagBits::eTransferDst,
  //     vk::SharingMode::eExclusive};

  // vertex_buffer_ = gpu_->CreateBuffer(vertex_buffer_ci, false, vertices_size,
  //                                     vertices.data(), "vertex");
  // index_buffer_ = gpu_->CreateBuffer(index_buffer_ci, false, indices_size,
  //                                    indices.data(), "index");
}

void Rendering::CreateGBuffer() {
  // extent_ = gpu_->GetExtent2D();
  // u64 color_image_count{color_formats_.size()};

  // {
  //   vk::ImageCreateInfo image_ci{{},
  //                                vk::ImageType::e2D,
  //                                {},
  //                                {extent_.width, extent_.height, 1},
  //                                1,
  //                                1,
  //                                vk::SampleCountFlagBits::e1,
  //                                vk::ImageTiling::eOptimal,
  //                                vk::ImageUsageFlagBits::eColorAttachment |
  //                                    vk::ImageUsageFlagBits::eSampled |
  //                                    vk::ImageUsageFlagBits::eStorage,
  //                                vk::SharingMode::eExclusive,
  //                                {},
  //                                vk::ImageLayout::eUndefined};

  //   vk::ImageViewCreateInfo image_view_ci{
  //       {},
  //       {},
  //       vk::ImageViewType::e2D,
  //       {},
  //       {},
  //       {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0,
  //        VK_REMAINING_ARRAY_LAYERS}};

  //   for (u64 i = 0; i < color_image_count; ++i) {
  //     image_ci.format = color_formats_[i];
  //     color_images_.push_back(std::move(
  //         gpu_->CreateImage(image_ci, vk::ImageLayout::eGeneral, 0, nullptr,
  //                           "g_color_" + std::to_string(i))));

  //     image_view_ci.image = *color_images_[i];
  //     image_view_ci.format = color_formats_[i];
  //     color_image_views_.push_back(std::move(gpu_->CreateImageView(
  //         image_view_ci, "g_color_" + std::to_string(i))));
  //   }
  // }

  // if (depth_format_ != vk::Format::eUndefined) {
  //   vk::ImageCreateInfo image_ci{
  //       {},
  //       vk::ImageType::e2D,
  //       {},
  //       {extent_.width, extent_.height, 1},
  //       1,
  //       1,
  //       vk::SampleCountFlagBits::e1,
  //       vk::ImageTiling::eOptimal,
  //       vk::ImageUsageFlagBits::eDepthStencilAttachment |
  //           vk::ImageUsageFlagBits::eSampled,
  //       vk::SharingMode::eExclusive,
  //       {},
  //       vk::ImageLayout::eUndefined};

  //   depth_image_ = gpu_->CreateImage(image_ci, vk::ImageLayout::eGeneral, 0,
  //                                    nullptr, "g_depth");

  //   vk::ImageViewCreateInfo image_view_ci{
  //       {},
  //       {},
  //       vk::ImageViewType::e2D,
  //       {},
  //       {},
  //       {vk::ImageAspectFlagBits::eDepth, 0, VK_REMAINING_MIP_LEVELS, 0,
  //        VK_REMAINING_ARRAY_LAYERS}};
  //   depth_image_view_ = gpu_->CreateImageView(image_view_ci, "g_depth");
  // }

  // vk::SamplerCreateInfo sampler_ci{{},
  //                                  vk::Filter::eLinear,
  //                                  vk::Filter::eLinear,
  //                                  vk::SamplerMipmapMode::eLinear,
  //                                  vk::SamplerAddressMode::eClampToBorder,
  //                                  vk::SamplerAddressMode::eClampToBorder,
  //                                  vk::SamplerAddressMode::eClampToBorder};
  // sampler_ = gpu_->CreateSampler(sampler_ci, "g");
}

}  // namespace luka
