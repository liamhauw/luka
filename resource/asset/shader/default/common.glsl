struct PunctualLight {
  vec3 direction;
  float range;
  vec3 color;
  float intensity;
  vec3 position;
  float inner_cone_cos;
  float outer_cone_cos;
  int type;
};

const int MaxPunctualLightCount = 8;
const int PunctualLightCount = 2;
const float Pi = 3.14159265359;
const int DirectionalLight = 0;
const int PointLight = 1;
const int SpotLight = 2;