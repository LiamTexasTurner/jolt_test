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
  // perform perspective divide
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  // transform to [0,1] range
  projCoords = projCoords * 0.5 + 0.5;
  // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
  float closestDepth = texture(shadowMap, projCoords.xy).r; 
  // get depth of current fragment from light's perspective
  float currentDepth = projCoords.z;
  // calculate bias (based on depth map resolution and slope)
  vec3 normal = normalize(fWorldNormal);
  vec3 lightDir = normalize(lightPos - fFragPos);
  float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
  // float bias = 0;
  // check whether current frag pos is in shadow
  // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
  // PCF
  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
  for(int x = -1; x <= 1; ++x)
    {
      for(int y = -1; y <= 1; ++y)
        {
          float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
          shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
  shadow /= 9.0;
    
  // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
  if(projCoords.z > 1.0)
    shadow = 0.0;
        
  return shadow;
}

void main()
{

  vec3 color = texture(DiffuseMap, fTexCoord).rgb;

  //ambient 
  vec3 ambient = 0.3 * vec3(1,1,1);
    
  float shadow = shadowCalculation(fFragPosLightSpace);

  color = (ambient + (1.0 - shadow)) * color;
    
  FragColor = vec4(color, 1.0);
}
