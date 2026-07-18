in vec3 fWorldPosition;
in vec2 fTexCoord;
in vec3 fWorldNormal;
in vec3 fFragPos;
in vec4 fFragPosLightSpace;

layout(location = SCENE_CAMERAPOS_UNIFORM_LOCATION)
uniform vec3 CameraPos;

layout(location = SCENE_HAS_DIFFUSE_MAP_UNIFORM_LOCATION)
uniform bool HasDiffuseMap;

layout(location = SCENE_LIGHT_POS)
uniform vec3 lightPos;

layout(location = SCENE_LIGHT_COLOR)
uniform vec3 light_color;

layout(location = SCENE_TEXCOORD_SCALE)
uniform vec2 tex_coord_scale;

layout(location = SCENE_TEXCOORD_OFFSET)
uniform vec2 tex_coord_offset;

layout(binding = SCENE_DIFFUSE_MAP_TEXTURE_BINDING)
uniform sampler2D DiffuseMap;

layout(binding = SCENE_SHADOW_MAP_TEXTURE_BINDING)
uniform sampler2D shadowMap;

out vec4 FragColor;

float shadowCalculation(vec4 fragPosLightSpace)
{
  vec3 projCoords =
    fragPosLightSpace.xyz / fragPosLightSpace.w;

  projCoords =
    projCoords * 0.5 + 0.5;

  float closestDepth =
    texture(shadowMap, projCoords.xy).r;

  float shadow = projCoords.z > closestDepth ? 1.0 : 0.0;

  if(projCoords.z > 1)
    {
      shadow = 0;
    }
  return shadow;
}

void main()
{
  
  

  if(HasDiffuseMap)
    {
      vec2 diffuse_map_tex_coords = fTexCoord * tex_coord_scale + tex_coord_offset;
      
      vec4 color = texture(DiffuseMap, diffuse_map_tex_coords);
      
      if(color.a < 0.1)
        discard;
  
      vec3 normal = normalize(fWorldNormal);

      vec3 lightColor = vec3(0.6, 0.6, 0.6);

      vec3 lightDir = normalize(lightPos);

      vec3 ambient = 0.3 * light_color;

      float diff = max(dot(lightDir, normal), 0.0);
      vec3 diffuse = diff * light_color;

      float shadow = shadowCalculation(fFragPosLightSpace);

      vec3 lighting = (ambient + (1.0 - shadow) * diffuse) * color.rgb;
    
  
      FragColor = vec4(lighting, 1.0);

    }
  else
    {
      FragColor = vec4(1, 0, 1, 1.0);
    }
}


