layout(location = SCENE_POSITION_ATTRIB_LOCATION)
in vec4 Position;

layout(location = SHADOW_MAP_LIGHT_SPACE_MATRIX_UNIFORM_LOCATION)
uniform mat4 lightSpaceMatrix;

layout(location = SHADOW_MAP_MW_UNIFORM_LOCATION)
uniform mat4 model;

void main()
{
  gl_Position = lightSpaceMatrix * model * Position;
}
