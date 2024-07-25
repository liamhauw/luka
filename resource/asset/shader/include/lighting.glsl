  // Lighting.
  vec3 Lo = vec3(0.0);

  vec3 v = normalize(subpass_uniform.camera_position.xyz - position);
  float ndotv = max(dot(normal, v), 0.0);
  vec3 F0 = mix(vec3(0.04), base_color.xyz, metallic);

  // Punctual.
  for (int i = 0; i < PUNCTUAL_LIGHT_COUNT; ++i) {
    PunctualLight light = subpass_uniform.punctual_lights[i];
    vec3 point_to_light;
    if (light.type != DIRECTIONAL_LIGHT) {
      point_to_light = light.position - position;
    } else {
      point_to_light = -light.direction;
    }

    vec3 l = normalize(point_to_light);
    vec3 h = normalize(v + l);
    float ndotl = max(dot(normal, l), 0.0);
    float vdoth = max(dot(v, h), 0.0);
    float ndoth = max(dot(normal, h), 0.0);

    if (ndotv > 0.0 || ndotl > 0.0) {
      vec3 Li = LightIntensity(light, point_to_light);

      vec3 F = F_Schlick(vdoth, F0);

      vec3 k_diffuse = (vec3(1.0) - F) * (1.0 - metallic);
      vec3 f_diffuse = k_diffuse * base_color.xyz / PI;

      vec3 k_specular = F;
      float D = D_GGX(ndoth, roughness);
      float G = G_Schlick(ndotv, ndotl, roughness);
      vec3 f_specular = k_specular * D * G / max(4.0 * ndotl * ndotv, 0.0001);

      vec3 brdf = f_diffuse + f_specular;

      Lo += Li * ndotl * brdf;
    }
  }

  // Ambient.
  Lo += vec3(0.03) * base_color.xyz * occlusion;

  // Emissive.
  Lo += emissive.xyz;