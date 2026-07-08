layout (location = SKYBOX_POSITION_ATTRIB_LOCATION)
in vec3 aPos;

out vec3 TexCoords;

layout (location = SKYBOX_PROJECTION_UNIFORM_LOCATION)
uniform mat4 projection;

layout(location = SKYBOX_VIEW_UNIFORM_LOCATION)
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  

