layout (location = DEBUG_REN_POS_ATTRIB_LOCATION) in vec3 aPos;
layout (location = DEBUG_REN_COL_ATTRIB_LOCATION) in vec3 aColor;

layout (location = DEBUG_REN_MODEL_UNIFORM_LOCATION)
uniform mat4 model;
layout (location = DEBUG_REN_VIEW_UNIFORM_LOCATION)
uniform mat4 view;
layout (location = DEBUG_REN_PROJECTION_UNIFORM_LOCATION)
uniform mat4 projection;

out vec3 color;

void main()
{
  color = vec3(1,1,1);
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
