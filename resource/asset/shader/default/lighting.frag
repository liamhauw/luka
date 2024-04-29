// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#version 450

layout(set = 0, binding = 0) uniform SubpassUniform {
  mat4 pv;
  mat4 inverse_vp;
  vec4 camera_position;
  vec4 light_position;
}
subpass_uniform;

layout(input_attachment_index = 0, set = 0,
       binding = 1) uniform subpassInput subpass_i_base_color;
layout(input_attachment_index = 1, set = 0,
       binding = 2) uniform subpassInput subpass_i_metallic_roughness;
layout(input_attachment_index = 2, set = 0,
       binding = 3) uniform subpassInput subpass_i_normal;
layout(input_attachment_index = 3, set = 0,
       binding = 4) uniform subpassInput subpass_i_depth;

layout(location = 0) in vec2 i_texcoord_0;
layout(location = 0) out vec4 o_color;

const float Pi = 3.14159265359;

float D_GGX(float ndoth, float roughness) {
  float ndoth2 = ndoth * ndoth;
  float a = roughness * roughness;
  float a2 = a * a;
  float x = ndoth2 * (a2 - 1.0) + 1.0;
  return a2 / (Pi * x * x);
}

float G_Schlick_1(float ndotx, float k) {
  return ndotx / (ndotx * (1.0 - k) + k);
}

float G_Schlick(float ndotl, float ndotv, float roughness) {
  float x = roughness + 1;
  float k = x * x / 8.0;
  return G_Schlick_1(ndotl, k) * G_Schlick_1(ndotv, k);
}

vec3 F_Schlick(float vdoth, vec3 F0) {
  float x = (-5.55473 * vdoth - 6.98316) * vdoth;
  return F0 + (1.0 - F0) * pow(2.0, x);
}

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

  // Position.
  float depth = subpassLoad(subpass_i_depth).x;
  vec4 clip = vec4(i_texcoord_0 * 2.0 - 1.0, depth, 1.0);
  vec4 pos_w = subpass_uniform.inverse_vp * clip;
  vec3 pos = pos_w.xyz / pos_w.w;

  // Calculate light contribution.
  vec3 Lo = vec3(0.0);
  vec3 v = normalize(subpass_uniform.camera_position.xyz - pos);
  float ndotv = max(dot(n, v), 0.0);
  vec3 F0 = mix(vec3(0.04), base_color, metallic);

  // 1. Traverse punctual lights.
  // TODO: set light position and value, here attach it to camera.
  vec3 l_pos = subpass_uniform.camera_position.xyz;
  vec3 l_val = vec3(50.0, 50.0, 50.0);
  float l_radius = 30.0;
  {
    // Input light.
    vec3 l = l_pos - pos;
    float dist = length(l);
    float falloff =
        pow(clamp(pow(1.0 - (dist / l_radius), 4.0), 0.0, 1.0), 2.0) /
        (dist * dist + 1.0);
    vec3 Li = l_val * falloff;

    // Light source related variables.
    l = normalize(l);
    vec3 h = normalize(v + l);
    float ndotl = max(dot(n, l), 0.0);
    float vdoth = max(dot(v, h), 0.0);
    float ndoth = max(dot(n, h), 0.0);

    // Brdf.
    vec3 F = F_Schlick(vdoth, F0);

    vec3 k_diffuse = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 f_diffuse = k_diffuse * base_color / Pi;

    vec3 k_specular = F;
    float D = D_GGX(ndoth, roughness);
    float G = G_Schlick(ndotv, ndotl, roughness);
    vec3 f_specular = k_specular * D * G / max(4.0 * ndotl * ndotv, 0.0001);

    vec3 brdf = f_diffuse + f_specular;

    // Reflection equation.
    Lo += Li * ndotl * brdf;
  }

  // 2. Ambient light.
  vec3 ambient = vec3(0.03) * base_color;

  o_color = vec4(ambient + Lo, 1.0);
}