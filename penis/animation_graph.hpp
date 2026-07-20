#pragma once
#include <stdint.h>
#include "arena.hpp"
#include "common.hpp"
#include "penis_mesh.hpp"
#include <vector>


struct StateMachine
{
      
};

struct AnimationGraph
{
      // ArenaArray<TRS> pose_1;
      // ArenaArray<TRS> pose_2;
      std::vector<TRS> out_pose;

      int bone_count;

      AnimationGraph() = default;

      AnimationGraph(int in_bone_count) :
            bone_count(in_bone_count)
      {
            
      }

      
};
void TickAnimGraph(Arena& arena, AnimationGraph& graph, float dt);
