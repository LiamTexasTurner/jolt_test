// Linear Blend Skinning
layout(location = SCENE_POSITION_ATTRIB_LOCATION)
in vec4 Position;

layout(location = SCENE_BONE_ID_ATTRIB_LOCATION)
in ivec4 bone_ID;

layout(location = SCENE_BONE_WEIGHTS_ATTRIB_LOCATION)
in vec4 bone_weights;

layout(location = SCENE_MW_UNIFORM_LOCATION)
uniform mat4 MW;

layout(location = SCENE_VIEW_UNIFORM_LOCATION)
uniform mat4 view;

layout(location = SCENE_PROJECTON_UNIFORM_LOCATION)
uniform mat4 projection;

layout(binding = SCENE_BONE_MAT_SSBO_BINDING, std430) readonly buffer bone_buffer
{
  mat4 bone_mats[];
};


void main()
{
  vec4 skinnedPosition =
    bone_weights*(bone_mats[bone_ID.x]*Position) +
    bone_weights*(bone_mats[bone_ID.y]*Position) +
    bone_weights*(bone_mats[bone_ID.z]*Position) +
    bone_weights*(bone_mats[bone_ID.w]*Position);
    
  gl_Position = projection * view * MW * skinnedPosition;
    
}
