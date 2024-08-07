// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/frame_graph.h"

#include "core/log.h"

namespace luka::ast {

FrameGraph::FrameGraph(const std::filesystem::path& frame_graph_path) {
  std::ifstream frame_graph_file{frame_graph_path.string()};
  if (!frame_graph_file) {
    THROW("Fail to load config file {}", frame_graph_path.string());
  }
  json_ = json::parse(frame_graph_file);

  if (json_.contains("enabled_scenes")) {
    const json& enabled_scenes_json{json_["enabled_scenes"]};
    for (const auto& enabled_scene_json : enabled_scenes_json) {
      EnabledScene enabled_scene{};

      if (enabled_scene_json.contains("index")) {
        enabled_scene.index = enabled_scene_json["index"].template get<u32>();
      }

      glm::vec3 scale{1.0F, 1.0F, 1.0F};
      glm::quat rotation{1.0F, 0.0F, 0.0F, 0.0F};
      glm::vec3 translation{0.0F, 0.0F, 0.0F};
      if (enabled_scene_json.contains("scale")) {
        const json& scale_json{enabled_scene_json["scale"]};
        std::vector<f32> scale_vector{
            scale_json.template get<std::vector<f32>>()};
        if (scale_vector.size() != 3) {
          THROW("Unknown scale");
        }
        scale = glm::vec3{scale_vector[0], scale_vector[1], scale_vector[2]};
      }

      if (enabled_scene_json.contains("rotation")) {
        const json& rotation_json{enabled_scene_json["rotation"]};
        std::vector<f32> rotation_vector{
            rotation_json.template get<std::vector<f32>>()};
        if (rotation_vector.size() != 4) {
          THROW("Unknown rotation");
        }
        rotation = glm::quat{rotation_vector[0], rotation_vector[1],
                             rotation_vector[2], rotation_vector[3]};
      }

      if (enabled_scene_json.contains("translation")) {
        const json& translation_json{enabled_scene_json["translation"]};
        std::vector<f32> translation_vector{
            translation_json.template get<std::vector<f32>>()};
        if (translation_vector.size() != 3) {
          THROW("Unknown translation");
        }
        translation = glm::vec3{translation_vector[0], translation_vector[1],
                                translation_vector[2]};
      }

      enabled_scene.model = glm::translate(glm::mat4(1.0F), translation) *
                            glm::mat4_cast(rotation) *
                            glm::scale(glm::mat4(1.0F), scale);

      enabled_scenes_.push_back(enabled_scene);
    }
  }

  if (json_.contains("passes")) {
    const json& passes_json{json_["passes"]};
    for (const auto& pass_json : passes_json) {
      ast::Pass pass{};
      // Pass name.
      if (pass_json.contains("name")) {
        pass.name = pass_json["name"].template get<std::string>();
      }

      // Pass type.
      if (pass_json.contains("type")) {
        std::string type{pass_json["type"].template get<std::string>()};
        if (type == "graphics") {
          pass.type = PassType::kGraphics;
        } else if (type == "compute") {
          pass.type = PassType::kCompute;
        } else {
          THROW("Unkonwn pass type");
        }
      }

      // Pass attachments.
      if (pass_json.contains("attachments")) {
        const json& attachments_json{pass_json["attachments"]};

        for (const auto& attachment_json : attachments_json) {
          std::string name;
          if (attachment_json.contains("name")) {
            name = attachment_json["name"].template get<std::string>();
          }
          vk::Format format{vk::Format::eR8G8B8A8Unorm};
          if (attachment_json.contains("format")) {
            std::string fmt{
                attachment_json["format"].template get<std::string>()};

            if (fmt == "R8Unorm") {
              format = vk::Format::eR8Unorm;
            } else if (fmt == "R8G8Unorm") {
              format = vk::Format::eR8G8Unorm;
            } else if (fmt == "R8G8B8Unorm") {
              format = vk::Format::eR8G8B8Unorm;
            } else if (fmt == "R8G8B8A8Unorm") {
              format = vk::Format::eR8G8B8A8Unorm;
            } else if (fmt == "A2R10G10B10UnormPack32") {
              format = vk::Format::eA2R10G10B10UnormPack32;
            } else if (fmt == "D32Sfloat") {
              format = vk::Format::eD32Sfloat;
            } else {
              THROW("Unkonwn attachment formt");
            }
          }

          bool output{};
          if (attachment_json.contains("output")) {
            output = attachment_json["output"].template get<bool>();
          }
          pass.attachments.push_back(Attachment{name, format, output});
        }
      }

      // Pase subpasses.
      if (pass.type == PassType::kGraphics && pass_json.contains("subpasses")) {
        const json& subpasses_json{pass_json["subpasses"]};
        for (const json& subpass_json : subpasses_json) {
          ast::Subpass subpass{};

          if (subpass_json.contains("name")) {
            subpass.name = subpass_json["name"].template get<std::string>();
          }

          if (subpass_json.contains("attachments")) {
            const json& attachments_json{subpass_json["attachments"]};
            if (attachments_json.contains("inputs")) {
              std::vector<u32> input_attachments;
              const json& input_jsons{attachments_json["inputs"]};
              for (const auto& input_json : input_jsons) {
                u32 index{input_json.template get<u32>()};
                input_attachments.push_back(index);
              }
              subpass.attachments.emplace(AttachmentType::kInput,
                                          input_attachments);
            }
            if (attachments_json.contains("colors")) {
              std::vector<u32> color_attachments;
              const json& colors_json{attachments_json["colors"]};
              for (const auto& color_json : colors_json) {
                u32 index{color_json.template get<u32>()};
                color_attachments.push_back(index);
              }
              subpass.attachments.emplace(AttachmentType::kColor,
                                          color_attachments);
            }

            if (attachments_json.contains("depth_stencil")) {
              const json& depth_stencil_json{attachments_json["depth_stencil"]};
              u32 index{depth_stencil_json.template get<u32>()};
              subpass.attachments.emplace(AttachmentType::kDepthStencil,
                                          std::vector{index});
            }
          }

          if (subpass_json.contains("shaders")) {
            const json& shaders_json{subpass_json["shaders"]};
            if (shaders_json.contains("vertex")) {
              u32 index{shaders_json["vertex"].template get<u32>()};
              subpass.shaders.emplace(vk::ShaderStageFlagBits::eVertex, index);
            }

            if (shaders_json.contains("fragment")) {
              u32 index{shaders_json["fragment"].template get<u32>()};
              subpass.shaders.emplace(vk::ShaderStageFlagBits::eFragment,
                                      index);
            }
          }

          if (subpass_json.contains("scene")) {
            subpass.scene = subpass_json["scene"].template get<std::string>();
          }

          if (subpass_json.contains("lights")) {
            const json& lights_json{subpass_json["lights"]};
            for (const auto& light_json : lights_json) {
              u32 index{light_json.template get<u32>()};
              subpass.lights.push_back(index);
            }
          }

          pass.subpasses.push_back(std::move(subpass));
        }
      }

      // Pass compute job
      if (pass.type == PassType::kCompute &&
          pass_json.contains("compute_job")) {
        ComputeJob compute_job{};
        const json& compute_job_json{pass_json["compute_job"]};
        if (compute_job_json.contains("shader")) {
          u32 shader{compute_job_json["shader"].template get<u32>()};
          compute_job.shader = shader;
        }
        pass.compute_job = compute_job;
      }

      passes_.push_back(pass);
    }
  }
}

const std::vector<EnabledScene>& FrameGraph::GetEnabledScenes() const {
  return enabled_scenes_;
}

const std::vector<ast::Pass>& FrameGraph::GetPasses() const { return passes_; }

}  // namespace luka::ast
