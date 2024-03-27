#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0) uniform sampler global_samplers[];
layout(set = 0, binding = 1) uniform texture2D global_images[];

layout(set = 1, binding = 0) uniform DrawElementUniform {
  mat4 m;
  vec4 base_color_factor;
  uvec4 sampler_indices;
  uvec4 image_indices;
}
draw_element_uniform;

#ifdef HAS_TEXCOORD_0_BUFFER
layout(location = 1) in vec2 i_texcoord_0;
#endif

#ifdef HAS_NORMAL_BUFFER
layout(location = 2) in vec3 i_normal;
#endif

layout(location = 0) out vec4 o_base_color;
layout(location = 1) out vec4 o_normal;

void main(void) {
  vec4 base_color = vec4(1.0, 1.0, 1.0, 1.0);

#ifdef HAS_BASE_COLOR_TEXTURE
  uint index = draw_element_uniform.sampler_indices.x;
  base_color = texture(nonuniformEXT(sampler2D(
                           global_images[draw_element_uniform.image_indices.x],
                           global_samplers[index])),
                       i_texcoord_0);
#else
  base_color = draw_element_uniform.base_color_factor;
#endif

  o_base_color = base_color;

  vec3 normal = vec4(1.0, 0.0, 0.0);

#ifdef HAS_NORMAL_BUFFER
  normal = normalize(i_normal) * 0.5 + 0.5;
#endif

  o_normal = vec4(normal, 1.0);
}
