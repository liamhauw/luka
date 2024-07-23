// These structures also need to be defined in the engine source code
struct PunctualLight {
  vec3 position;
  uint type;
  vec3 direction;
  float intensity;
  vec3 color;
  float range;
  float inner_cone_cos;
  float outer_cone_cos;
  vec2 padding;
};

struct SubpassUniform {
  mat4 pv;
  mat4 inverse_pv;
  vec4 camera_position;
  PunctualLight punctual_lights[PUNCTUAL_LIGHT_MAX_COUNT];
};

struct DrawElementUniform {
  mat4 m;
  mat4 inverse_m;
  uvec4 sampler_indices_0;
  uvec4 sampler_indices_1;
  uvec4 image_indices_0;
  uvec4 image_indices_1;
  vec4 base_color_factor;
  float metallic_factor;
  float roughness_factor;
  float normal_scale;
  float occlusion_strength;
  vec4 emissive_factor;
  float alpha_cutoff;
};

float RangeAttenuation(float range, float dist) {
  if (range <= 0.0) {
    return 1.0 / pow(dist, 2.0);
  }
  return max(min(1.0 - pow(dist / range, 4.0), 1.0), 0.0) / pow(dist, 2.0);
}

float SpotAttenuation(vec3 direction, float inner_cone_cos,
                      float outer_cone_cos, vec3 l) {
  float actual_cos = dot(normalize(direction), normalize(-l));

  if (actual_cos > outer_cone_cos) {
    if (actual_cos < inner_cone_cos) {
      float angular_attenuation =
          (actual_cos - outer_cone_cos) / (inner_cone_cos - outer_cone_cos);
      return angular_attenuation * angular_attenuation;
    }
    return 1.0;
  }
  return 0.0;
}

vec3 LightIntensity(PunctualLight light, vec3 point_to_light) {
  float range_attenuation = 1.0;
  float spot_attenuation = 1.0;

  if (light.type != DIRECTIONAL_LIGHT) {
    range_attenuation = RangeAttenuation(light.range, length(point_to_light));
  }
  if (light.type == SPOT_LIGHT) {
    spot_attenuation = SpotAttenuation(light.direction, light.inner_cone_cos,
                                       light.outer_cone_cos, point_to_light);
  }

  return range_attenuation * spot_attenuation * light.intensity * light.color;
}

float D_GGX(float ndoth, float roughness) {
  float ndoth2 = ndoth * ndoth;
  float a = roughness * roughness;
  float a2 = a * a;
  float x = ndoth2 * (a2 - 1.0) + 1.0;
  return a2 / (PI * x * x);
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