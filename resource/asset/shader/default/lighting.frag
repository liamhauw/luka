// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#version 450

layout(set = 0, binding = 0) uniform SubpassUniform {
  mat4 pv;
  mat4 inverse_vp;
  vec4 camera_position;
  vec4 light_position;
}
subpass_uniform;

layout(input_attachment_index = 0, set = 0, binding = 1) uniform subpassInput subpass_i_base_color;
layout(input_attachment_index = 1, set = 0, binding = 2) uniform subpassInput subpass_i_normal;
layout(input_attachment_index = 2, set = 0, binding = 3) uniform subpassInput subpass_i_depth;

layout(location = 0) in vec2 i_texcoord_0;
layout(location = 0) out vec4 o_color;

void main() {
  vec4 base_color = subpassLoad(subpass_i_base_color);
  vec3 normal = subpassLoad(subpass_i_normal).xyz;
  normal = normalize(2.0 * normal - 1.0);
  float depth = subpassLoad(subpass_i_depth).x;

  vec4 clip = vec4(i_texcoord_0 * 2.0 - 1.0, depth, 1.0);
  vec4 world = subpass_uniform.inverse_vp * clip;
  vec3 pos = world.xyz / world.w;

  vec3 light_vector = normalize(subpass_uniform.light_position.xyz - pos);
  vec3 camera_vector = normalize(subpass_uniform.camera_position.xyz - pos);
  vec3 reflection_vector = reflect(subpass_uniform.light_position.xyz, normal);

  vec3 diffuse = max(dot(normal, light_vector), 0.6) * base_color.xyz;
  vec3 specular = pow(max(dot(reflection_vector, camera_vector), 0.0), 1.0) * vec3(0.01);
  
  o_color = vec4(diffuse * base_color.xyz, 1.0);
}