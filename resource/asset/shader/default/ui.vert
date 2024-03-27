layout(location = 0) out vec2 o_texcoord_0;

void main() {
  o_texcoord_0 = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
  gl_Position = vec4(o_texcoord_0 * 2.0f - 1.0f, 0.0f, 1.0f);
}