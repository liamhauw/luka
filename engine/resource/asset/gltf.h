/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "core/json.h"
#include "core/math.h"

namespace luka {

struct GltfScene {
  std::string name;
  std::vector<uint32_t> nodes;
};

struct GltfNode {
  std::string name;
  uint32_t skin;
  uint32_t mesh;
  glm::vec4 scale;
  glm::vec4 rotation;
  glm::vec4 translation;
  glm::mat4 matrix;
  std::vector<uint32_t> children;
};

struct GltfPrimitive {
  glm::vec4 center;
  glm::vec4 radius;
};

struct GltfMesh {
  std::vector<GltfPrimitive> primitives;
};

struct GltfBuffer {
  std::vector<char> bin;
};

struct GltfAccessor {
  const void* data{nullptr};
  uint32_t count{1};
  uint32_t stride;
  uint32_t dimension_size;
  uint32_t componet_size;
  glm::vec4 min;
  glm::vec4 max;

  const void* Get(uint32_t i) const {
    if (i >= count) {
      i = count - 1;
    }
    return reinterpret_cast<const char*>(data) + stride * i;
  }

  uint32_t FindClosestFloatIndex(float val) const {
    if (val < 0.0f) {
      return 0;
    }

    uint32_t ini{0};
    uint32_t fin{count - 1};

    while (ini <= fin) {
      uint32_t mid{(ini + fin) / 2};
      float v{*reinterpret_cast<const float*>(Get(mid))};

      if (val < v)
        fin = mid - 1;
      else if (val > v)
        ini = mid + 1;
      else
        return mid;
    }

    return fin;
  }
};

struct GltfSampler {
  GltfAccessor time_accessor;
  GltfAccessor value_accessor;

  void SampleLinear(float time, float* frac, float** cur, float** next) const {
    uint32_t cur_index{time_accessor.FindClosestFloatIndex(time)};
    uint32_t next_index =
        std::min<uint32_t>(cur_index + 1, time_accessor.count - 1);

    *cur = (float*)value_accessor.Get(cur_index);
    *next = (float*)value_accessor.Get(next_index);

    if (cur_index == next_index) {
      *frac = 0;
    } else {
      float cur_time{*(float*)(time_accessor.Get(cur_index))};
      float next_time{*(float*)(time_accessor.Get(next_index))};
      *frac = (time - cur_time) / (next_time / cur_time);
    }
  }
};

struct GltfChannel {
  GltfSampler scale;
  GltfSampler rotation;
  GltfSampler translation;
};

struct GltfAnimation {
  float duration;
  std::map<uint32_t, GltfChannel> node2channel;
};

struct GltfInfo {
  uint32_t scene;
  std::vector<GltfScene> scenes;
  std::vector<GltfNode> nodes;
  std::vector<GltfMesh> meshes;
  std::vector<GltfBuffer> buffers;
  std::vector<GltfAnimation> animations;
};

class Gltf {
 public:
  Gltf(const std::filesystem::path& model_file_path);

 private:
  json j_;
  GltfInfo gltf_info_;
};
}  // namespace luka
