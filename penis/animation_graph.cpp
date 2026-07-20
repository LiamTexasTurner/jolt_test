#include "animation_graph.hpp"
#include "animation.hpp"
#include <iostream>
#include "arena.hpp"
#include "scene.hpp"

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

void TickAnimGraph(Arena& arena, AnimationGraph& graph, float dt)
{
      ArenaArray<TRS> pose_1 = CreateArenaArray<TRS>(arena, graph.bone_count);
      memset(pose_1.items, 0, pose_1.capacity * sizeof(*pose_1.items));

      ArenaArray<TRS> pose_2 = CreateArenaArray<TRS>(arena, graph.bone_count);
      memset(pose_2.items, 0, pose_2.capacity * sizeof(*pose_2.items));

      ArenaArray<TRS> out_pose_ls = CreateArenaArray<TRS>(arena, graph.bone_count);
      memset(out_pose_ls.items, 0, out_pose_ls.capacity * sizeof(*out_pose_ls.items));

      AnimationClip& clip = graph.clips[12];
      TickClipTime(clip, dt, true);

      cout << clip.current_time << endl;

      std::vector<std::vector<TRS>>& frame_poses = (*graph.animations)[clip.ID].frame_poses;

      graph.out_pose = frame_poses[(int)clip.current_time];
}
