#version 320 es

precision highp float;

layout(location = 0) in vec4 i_position;

#ifdef HAS_TEXCOORD_0_BUFFER
layout(location = 1) in vec2 i_texcoord_0;
#endif

#ifdef HAS_NORMAL_BUFFER
layout(location = 2) in vec3 i_normal;
#endif

layout(location = 0) out vec4 o_color;

#ifdef HAS_BASE_COLOR_TEXTURE
layout(set = 0, binding = 0) uniform sampler2D base_color_texture;
#endif

void main(void)
{
	vec4 base_color = vec4(1.0, 0.0, 0.0, 1.0);

#ifdef HAS_BASE_COLOR_TEXTURE
	base_color = texture(base_color_texture, i_texcoord_0);
#endif
	vec3 ambient_color = vec3(0.2) * base_color.xyz;

	o_color = vec4(ambient_color + base_color.xyz, base_color.w);
}
