layout(location = SCENE_POSITION_ATTRIB_LOCATION)
in vec4 Position;

layout(location = SCENE_TEXCOORD_ATTRIB_LOCATION)
in vec2 TexCoord;

layout(location = SCENE_NORMAL_ATTRIB_LOCATION)
in vec3 Normal;

layout(location = SCENE_MW_UNIFORM_LOCATION)
uniform mat4 MW;

layout(location = SCENE_MVP_UNIFORM_LOCATION)
uniform mat4 MVP;

layout(location = SCENE_N_MW_UNIFORM_LOCATION)
uniform mat3 N_MW;

layout(location = SCENE_LIGHT_SPACE_MATRIX_UNIFORM_LOCATION)
uniform mat4 lightSpaceMatrix;

out vec3 fWorldPosition;
out vec2 fTexCoord;
out vec3 fWorldNormal;
out vec3 fFragPos;
out vec4 fFragPosLightSpace;

void main()
{  
    fWorldPosition = (MW * Position).xyz;
    fTexCoord = TexCoord;
    // fWorldNormal = N_MW * Normal;
    fWorldNormal = mat3(transpose(inverse(MW))) * Normal;
    fFragPos = vec3(MW * Position);
    fFragPosLightSpace = lightSpaceMatrix * vec4(fFragPos, 1.0);
    gl_Position = MVP * Position;    
}
