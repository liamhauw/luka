// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#version 450

layout(location = 0) out vec2 o_texcoord_0;

void main() {
  o_texcoord_0 = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
  gl_Position = vec4(o_texcoord_0 * 2.0 - 1.0, 0.0, 1.0);
}