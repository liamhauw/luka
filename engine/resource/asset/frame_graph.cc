// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/frame_graph.h"

#include "core/log.h"

namespace luka {

namespace ast {

FrameGraph::FrameGraph(const std::filesystem::path& frame_graph_path) {
  std::ifstream frame_graph_file{frame_graph_path.string()};
  if (!frame_graph_file) {
    THROW("Fail to load config file {}", frame_graph_path.string());
  }
  json_ = json::parse(frame_graph_file);

  if (json_.contains("passes")) {
    const json& passes_json{json_["passes"]};
    for (const auto& pass_json : passes_json) {
      ast::Pass pass;
      // Pass name.
      if (pass_json.contains("name")) {
        pass.name = pass_json["name"].template get<std::string>();
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
            if (fmt == "R8B8B8A8Unorm") {
              format = vk::Format::eR8G8B8A8Unorm;
            } else if (fmt == "A2R10G10B10UnormPack32") {
              format = vk::Format::eA2R10G10B10UnormPack32;
            } else if (fmt == "D32Sfloat") {
              format = vk::Format::eD32Sfloat;
            }
          }
          pass.attachments.emplace_back(name, format);
        }
      }

      // Pase subpasses.
      if (pass_json.contains("subpasses")) {
        const json& subpasses_json{pass_json["subpasses"]};
        for (const json& subpass_json : subpasses_json) {
          ast::Subpass subpass;

          if (subpass_json.contains("name")) {
            subpass.name = subpass_json["name"].template get<std::string>();
          }

          if (subpass_json.contains("attachments")) {
            const json& attachments_json{subpass_json["attachments"]};
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

          if (subpass_json.contains("scenes")) {
            const json& scenes_json{subpass_json["scenes"]};
            for (const auto& scene_json : scenes_json) {
              u32 index{scene_json.template get<u32>()};
              subpass.scenes.push_back(index);
            }
          }

          if (subpass_json.contains("shaders")) {
            const json& shaders_json{subpass_json["shaders"]};
            if (shaders_json.contains("vertex")) {
              u32 index{shaders_json["vertex"].template get<u32>()};
              subpass.shaders.emplace(ShaderType::kVertex, index);
            }

            if (shaders_json.contains("fragment")) {
              u32 index{shaders_json["fragment"].template get<u32>()};
              subpass.shaders.emplace(ShaderType::kFragment, index);
            }
          }
          pass.subpasses.push_back(std::move(subpass));
        }
      }
      passes_.push_back(pass);
    }
  }
}

}  // namespace ast

}  // namespace luka
