// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#version 450

#include "common.glsl"

#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 1, binding = 0) uniform sampler bindless_samplers[];
layout(set = 1, binding = 1) uniform texture2D bindless_images[];

layout(set = 2, binding = 0) uniform DrawElement {
  DrawElementUniform draw_element_uniform;
};

layout(location = 0) in vec3 i_position;

layout(location = 1) in vec3 i_normal;

#if defined(HAS_TANGENT_BUFFER)
layout(location = 2) in vec3 i_tangent;
#endif

#if defined(HAS_TEXCOORD_0_BUFFER)
layout(location = 3) in vec2 i_texcoord_0;
#endif

layout(location = 0) out vec4 o_color;

void main(void) {
  // Base color.
  vec4 base_color = draw_element_uniform.base_color_factor;
#if defined(HAS_BASE_COLOR_TEXTURE)
  vec4 base_color_texel =
      texture(nonuniformEXT(sampler2D(
                  bindless_images[draw_element_uniform.image_indices_0.x],
                  bindless_samplers[draw_element_uniform.sampler_indices_0.x])),
              i_texcoord_0);
  base_color = vec4(pow(base_color_texel.rgb, vec3(2.2)), base_color_texel.a);
#endif

  o_color = base_color;
}
