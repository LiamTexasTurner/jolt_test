in vec2 oTexCoord;
out vec4 FragCol;

layout(location = DEBUG_DEPTH_MAP_TEXURE_BINDING)
uniform sampler2D depth_map;

layout(location = DEBUG_DEPTH_MAP_NEAR_PLANE)
uniform float near_plane;

layout(location = DEBUG_DEPTH_MAP_FAR_PLANE)
uniform float far_plane;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}

void main()
{
  float depthValue = texture(depth_map, oTexCoord).r;
  FragCol = vec4(vec3(depthValue), 1.0); 
}
