layout(location = BLIT_TEXCOORD_VARYING_LOCATION) in vec2 oTexCoord;

out vec4 FragCol;

layout(location = BLIT_TEXTURE_TEXURE_BINDING)
uniform sampler2D blit_texture;

void main()
{
  FragCol = texture(blit_texture, oTexCoord);
}
