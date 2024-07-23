// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#version 450

#include "common.glsl"

layout(set = 0, binding = 0) uniform Subpass {
  SubpassUniform subpass_uniform;
};

layout(set = 2, binding = 0) uniform DrawElement {
  DrawElementUniform draw_element_uniform;
};

layout(location = 0) in vec3 position;
layout(location = 0) out vec3 o_position;

layout(location = 1) in vec3 normal;
layout(location = 1) out vec3 o_normal;

#if defined(HAS_TANGENT_BUFFER)
layout(location = 2) in vec3 tangent;
layout(location = 2) out vec3 o_tangent;
#endif

#if defined(HAS_TEXCOORD_0_BUFFER)
layout(location = 3) in vec2 texcoord_0;
layout(location = 3) out vec2 o_texcoord_0;
#endif

void main(void) {
  vec4 world_position = draw_element_uniform.m * vec4(position, 1.0);
  o_position = world_position.xyz / world_position.w;
  gl_Position = subpass_uniform.pv * world_position;

  mat3 ti_m = transpose(mat3(draw_element_uniform.inverse_m));
  o_normal = ti_m * normal;

#if defined(HAS_TANGENT_BUFFER)
  o_tangent = ti_m * tangent;
#endif

#if defined(HAS_TEXCOORD_0_BUFFER)
  o_texcoord_0 = texcoord_0;
#endif
}
