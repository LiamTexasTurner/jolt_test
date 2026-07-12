#include "animation.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

void FK(std::span<const BoneInfo> bones, std::span<transform_penis> out_pose)
{
      for(int i = 1; i < bones.size(); i++)
      {
            int bone_index = bones[i].index;
            int parent_index = bones[i].parent;
    
            glm::vec3 parent_position = out_pose[parent_index].translation;
            glm::quat parent_rotation = out_pose[parent_index].rotation;
            
            glm::vec3 local_position = out_pose[bone_index].translation;
            glm::vec3 ls_to_ms_position = (parent_rotation * local_position) + parent_position;

            glm::quat local_rotation = out_pose[bone_index].rotation;
            glm::quat ls_to_ms_rotation = parent_rotation * local_rotation;

            out_pose[bone_index].translation = ls_to_ms_position;

            out_pose[bone_index].rotation = ls_to_ms_rotation;
      }
}
