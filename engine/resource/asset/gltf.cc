/*
  SPDX license identifier: MIT
  Copyright (C) 2023 Liam Hauw
*/

#include "resource/asset/gltf.h"

#include <fstream>

#include "core/log.h"

namespace luka {

std::filesystem::path parent_path;
const json* accessors{nullptr};
const json* buffer_views{nullptr};
const std::vector<GltfBuffer>* buffers{nullptr};

uint32_t GetCompoentSize(uint32_t componet_type) {
  switch (componet_type) {
    case 5120:
      return 1;  //(BYTE)1
    case 5121:
      return 1;  //(UNSIGNED_BYTE)1
    case 5122:
      return 2;  //(SHORT)2
    case 5123:
      return 2;  //(UNSIGNED_SHORT)2
    case 5124:
      return 4;  //(SIGNED_INT)4
    case 5125:
      return 4;  //(UNSIGNED_INT)4
    case 5126:
      return 4;  //(FLOAT)4
  }
  return UINT32_MAX;
}

uint32_t GetDimensionSize(const std::string& type) {
  if (type == "SCALAR")
    return 1;
  else if (type == "VEC2")
    return 2;
  else if (type == "VEC3")
    return 3;
  else if (type == "VEC4")
    return 4;
  else if (type == "MAT4")
    return 4 * 4;
  else
    return UINT32_MAX;
}

void GetBufferDetails(uint32_t accessor, GltfAccessor* gltf_accessor) {
  const json& aj{accessors->at(accessor)};
  uint32_t buffer_view{aj.value("bufferView", UINT32_MAX)};
  uint32_t acc_byte_offset{aj.value("byteOffset", UINT32_MAX)};
  uint32_t componet_type{aj.value("componentType", UINT32_MAX)};
  uint32_t count{aj.value("count", UINT32_MAX)};
  std::string type{aj.value("type", "")};

  const json& bvj{buffer_views->at(buffer_view)};

  uint32_t buffer{bvj.value("buffer", UINT32_MAX)};
  uint32_t byte_length{bvj.value("byteLength", UINT32_MAX)};
  uint32_t byte_offset{bvj.value("byteOffset", UINT32_MAX)};

  const char* buffer_data{buffers->at(buffer).bin.data()};
  byte_offset += acc_byte_offset;
  byte_length -= acc_byte_offset;

  gltf_accessor->data = &buffer_data[byte_offset];
  gltf_accessor->dimension_size = GetDimensionSize(type);
  gltf_accessor->componet_size = GetCompoentSize(componet_type);
  gltf_accessor->stride =
      gltf_accessor->dimension_size * gltf_accessor->componet_size;
  gltf_accessor->count = count;
}

void from_json(const json& j, GltfScene& scene) {
  if (j.contains("name")) {
    j.at("name").get_to(scene.name);
  }
  if (j.contains("nodes")) {
    j.at("nodes").get_to(scene.nodes);
  }
}

void from_json(const json& j, GltfNode& gltf_node) {
  if (j.contains("name")) {
    j.at("name").get_to(gltf_node.name);
  }
  if (j.contains("children")) {
    j.at("children").get_to(gltf_node.children);
  }
  if (j.contains("mesh")) {
    j.at("mesh").get_to(gltf_node.mesh);
  }
  if (j.contains("skin")) {
    j.at("skin").get_to(gltf_node.skin);
  }
  if (j.contains("scale")) {
    j.at("scale").get_to(gltf_node.scale);
  }
  if (j.contains("rotation")) {
    j.at("rotation").get_to(gltf_node.rotation);
  }
  if (j.contains("translation")) {
    j.at("translation").get_to(gltf_node.translation);
  }
  if (j.contains("matrix")) {
    j.at("matrix").get_to(gltf_node.matrix);
  }
}

void from_json(const json& j, GltfPrimitive& gltf_primitive) {
  if (j.contains("attributes") && j.at("attributes").contains("POSITION") &&
      accessors) {
    uint32_t posotion{
        j.at("attributes").at("POSITION").template get<uint32_t>()};
    const json& aj{accessors->at(posotion)};

    glm::vec4 max, min;
    if (aj.contains("max") && aj.contains("min")) {
      max = aj.at("max").template get<glm::vec4>();
      min = aj.at("min").template get<glm::vec4>();
    };

    gltf_primitive.center = (max + min) * 0.5f;
    gltf_primitive.radius = max - gltf_primitive.center;
    gltf_primitive.center.w = 1.0f;
  }
}

void from_json(const json& j, GltfMesh& gltf_mesh) {
  if (j.contains("primitives")) {
    j.at("primitives").get_to(gltf_mesh.primitives);
  }
}

void from_json(const json& j, GltfBuffer& buffer) {
  if (j.contains("byteLength") && j.contains("uri")) {
    uint32_t byte_length{j.at("byteLength").template get<uint32_t>()};

    std::string uri{j.at("uri").template get<std::string>()};
    std::string data_file_path{(parent_path / uri).string()};
    std::ifstream bin_file{data_file_path, std::ios::in | std::ios::binary};
    bin_file.seekg(0, bin_file.beg);

    buffer.bin.resize(byte_length);
    bin_file.read(buffer.bin.data(), byte_length);
  }
}

void from_json(const json& j, GltfAnimation& gltf_animation) {
  if (j.contains("channels") && j.contains("samplers")) {
    const json& cj{j.at("channels")};
    const json& sj{j.at("samplers")};

    for (uint32_t i{0}; i < cj.size(); ++i) {
      const json& channel{cj[i]};

      if (channel.contains("sampler") && channel.contains("target") &&
          channel.at("target").contains("node") &&
          channel.at("target").contains("path")) {
        uint32_t sampler{channel.at("sampler").template get<uint32_t>()};
        uint32_t node{channel.at("target").at("node").template get<uint32_t>()};
        std::string path{
            channel.at("target").at("path").template get<std::string>()};

        GltfChannel* gltf_channel;
        auto iter{gltf_animation.node2channel.find(node)};
        if (iter == gltf_animation.node2channel.end()) {
          gltf_channel = &gltf_animation.node2channel[node];
        } else {
          gltf_channel = &iter->second;
        }

        GltfSampler gltf_sampler;
        GetBufferDetails(sj.at(sampler).at("input").template get<uint32_t>(),
                         &gltf_sampler.time_accessor);

        gltf_animation.duration = std::max(
            gltf_animation.duration, *(float*)gltf_sampler.time_accessor.Get(
                                         gltf_sampler.time_accessor.count - 1));

        GetBufferDetails(sj[sampler]["output"].get<uint32_t>(),
                         &gltf_sampler.value_accessor);

        if (path == "scale") {
          gltf_channel->scale = gltf_sampler;
        } else if (path == "rotation") {
          gltf_channel->rotation = gltf_sampler;
        } else if (path == "translation") {
          gltf_channel->translation = gltf_sampler;
        }
      }
    }
  }
}

void from_json(const json& j, GltfInfo& gltf_gltf) {
  if (j.contains("accessors")) {
    accessors = &j.at("accessors");
  }
  if (j.contains("bufferViews")) {
    buffer_views = &j.at("bufferViews");
  }
  if (j.contains("scene")) {
    j.at("scene").get_to(gltf_gltf.scene);
  }
  if (j.contains("scenes")) {
    j.at("scenes").get_to(gltf_gltf.scenes);
  }
  if (j.contains("nodes")) {
    j.at("nodes").get_to(gltf_gltf.nodes);
  }
  if (j.contains("meshes")) {
    j.at("meshes").get_to(gltf_gltf.meshes);
  }
  if (j.contains("buffers")) {
    j.at("buffers").get_to(gltf_gltf.buffers);
    buffers = &(gltf_gltf.buffers);
  }
  if (j.contains("animations")) {
    j.at("animations").get_to(gltf_gltf.animations);
  }
}

Gltf::Gltf(const std::filesystem::path& model_file_path) {
  // Parse model json.
  parent_path = model_file_path.parent_path();

  std::ifstream model_file{model_file_path.string()};
  if (!model_file) {
    THROW("Fail to open {}", model_file_path.string());
  }
  j_ = json::parse(model_file);

  j_.get_to(gltf_info_);
}

}  // namespace luka
