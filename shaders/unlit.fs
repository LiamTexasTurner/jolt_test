#version 410 core
out vec4 FragColor;
  
in vec2 TexCoord;

uniform bool has_albedo;
uniform vec4 base_color;
uniform sampler2D diffuse;

void main()
{
  if(has_albedo)
  {
        FragColor = texture(diffuse, TexCoord);
  }
  else
  {
    FragColor = base_color;
  }
  
}
