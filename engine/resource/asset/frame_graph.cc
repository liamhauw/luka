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
    const json& pass_jsons{json_["passes"]};
    for (const auto& pass_json : pass_jsons) {
      ast::Pass pass;
      // Pass name.
      if (pass_json.contains("name")) {
        pass.name = pass_json["name"].template get<std::string>();
      }

      // Pass attachments.
      if (pass_json.contains("attachments")) {
        const json& attachment_jsons{pass_json["attachments"]};
        if (attachment_jsons.contains("color")) {
          const json& color_jsons{attachment_jsons["color"]};
          std::vector<AttachmentInfo> infos;

          for (const auto& color_json : color_jsons) {
            AttachmentInfo info;
            if (color_json.contains("name")) {
              info.name = color_json["name"].template get<std::string>();
            }
            infos.push_back(std::move(info));
          }

          pass.attachments.emplace(AttachmentType::kColor, std::move(infos));
        }

        if (attachment_jsons.contains("resolve")) {
          const json& resolve_json{attachment_jsons["depth_stencil"]};
          std::vector<AttachmentInfo> infos(1);
          if (resolve_json.contains("name")) {
            infos[0].name = resolve_json["name"].template get<std::string>();
          }
          pass.attachments.emplace(AttachmentType::kResolve, std::move(infos));
        }

        if (attachment_jsons.contains("depth_stencil")) {
          const json& depth_stencil_json{attachment_jsons["depth_stencil"]};
          std::vector<AttachmentInfo> infos(1);
          if (depth_stencil_json.contains("name")) {
            infos[0].name =
                depth_stencil_json["name"].template get<std::string>();
          }
          pass.attachments.emplace(AttachmentType::kDepthStencil,
                                   std::move(infos));
        }
      }

      // Pase subpasses.
      if (pass_json.contains("subpasses")) {
        const json& subpass_jsons{pass_json["subpasses"]};
        for (const json& subpass_json : subpass_jsons) {
          ast::Subpass subpass;

          if (subpass_json.contains("name")) {
            subpass.name = subpass_json["name"].template get<std::string>();
          }

          if (subpass_json.contains("scenes")) {
            const json& scene_jsons{subpass_json["scenes"]};
            for (const auto& scene_json : scene_jsons) {
              u32 scene{scene_json.template get<u32>()};
              subpass.scenes.push_back(scene);
            }
          }

          if (subpass_json.contains("shaders")) {
            const json& shader_jsons{subpass_json["shaders"]};
            if (shader_jsons.contains("vertex")) {
              u32 shader{shader_jsons["vertex"].template get<u32>()};
              subpass.shaders.emplace("vertex", shader);
            }
            if (shader_jsons.contains("fragment")) {
              u32 shader{shader_jsons["fragment"].template get<u32>()};
              subpass.shaders.emplace("fragment", shader);
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
