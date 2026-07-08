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

      if(projCoords.x < 0.0 || projCoords.x > 1.0 ||
         projCoords.y < 0.0 || projCoords.y > 1.0 ||
         projCoords.z < 0.0 || projCoords.z > 1.0)
      {
            return 0.0;
      }

      float currentDepth =
            projCoords.z;

      vec2 texelSize =
            1.0 / vec2(textureSize(shadowMap, 0));

      float shadow = 0.0;

      for(int x = -1; x <= 1; x++)
      {
            for(int y = -1; y <= 1; y++)
            {
                  vec2 sampleCoords =
                        projCoords.xy +
                        vec2(x, y) * texelSize;

                  float closestDepth =
                        texture(shadowMap, sampleCoords).r;

                  shadow +=
                        currentDepth > closestDepth
                        ? 1.0
                        : 0.0;
            }
      }

      return shadow / 9.0;
}

void main()
{
      vec3 color;

      if(HasDiffuseMap != 0)
      {
            color =
                  texture(DiffuseMap, fTexCoord).rgb;
      }
      else
      {
            color =
                  vec3(1.0);
      }

      vec3 normal =
            normalize(fWorldNormal);

      vec3 toLight =
            normalize(lightPos - fFragPos);

      float diffuseFactor =
            max(dot(normal, toLight), 0.0);

      float shadow =
            shadowCalculation(fFragPosLightSpace);

      vec3 ambient =
            0.3 * color;

      vec3 diffuse =
            diffuseFactor *
            (1.0 - shadow) *
            color;

      vec3 finalColor =
            ambient + diffuse;

      FragColor =
        vec4(color * (1 - shadow), 1.0);
}
