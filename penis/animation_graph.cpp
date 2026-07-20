#include "animation_graph.hpp"
#include "animation.hpp"
#include <iostream>
#include "arena.hpp"
#include "scene.hpp"

using namespace std;

void TickAnimGraph(Arena& arena, AnimationGraph& graph, float dt)
{
      ArenaArray<TRS> pose_1 = CreateArenaArray<TRS>(arena, graph.bone_count);
      memset(pose_1.items, 0, pose_1.capacity * sizeof(*pose_1.items));

      ArenaArray<TRS> pose_2 = CreateArenaArray<TRS>(arena, graph.bone_count);
      memset(pose_2.items, 0, pose_2.capacity * sizeof(*pose_2.items));

      ArenaArray<TRS> out_pose_ls = CreateArenaArray<TRS>(arena, graph.bone_count);
      memset(out_pose_ls.items, 0, out_pose_ls.capacity * sizeof(*out_pose_ls.items));
      
      float x = 0;
}
