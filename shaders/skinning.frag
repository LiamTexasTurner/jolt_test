in vec3 fWorldPosition;
in vec2 fTexCoord;
in vec3 fWorldNormal;
in vec3 fFragPos;

layout(location = SCENE_CAMERAPOS_UNIFORM_LOCATION)
uniform vec3 CameraPos;

layout(location = SCENE_HAS_DIFFUSE_MAP_UNIFORM_LOCATION)
uniform int HasDiffuseMap;

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


void main()
{
  
  vec4 color = texture(DiffuseMap, fTexCoord);

  FragColor = color;

}


