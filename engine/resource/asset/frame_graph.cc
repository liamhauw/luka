// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/frame_graph.h"

namespace luka {

namespace ast {

FrameGraph::FrameGraph(const std::filesystem::path& frame_graph_path) {
   // if (config_json_.contains("subpasses")) {
  //   const json& subpass_jsons{config_json_["subpasses"]};
  //   for (const json& subpass_json : subpass_jsons) {
  //     cfg::Subpass subpass;
  //     if (subpass_json.contains("name")) {
  //       subpass.name = subpass_json["name"].template get<std::string>();
  //     }
  //     if (subpass_json.contains("scenes")) {
  //       const json& scene_jsons{subpass_json["scenes"]};
  //       for (const auto& scene_json : scene_jsons) {
  //         u32 scene{scene_json.template get<u32>()};
  //         subpass.scenes.push_back(scene);
  //       }
  //     }
  //     if (subpass_json.contains("shaders")) {
  //       const json& shader_jsons{subpass_json["shaders"]};
  //       if (shader_jsons.contains("vertex")) {
  //         u32 shader{shader_jsons["vertex"].template get<u32>()};
  //         subpass.shaders.emplace("vertex", shader);
  //       }
  //       if (shader_jsons.contains("fragment")) {
  //         u32 shader{shader_jsons["fragment"].template get<u32>()};
  //         subpass.shaders.emplace("fragment", shader);
  //       }
  //     }
  //     subpasses_.push_back(std::move(subpass));
  //   }
  // }

  // if (config_json_.contains("passes")) {
  //   const json& pass_jsons{config_json_["passes"]};
  //   for (const auto& pass_json : pass_jsons) {
  //     cfg::Pass pass;
  //     if (pass_json.contains("name")) {
  //       pass.name = pass_json["name"].template get<std::string>();
  //     }
  //     if (pass_json.contains("subpasses")) {
  //       const json& subpass_jsons{pass_json["subpasses"]};
  //       for (const json& subpass_json : subpass_jsons) {
  //         u32 subpass{subpass_json.template get<u32>()};
  //         pass.subpasses.push_back(subpass);
  //       }
  //     }
  //     passes_.push_back(pass);
  //   }
  // }

  // if (config_json_.contains("frame_graph")) {
  //   const json& frame_graph_jsons{config_json_["frame_graph"]};
  //   for (const auto& frame_graph_json : frame_graph_jsons) {
  //     cfg::FrameGraph frame_graph;
  //     if (frame_graph_json.contains("passes")) {
  //       const json& pass_jsons{frame_graph_json["passes"]};
  //       for (const json& pass_json : pass_jsons) {
  //         u32 pass{pass_json.template get<u32>()};
  //         frame_graph.passes.push_back(pass);
  //       }
  //     }
  //     frame_graphs_.push_back(std::move(frame_graph));
  //   }
  // }

  // if (config_json_.contains("frame_graph")) {
  //   frame_graph_ = config_json_["frame_graph"].template get<u32>();
  // }
}

}  // namespace ast

}  // namespace luka
