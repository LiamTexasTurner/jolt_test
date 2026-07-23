layout(location = BLIT_TEXCOORD_VARYING_LOCATION) in vec2 oTexCoord;

out vec4 FragCol;

layout(location = POST_PROCESS_ITIME_UNIFORM_LOCATION)
uniform float iTime;

layout(binding = POST_PROCESS_TEXTURE_BINDING)
uniform sampler2D in_texture;


void main()
{
  FragCol = texture(in_texture, oTexCoord);
}
