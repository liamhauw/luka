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

layout(set = 1, binding = 0) uniform sampler global_samplers[];
layout(set = 1, binding = 1) uniform texture2D global_images[];


layout(set = 2, binding = 0) uniform sampler2D color;

layout(location = 0) in vec2 i_texcoord_0;
layout(location = 0) out vec4 o_color;

void main() { o_color = texture(color, i_texcoord_0); }