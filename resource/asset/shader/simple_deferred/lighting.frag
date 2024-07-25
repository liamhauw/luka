// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#version 450

#include "../include/defination.glsl"

layout(set = 0, binding = 0) uniform Subpass {
  SubpassUniform subpass_uniform;
};

layout(input_attachment_index = 0, set = 0,
       binding = 1) uniform subpassInput subpass_i_base_color;
layout(input_attachment_index = 1, set = 0,
       binding = 2) uniform subpassInput subpass_i_metallic_roughness;
layout(input_attachment_index = 2, set = 0,
       binding = 3) uniform subpassInput subpass_i_normal;
layout(input_attachment_index = 3, set = 0,
       binding = 4) uniform subpassInput subpass_i_occlusion;
layout(input_attachment_index = 4, set = 0,
       binding = 5) uniform subpassInput subpass_i_emissive;
layout(input_attachment_index = 5, set = 0,
       binding = 6) uniform subpassInput subpass_i_depth;

layout(location = 0) in vec2 i_texcoord_0;
layout(location = 0) out vec4 o_color;

void main() {
  // Base color.
  vec4 base_color = subpassLoad(subpass_i_base_color);

  // Metallic and roughness.
  vec2 metallic_roughness = subpassLoad(subpass_i_metallic_roughness).xy;
  float metallic = metallic_roughness.x;
  float roughness = metallic_roughness.y;

  // Normal.
  vec3 normal = subpassLoad(subpass_i_normal).xyz;
  normal = normalize(2.0 * n - 1.0);

  // Occlusion.
  float occlusion = subpassLoad(subpass_i_occlusion).x;

  // Emissive.
  vec4 emissive = subpassLoad(subpass_i_emissive);

  // Position.
  float depth = subpassLoad(subpass_i_depth).x;
  vec4 clip = vec4(i_texcoord_0 * 2.0 - 1.0, depth, 1.0);
  vec4 pos_w = subpass_uniform.inverse_pv * clip;
  vec3 position = pos_w.xyz / pos_w.w;

#include "../include/lighting.glsl"

  o_color = vec4(Lo, 1.0);
}