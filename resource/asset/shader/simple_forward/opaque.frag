// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#version 450

#include "../common/common.glsl"

#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0) uniform Subpass {
  SubpassUniform subpass_uniform;
};
layout(set = 1, binding = 0) uniform sampler bindless_samplers[];
layout(set = 1, binding = 1) uniform texture2D bindless_images[];

layout(set = 2, binding = 0) uniform DrawElement {
  DrawElementUniform draw_element_uniform;
};

layout(location = 0) in vec3 i_position;

layout(location = 1) in vec3 i_normal;

#if defined(HAS_TANGENT_BUFFER)
layout(location = 2) in vec3 i_tangent;
#endif

#if defined(HAS_TEXCOORD_0_BUFFER)
layout(location = 3) in vec2 i_texcoord_0;
#endif

layout(location = 0) out vec4 o_color;

void main(void) {
  // Base color.
  vec4 base_color4 = draw_element_uniform.base_color_factor;
#if defined(HAS_BASE_COLOR_TEXTURE)
  vec4 base_color_texel =
      texture(nonuniformEXT(sampler2D(
                  bindless_images[draw_element_uniform.image_indices_0.x],
                  bindless_samplers[draw_element_uniform.sampler_indices_0.x])),
              i_texcoord_0);
  base_color4 = vec4(pow(base_color_texel.rgb, vec3(2.2)), base_color_texel.a);
#endif

#if defined(HAS_MASK_ALPHA) 
  if (base_color4.a < draw_element_uniform.alpha_cutoff) {
    discard;
  }
#endif
  vec3 base_color = base_color4.xyz;

  // Metallic and roughness.
  float metallic = draw_element_uniform.metallic_factor;
  float roughness = draw_element_uniform.roughness_factor;
#if defined(HAS_METALLIC_ROUGHNESS_TEXTURE)
  vec4 metallic_roughness_texel =
      texture(nonuniformEXT(sampler2D(
                  bindless_images[draw_element_uniform.image_indices_0.y],
                  bindless_samplers[draw_element_uniform.sampler_indices_0.y])),
              i_texcoord_0);
  metallic = metallic_roughness_texel.b;
  roughness = metallic_roughness_texel.g;
#endif

  // Normal
  vec3 normal = normalize(i_normal) * 0.5 + 0.5;

#if defined(HAS_NORMAL_TEXTURE)
  vec3 normal_texel =
      texture(nonuniformEXT(sampler2D(
                  bindless_images[draw_element_uniform.image_indices_0.z],
                  bindless_samplers[draw_element_uniform.sampler_indices_0.z])),
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

  // Occlusion
  float occlusion = draw_element_uniform.occlusion_strength;
#if defined(HAS_OCCLUSION_TEXTURE)
  float occlusion_texel =
      texture(nonuniformEXT(sampler2D(
                  bindless_images[draw_element_uniform.image_indices_0.w],
                  bindless_samplers[draw_element_uniform.sampler_indices_0.w])),
              i_texcoord_0)
          .x;
  occlusion = 1.0 + occlusion * (occlusion_texel - 1.0);
#endif

  // Emissive
  vec4 emissive4 = draw_element_uniform.emissive_factor;
#if defined(HAS_EMISSIVE_TEXTURE)
  vec4 emissive_texel =
      texture(nonuniformEXT(sampler2D(
                  bindless_images[draw_element_uniform.image_indices_1.x],
                  bindless_samplers[draw_element_uniform.sampler_indices_1.x])),
              i_texcoord_0);
  emissive_texel = vec4(pow(emissive_texel.rgb, vec3(2.2)), emissive_texel.a);
  emissive4 *= emissive_texel;
#endif
  vec3 emissive = emissive4.xyz;

  // Calculate light contribution.
  vec3 Lo = vec3(0.0);

  vec3 v = normalize(subpass_uniform.camera_position.xyz - i_position);
  float ndotv = max(dot(normal, v), 0.0);
  vec3 F0 = mix(vec3(0.04), base_color, metallic);

  // Punctual light.
  for (int i = 0; i < PUNCTUAL_LIGHT_COUNT; ++i) {
    PunctualLight light = subpass_uniform.punctual_lights[i];
    // Input light.
    vec3 point_to_light;
    if (light.type != DIRECTIONAL_LIGHT) {
      point_to_light = light.position - i_position;
    } else {
      point_to_light = -light.direction;
    }

    vec3 l = normalize(point_to_light);
    vec3 h = normalize(v + l);
    float ndotl = max(dot(normal, l), 0.0);
    float vdoth = max(dot(v, h), 0.0);
    float ndoth = max(dot(normal, h), 0.0);

    if (ndotv > 0.0 || ndotl > 0.0) {
      // Reflection equation.
      vec3 Li = LightIntensity(light, point_to_light);

      vec3 F = F_Schlick(vdoth, F0);

      vec3 k_diffuse = (vec3(1.0) - F) * (1.0 - metallic);
      vec3 f_diffuse = k_diffuse * base_color / PI;

      vec3 k_specular = F;
      float D = D_GGX(ndoth, roughness);
      float G = G_Schlick(ndotv, ndotl, roughness);
      vec3 f_specular = k_specular * D * G / max(4.0 * ndotl * ndotv, 0.0001);

      vec3 brdf = f_diffuse + f_specular;

      Lo += Li * ndotl * brdf;
    }
  }

  // Ambient light.
  vec3 ambient = vec3(0.03) * base_color * occlusion;
  
  Lo += ambient;

  vec3 color = emissive + Lo;

  o_color = vec4(color, 1.0);
}
