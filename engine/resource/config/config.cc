/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#include "resource/config/config.h"

#include <fstream>

#include "context.h"
#include "core/log.h"

namespace luka {

Config::Config() {
  std::ifstream config_file{config_file_path_.string()};
  if (!config_file) {
    THROW("Fail to load config file {}", config_file_path_.string());
  }
  cj_ = json::parse(config_file);

  // Load scene.
  json::object_t cj{cj_};
  scene_ = GetElementUint32(cj, "scene", 0);
}

void Config::Tick() {
  if (gContext.load) {
    const json& scene{cj_["scenes"][scene_]};

    json::object_t scene_object{scene};

    // Load model
    std::string model_name{
        ReplacePathSlash(GetElementString(scene_object, "model", ""))};
    model_file_path_ = model_path_ / model_name;

    // Load camera.
    camera_from_ = GetElementVec4(scene_object, "camera/from",
                                  glm::vec4{0.0f, 0.0f, 10.0f, 1.0f});
    camera_to_ = GetElementVec4(scene_object, "camera/to",
                                glm::vec4{0.0f, 0.0f, 0.0f, 1.0f});
  }
}

const std::filesystem::path& Config::GetModelFilePath() const {
  return model_file_path_;
}

const glm::vec4& Config::GetCameraFrom() const { return camera_from_; }

const glm::vec4& Config::GetCameraTo() const { return camera_to_; }

}  // namespace luka