in vec3 fWorldPosition;
in vec2 fTexCoord;
in vec3 fWorldNormal;
in vec3 fFragPos;
in mat4 fFragPosLightSpace;

layout(location = SCENE_CAMERAPOS_UNIFORM_LOCATION)
uniform vec3 CameraPos;

layout(location = SCENE_HAS_DIFFUSE_MAP_UNIFORM_LOCATION)
uniform int HasDiffuseMap;

layout(binding = SCENE_DIFFUSE_MAP_TEXTURE_BINDING)
uniform sampler2D DiffuseMap;

out vec4 FragColor;

void main()
{
    vec3 diffuseMap;
    if (HasDiffuseMap != 0)
      diffuseMap = texture(DiffuseMap, fTexCoord).rgb;
    else
      diffuseMap = vec3(1.0);
    

    FragColor = vec4(diffuseMap, 1);
}
