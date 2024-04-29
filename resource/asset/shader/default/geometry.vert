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

layout(set = 2, binding = 0) uniform DrawElementUniform {
  mat4 m;
  mat4 inverse_m;
  vec4 base_color_factor;
  uvec4 sampler_indices;
  uvec4 image_indices;
  float metallic_factor;
  float roughness_factor;
  int alpha_model;
  float alpha_cutoff;
}
draw_element_uniform;

layout(location = 0) in vec3 position;

layout(location = 1) in vec3 normal;
layout(location = 1) out vec3 o_normal;

#ifdef HAS_TEXCOORD_0_BUFFER
layout(location = 2) in vec2 texcoord_0;
layout(location = 2) out vec2 o_texcoord_0;
#endif

#ifdef HAS_TANGENT_BUFFER
layout(location = 3) in vec3 tangent;
layout(location = 3) out vec3 o_tangent;
#endif

void main(void) {
  gl_Position =
      subpass_uniform.pv * draw_element_uniform.m * vec4(position, 1.0);

  mat3 ti_m = transpose(mat3(draw_element_uniform.inverse_m));
  o_normal = ti_m * normal;

#ifdef HAS_TEXCOORD_0_BUFFER
  o_texcoord_0 = texcoord_0;
#endif

#ifdef HAS_TANGENT_BUFFER
  o_tangent = ti_m * tangent;
#endif
}
