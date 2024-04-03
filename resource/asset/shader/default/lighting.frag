// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#version 450

layout(set = 0, binding = 0) uniform SubpassUniform {
  mat4 pv;
  mat4 inverse_pv;
  vec4 camera_position;
  vec4 light_position;
}
subpass_uniform;

layout(input_attachment_index = 0, set = 0, binding = 1) uniform subpassInput i_base_color;
layout(input_attachment_index = 1, set = 0, binding = 2) uniform subpassInput i_normal;
layout(input_attachment_index = 2, set = 0, binding = 3) uniform subpassInput i_depth;

layout(location = 0) in vec2 i_texcoord_0;
layout(location = 0) out vec4 o_color;

void main() {
  vec4 clip = vec4(i_texcoord_0 * 2.0 - 1.0, subpassLoad(i_depth).x, 1.0);
  vec4 world = inverse(subpass_uniform.pv) * clip;
  vec3 pos = world.xyz / world.w;
  // o_color = vec4(pos, 1.0);
  vec4 base_color = subpassLoad(i_base_color);
  o_color = base_color;
}