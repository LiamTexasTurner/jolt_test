in vec3 fWorldPosition;
in vec2 fTexCoord;
in vec3 fWorldNormal;
in vec3 fFragPos;
in vec4 fFragPosLightSpace;

layout(location = SCENE_CAMERAPOS_UNIFORM_LOCATION)
uniform vec3 CameraPos;

layout(location = SCENE_HAS_DIFFUSE_MAP_UNIFORM_LOCATION)
uniform int HasDiffuseMap;

layout(location = SCENE_LIGHT_POS)
uniform vec3 lightPos;

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

  return projCoords.z > closestDepth
    ? 1.0
    : 0.0;
}

void main()
{

  vec3 color = texture(DiffuseMap, fTexCoord).rbg;
  
  vec3 normal = normalize(fWorldNormal);

  vec3 lightColor = vec3(1,1,1);

  vec3 lightDir = normalize(lightPos);

  vec3 ambient = 0.15 * lightColor;

  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = diff * lightColor;

  float shadow = shadowCalculation(fFragPosLightSpace);

  vec3 lighting = (ambient + (1.0 - shadow) * diffuse) * color;
    
  
  FragColor = vec4(lighting, 1.0);
}


