layout(location = SCENE_POSITION_ATTRIB_LOCATION)
in vec4 Position;

layout(location = SCENE_TEXCOORD_ATTRIB_LOCATION)
in vec2 TexCoord;

layout(location = SHADOW_MAP_LIGHT_SPACE_MATRIX_UNIFORM_LOCATION)
uniform mat4 lightSpaceMatrix;

layout(location = SHADOW_MAP_MW_UNIFORM_LOCATION)
uniform mat4 model;

out vec2 fTexCoord;

void main()
{

  fTexCoord = TexCoord;
  gl_Position = lightSpaceMatrix * model * Position;
}
