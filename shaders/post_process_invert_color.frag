in vec2 oTexCoord;

out vec4 FragCol;

layout(location = BLIT_TEXTURE_TEXURE_BINDING)
uniform sampler2D blit_texture;

void main()
{
  vec3 color = texture(blit_texture, oTexCoord).rgb;
  FragCol = vec4(vec3(1.0 - color), 1.0);
}
