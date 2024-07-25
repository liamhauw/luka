// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#version 450

#include "../include/defination.glsl"

#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0) uniform Subpass {
  SubpassUniform subpass_uniform;
};
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
#include "../include/get_geometry_attribute.glsl"
  vec3 position = i_position;

#include "../include/lighting.glsl"


  o_color = vec4(Lo, 1.0);
}
