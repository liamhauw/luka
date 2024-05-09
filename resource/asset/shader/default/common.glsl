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
