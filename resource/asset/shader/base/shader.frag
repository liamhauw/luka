#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout ( set = 0, binding = 0 ) uniform sampler2D global_images[];

layout(set = 1, binding = 0) uniform DrawElementUniform {
    mat4 m;
		vec4 base_color_factor;
		uvec4 image_indices;
} draw_element_uniform;

layout(location = 0) in vec4 i_position;

#ifdef HAS_TEXCOORD_0_BUFFER
layout(location = 1) in vec2 i_texcoord_0;
#endif

#ifdef HAS_NORMAL_BUFFER
layout(location = 2) in vec3 i_normal;
#endif

layout(location = 0) out vec4 o_color;

void main(void)
{
	vec4 base_color = vec4(1.0, 1.0, 1.0, 1.0);

#ifdef HAS_BASE_COLOR_TEXTURE
	base_color = texture(global_images[nonuniformEXT(draw_element_uniform.image_indices.x)], i_texcoord_0);
#else
	base_color = draw_element_uniform.base_color_factor;
#endif

	o_color = base_color;
}
