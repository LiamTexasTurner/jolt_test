layout(location = BLIT_TEXCOORD_VARYING_LOCATION) in vec2 oTexCoord;

out vec4 FragCol;

void main()
{
      vec3 bottom_color = vec3(1.0, 1.0, 0.0);
      vec3 top_color    = vec3(1.0, 0.0, 0.0);

      vec3 color = mix(bottom_color, top_color, oTexCoord.x);

      FragCol = vec4(color, 1.0);
}
