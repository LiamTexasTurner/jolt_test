#include "animation_graph.hpp"
#include "animation.hpp"
#include <iostream>
#include "arena.hpp"
#include "scene.hpp"
#include <algorithm>
#include <span>

using namespace std;

void TickClipTime(AnimationClip& clip, float dt, bool looping)
{
      clip.current_time += (60.0f * dt) * clip.play_rate;
      if(clip.current_time > clip.frame_count)
      {
            if(!looping)
            {
                  clip.current_time = (float)clip.frame_count;
                  return;
            }
            clip.current_time = fmod(clip.current_time, (float)clip.frame_count);
      }
}

void TickPose(const Animation& in_animation, span<TRS> out_pose, float frame)
{
      int current_frame = (int)frame;
      int next_frame = current_frame + 1;
      float blend = frame - current_frame;
      blend = clamp(blend, 0.0f, 1.0f);
      if(current_frame >= in_animation.frame_count)
      {
            current_frame = current_frame % in_animation.frame_count;     
      }
      if(next_frame >= in_animation.frame_count)
      {
            next_frame = next_frame % in_animation.frame_count;
      }

      for(int i = 0; i < out_pose.size(); i++)
      {
            glm::vec3 translation_f1 = in_animation.frame_poses[current_frame][i].translation;
            glm::vec3 translation_f2 = in_animation.frame_poses[next_frame][i].translation;
            out_pose[i].translation = glm::vec4(glm::mix(translation_f1, translation_f2, blend), 1.0f);

            glm::quat rotation_f1 = in_animation.frame_poses[current_frame][i].rotation;
            glm::quat rotation_f2 = in_animation.frame_poses[next_frame][i].rotation;
            out_pose[i].rotation = glm::slerp(rotation_f1, rotation_f2, blend);

            glm::vec3 scale_f1 = in_animation.frame_poses[current_frame][i].scale;
            glm::vec3 scale_f2 = in_animation.frame_poses[next_frame][i].scale;
            out_pose[i].scale = glm::vec4(glm::mix(scale_f1, scale_f2, blend), 1.0f);
      }
}

void TickAnimGraph(Arena& arena, AnimationGraph& graph, float dt)
{
      // ArenaArray<TRS> pose_1 = CreateArenaArray<TRS>(arena, graph.bone_count);
      // memset(pose_1.items, 0, pose_1.capacity * sizeof(*pose_1.items));

      // ArenaArray<TRS> pose_2 = CreateArenaArray<TRS>(arena, graph.bone_count);
      // memset(pose_2.items, 0, pose_2.capacity * sizeof(*pose_2.items));

      // ArenaArray<TRS> out_pose_ls = CreateArenaArray<TRS>(arena, graph.bone_count);
      // memset(out_pose_ls.items, 0, out_pose_ls.capacity * sizeof(*out_pose_ls.items));

      AnimationClip& clip = graph.clips[16];
      TickClipTime(clip, dt, true);

      TickPose((*graph.animations)[clip.ID], graph.out_pose, clip.current_time);

      FK(graph.bone_info, graph.out_pose);


            
}
void TickAnimGraphSerial( AnimationGraph& graph, float dt)
{
      


      AnimationClip& clip = graph.clips[16];
      TickClipTime(clip, dt, true);

      TickPose((*graph.animations)[clip.ID], graph.out_pose, clip.current_time);
}
