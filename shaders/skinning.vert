// Linear Blend Skinning
layout(location = SCENE_POSITION_ATTRIB_LOCATION)
in vec4 Position;
layout(location = SCENE_MVP_UNIFORM_LOCATION)
uniform mat4 MVP;

layout(binding = SCENE_BONE_MAT_SSBO_BINDING, std430) readonly buffer bone_buffer
{
  mat4 bone_mats[];
};


void main()
{
  // vec4 skinnedPosition =
  //     aBoneWeights.x*(bone_matrices[aBoneID.x]*vec4(Position, 1.0)) +
  //     aBoneWeights.y*(bone_matrices[aBoneID.y]*vec4(Position, 1.0)) + 
  //     aBoneWeights.z*(bone_matrices[aBoneID.z]*vec4(Position, 1.0)) + 
  //     aBoneWeights.w*(bone_matrices[aBoneID.w]*vec4(Position, 1.0));

  vec4 skinnedPosition = vec4(0,0,0,1);

    gl_Position = MVP * skinnedPosition;
    
}
