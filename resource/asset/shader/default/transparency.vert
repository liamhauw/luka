// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#version 450

#include "common.glsl"

layout(set = 0, binding = 0) uniform Subpass {
  SubpassUniform subpass_uniform;
};

layout(set = 1, binding = 0) uniform DrawElement {
  DrawElementUniform draw_element_uniform;
};

layout(location = 0) in vec3 position;

void main(void) {
  vec4 world_position = draw_element_uniform.m * vec4(position, 1.0);
  gl_Position = subpass_uniform.pv * world_position;
}