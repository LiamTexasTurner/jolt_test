layout (location = DEBUG_POSITION_ATTRIB_LOCATION) in vec3 aPos;
layout (location = DEBUG_COLOR_ATTRIB_LOCATION) in vec3 aColor;



layout (location = DEBUG_MODEL_UNIFORM_LOCATION)
uniform mat4 model;

layout (location = DEBUG_VIEW_UNIFORM_LOCATION)
uniform mat4 view;

layout (location = DEBUG_PROJECTION_UNIFORM_LOCATION)
uniform mat4 projection;

out vec3 color;

void main()
{
    color = aColor;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
