// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "core/json.h"
#include "core/math.h"

namespace luka {

struct GltfScene {
  std::string name;
  std::vector<u32> nodes;
};

struct GltfNode {
  std::string name;
  u32 skin;
  u32 mesh;
  glm::vec4 scale;
  glm::vec4 rotation;
  glm::vec4 translation;
  glm::mat4 matrix;
  std::vector<u32> children;
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
  u32 count{1};
  u32 stride;
  u32 dimension_size;
  u32 componet_size;
  glm::vec4 min;
  glm::vec4 max;

  const void* Get(u32 i) const {
    if (i >= count) {
      i = count - 1;
    }
    return reinterpret_cast<const char*>(data) + stride * i;
  }

  u32 FindClosestFloatIndex(f32 val) const {
    if (val < 0.0f) {
      return 0;
    }

    u32 ini{0};
    u32 fin{count - 1};

    while (ini <= fin) {
      u32 mid{(ini + fin) / 2};
      f32 v{*reinterpret_cast<const f32*>(Get(mid))};

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

  void SampleLinear(f32 time, f32* frac, f32** cur, f32** next) const {
    u32 cur_index{time_accessor.FindClosestFloatIndex(time)};
    u32 next_index = std::min<u32>(cur_index + 1, time_accessor.count - 1);

    *cur = (f32*)value_accessor.Get(cur_index);
    *next = (f32*)value_accessor.Get(next_index);

    if (cur_index == next_index) {
      *frac = 0;
    } else {
      f32 cur_time{*(f32*)(time_accessor.Get(cur_index))};
      f32 next_time{*(f32*)(time_accessor.Get(next_index))};
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
  f32 duration;
  std::map<u32, GltfChannel> node2channel;
};

struct GltfInfo {
  u32 scene;
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
