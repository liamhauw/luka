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
  vec4 base_color_factor;
  uvec4 texture_indices;
}
draw_element_uniform;

layout(location = 0) in vec3 position;

#ifdef HAS_TEXCOORD_0_BUFFER
layout(location = 1) in vec2 texcoord_0;
layout(location = 1) out vec2 o_texcoord_0;
#endif

#ifdef HAS_NORMAL_BUFFER
layout(location = 2) in vec3 normal;
layout(location = 2) out vec3 o_normal;
#endif

void main(void) {
#ifdef HAS_TEXCOORD_0_BUFFER
  o_texcoord_0 = texcoord_0;
#endif

#ifdef HAS_NORMAL_BUFFER
  o_normal = mat3(draw_element_uniform.m) * normal;
#endif

  gl_Position =
      subpass_uniform.pv * draw_element_uniform.m * vec4(position, 1.0);
}
