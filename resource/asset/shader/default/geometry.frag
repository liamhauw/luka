// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 1, binding = 0) uniform sampler bindless_samplers[];
layout(set = 1, binding = 1) uniform texture2D bindless_images[];

layout(set = 2, binding = 0) uniform DrawElementUniform {
  mat4 m;
  mat4 inverse_m;
  uvec4 sampler_indices;
  uvec4 image_indices;
  vec4 base_color_factor;
  float metallic_factor;
  float roughness_factor;
  float normal_scale;
  float occlusion_strength;
  vec4 emissiveFactor;
  uint alpha_model;
  float alpha_cutoff;
}
draw_element_uniform;

layout(location = 0) in vec3 i_position;

layout(location = 1) in vec3 i_normal;

#if defined(HAS_TANGENT_BUFFER)
layout(location = 2) in vec3 i_tangent;
#endif

#if defined(HAS_TEXCOORD_0_BUFFER)
layout(location = 3) in vec2 i_texcoord_0;
#endif

layout(location = 0) out vec4 o_base_color;
layout(location = 1) out vec2 o_metallic_roughness;
layout(location = 2) out vec4 o_normal;
layout(location = 3) out vec4 o_emissive;

void main(void) {
  // Base color.
  vec4 base_color = draw_element_uniform.base_color_factor;
#if defined(HAS_BASE_COLOR_TEXTURE)
  vec4 base_color_texel =
      texture(nonuniformEXT(sampler2D(
                  bindless_images[draw_element_uniform.image_indices.x],
                  bindless_samplers[draw_element_uniform.sampler_indices.x])),
              i_texcoord_0);
  base_color = vec4(pow(base_color_texel.rgb, vec3(2.2)), base_color_texel.a);
#endif
  if (draw_element_uniform.alpha_model == 1 &&
      base_color.a < draw_element_uniform.alpha_cutoff) {
    discard;
  }
  o_base_color = base_color;

  // Metallic and roughness.
  float metallic = draw_element_uniform.metallic_factor;
  float roughness = draw_element_uniform.roughness_factor;
#if defined(HAS_METALLIC_ROUGHNESS_TEXTURE)
  vec4 metallic_roughness_texel =
      texture(nonuniformEXT(sampler2D(
                  bindless_images[draw_element_uniform.image_indices.y],
                  bindless_samplers[draw_element_uniform.sampler_indices.y])),
              i_texcoord_0);
  metallic = metallic_roughness_texel.b;
  roughness = metallic_roughness_texel.g;
#endif
  o_metallic_roughness = vec2(metallic, roughness);

  // Normal
  vec3 normal = normalize(i_normal) * 0.5 + 0.5;

#if defined(HAS_NORMAL_TEXTURE)
  vec3 normal_texel =
      texture(nonuniformEXT(sampler2D(
                  bindless_images[draw_element_uniform.image_indices.z],
                  bindless_samplers[draw_element_uniform.sampler_indices.z])),
              i_texcoord_0)
          .xyz;

#if defined(HAS_TANGENT_BUFFER)
  vec3 T = normalize(i_tangent);
  vec3 B = normalize(cross(normal, T));
  mat3 TNB = mat3(T, B, normal);
  normal = normalize(TNB * (normal_texel * 2.0 - 1.0));
#elif defined(HAS_TEXCOORD_0_BUFFER)
  vec2 uv_dx = dFdx(i_texcoord_0);
  vec2 uv_dy = dFdy(i_texcoord_0);
  if (length(uv_dx) <= 1e-2) {
    uv_dx = vec2(1.0, 0.0);
  }
  if (length(uv_dy) <= 1e-2) {
    uv_dy = vec2(0.0, 1.0);
  }

  vec3 T0 = (uv_dy.t * dFdx(i_position) - uv_dx.t * dFdy(i_position)) /
            (uv_dx.s * uv_dy.t - uv_dy.s * uv_dx.t);
  vec3 T = normalize(T0 - normal * dot(normal, T0));
  vec3 B = normalize(cross(normal, T));
  mat3 TNB = mat3(T, B, normal);
  normal = normalize(TNB * (normal_texel * 2.0 - 1.0));
#endif

#endif
  if (draw_element_uniform.normal_scale != 1.0) {
    normal.xy *= draw_element_uniform.normal_scale;
  }
  o_normal = vec4(normal, 1.0);

  // Emissive
  vec4 emissive = draw_element_uniform.emissiveFactor;
#if defined(HAS_EMISSIVE_TEXTURE)
  vec4 emissive_texel =
      texture(nonuniformEXT(sampler2D(
                  bindless_images[draw_element_uniform.image_indices.w],
                  bindless_samplers[draw_element_uniform.sampler_indices.w])),
              i_texcoord_0);
  emissive_texel = vec4(pow(emissive_texel.rgb, vec3(2.2)), emissive_texel.a);
  emissive *= emissive_texel;
#endif
  o_emissive = emissive;
}
