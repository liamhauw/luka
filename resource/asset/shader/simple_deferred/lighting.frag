// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#version 450

#include "../common/common.glsl"

layout(set = 0, binding = 0) uniform Subpass {
  SubpassUniform subpass_uniform;
};

layout(input_attachment_index = 0, set = 0,
       binding = 1) uniform subpassInput subpass_i_base_color;
layout(input_attachment_index = 1, set = 0,
       binding = 2) uniform subpassInput subpass_i_metallic_roughness;
layout(input_attachment_index = 2, set = 0,
       binding = 3) uniform subpassInput subpass_i_normal;
layout(input_attachment_index = 3, set = 0,
       binding = 4) uniform subpassInput subpass_i_occlusion;
layout(input_attachment_index = 4, set = 0,
       binding = 5) uniform subpassInput subpass_i_emissive;
layout(input_attachment_index = 5, set = 0,
       binding = 6) uniform subpassInput subpass_i_depth;

layout(location = 0) in vec2 i_texcoord_0;
layout(location = 0) out vec4 o_color;

void main() {
  // Base color.
  vec3 base_color = subpassLoad(subpass_i_base_color).xyz;

  // Metallic and roughness.
  vec2 metallic_roughness = subpassLoad(subpass_i_metallic_roughness).xy;
  float metallic = metallic_roughness.x;
  float roughness = metallic_roughness.y;

  // Normal.
  vec3 n = subpassLoad(subpass_i_normal).xyz;
  n = normalize(2.0 * n - 1.0);

  // Occlusion.
  float occlusion = subpassLoad(subpass_i_occlusion).x;

  // Emissive.
  vec3 emissive = subpassLoad(subpass_i_emissive).xyz;

  // Position.
  float depth = subpassLoad(subpass_i_depth).x;
  vec4 clip = vec4(i_texcoord_0 * 2.0 - 1.0, depth, 1.0);
  vec4 pos_w = subpass_uniform.inverse_pv * clip;
  vec3 pos = pos_w.xyz / pos_w.w;

  // Calculate light contribution.
  vec3 Lo = vec3(0.0);

  vec3 v = normalize(subpass_uniform.camera_position.xyz - pos);
  float ndotv = max(dot(n, v), 0.0);
  vec3 F0 = mix(vec3(0.04), base_color, metallic);

  // Punctual light.
  for (int i = 0; i < PUNCTUAL_LIGHT_COUNT; ++i) {
    PunctualLight light = subpass_uniform.punctual_lights[i];
    // Input light.
    vec3 point_to_light;
    if (light.type != DIRECTIONAL_LIGHT) {
      point_to_light = light.position - pos;
    } else {
      point_to_light = -light.direction;
    }

    vec3 l = normalize(point_to_light);
    vec3 h = normalize(v + l);
    float ndotl = max(dot(n, l), 0.0);
    float vdoth = max(dot(v, h), 0.0);
    float ndoth = max(dot(n, h), 0.0);

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