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
  PunctualLight punctual_lights[PunctualLightMaxCount];
};

struct DrawElementUniform {
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
};