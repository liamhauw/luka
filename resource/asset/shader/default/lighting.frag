layout(input_attachment_index = 0, binding = 0) uniform subpassInput i_base_color;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput i_normal;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput i_depth;

layout(location = 0) in vec2 i_texcoord_0;
layout(location = 0) out vec4 o_color;

void main() {
  vec4 base_color = subpassLoad(i_base_color);
  o_color = base_color;
}