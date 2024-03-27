// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#version 450

layout (set=0, binding=0) uniform sampler2D color_image;

layout(location = 0) in vec2 i_texcoord_0;
layout(location = 0) out vec4 o_color;

void main() {
  o_color = texture(color_image, i_texcoord_0);
}