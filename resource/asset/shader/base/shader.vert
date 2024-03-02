#version 450

layout(push_constant, std430) uniform PushConstantUniform {
	mat4 pv;
    vec3 camera_position;
} push_constant_uniform;

layout(set = 1, binding = 0) uniform DrawElementUniform {
    mat4 m;
    vec4 base_color_factor;
    uvec4 texture_indices;
} draw_element_uniform;

layout(location = 0) in vec3 position;
layout(location = 0) out vec4 o_position;

#ifdef HAS_TEXCOORD_0_BUFFER
layout(location = 1) in vec2 texcoord_0;
layout(location = 1) out vec2 o_texcoord_0;
#endif

#ifdef HAS_NORMAL_BUFFER
layout(location = 2) in vec3 normal;
layout(location = 2) out vec3 o_normal;
#endif

void main(void)
{
    o_position = draw_element_uniform.m * vec4(position, 1.0);

#ifdef HAS_TEXCOORD_0_BUFFER
    o_texcoord_0 = texcoord_0;
#endif

#ifdef HAS_NORMAL_BUFFER
    o_normal = mat3(draw_element_uniform.m) * normal;
#endif

    gl_Position = push_constant_uniform.pv * o_position;
}
