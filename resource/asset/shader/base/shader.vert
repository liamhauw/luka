#version 320 es

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord_0;
layout(location = 2) in vec3 normal;

layout (location = 0) out vec4 o_position;
layout (location = 1) out vec2 o_texcoord_0;
layout (location = 2) out vec3 o_normal;

layout(set = 0, binding = 1) uniform GlobalUniform {
    mat4 pv;
    vec3 view_position;
} global_uniform;

layout(set = 0, binding = 2) uniform DrawElementUniform {
    mat4 m;
} draw_element_uniform;

void main(void)
{
    o_position = draw_element_uniform.m * vec4(position, 1.0);

    o_texcoord_0 = texcoord_0;

    o_normal = mat3(draw_element_uniform.m) * normal;

    gl_Position = global_uniform.pv * o_position;
}
