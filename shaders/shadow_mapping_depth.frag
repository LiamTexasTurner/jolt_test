layout(binding = SHADOW_DIFFUSE_MAP_TEXTURE_BINDING)
uniform sampler2D diffuse_map;

in vec2 fTexCoord;

void main()
{
  vec4 color = texture(diffuse_map, fTexCoord);

  if(color.a < 0.9)
    discard;
   
  float depth =
    gl_FragCoord.z;

  float bias =
    fwidth(depth) * 4;

  gl_FragDepth =
    min(depth + bias, 1.0);
}
