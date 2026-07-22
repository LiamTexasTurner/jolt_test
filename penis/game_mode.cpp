#include "game_mode.hpp"
#include "scene.hpp"
#include "penis_jolt.hpp"
#include "renderer.hpp"
#include "entity.hpp"
#include "GLFW/glfw3.h"
#include "input.hpp"
#include "job_system.h"
#include <cereal/cereal.hpp>
#include <fstream>

#include <iostream>

using namespace std;

class GameMode : public IGameMode
{
public:

      std::vector<Entity> entities;      
      Scene* m_scene;
      pJolt* m_jolt;
      IRenderer* m_renderer;
      GLFWwindow* m_window;


      uint32_t current_skybox;

      bool m_first_update;

      unsigned int m_last_update_tick;

      void Init(Scene* scene, pJolt* p_jolt, GLFWwindow* window, IRenderer* renderer) override
      {
            m_scene = scene;
      
            m_jolt = p_jolt;

            m_renderer = renderer;

            m_window = window;

            m_first_update = true;

            uint32_t chips_skeleton_ID = 0;
            vector<uint32_t> chips_animations;
            

            //skeletons
            {
                  SkeletonData skeleton_data;
                  std::ifstream is("../resources/skeletons/chips_rig.pskeleton", ios::binary);
                  cereal::BinaryInputArchive i_archive(is);
                  i_archive(skeleton_data);

                  chips_skeleton_ID = LoadSkeleton(*m_scene, skeleton_data);

                  chips_animations = LoadAnimations(*m_scene, "../resources/chips_2/animations/");
            }


            //Chips
            {
                  MeshData chips_mesh;
                  std::ifstream is("../resources/chips_2/bin/chips_2.pbin", ios::binary);
                  cereal::BinaryInputArchive i_archive(is);
                  i_archive(chips_mesh);

                  pSkeleton chips_skeleton = scene->skeletons[chips_skeleton_ID];
                  AnimationGraph chips_graph(&m_scene->animations,
                                             chips_skeleton.bone_count,
                                             chips_skeleton_ID,
                                             chips_animations,
                                             chips_skeleton.bone_info);                

                  uint32_t chips_ID = LoadSkeletalMesh(*m_scene, chips_mesh);



                  
                  
                  //Instances
                  for(int i = 0; i < 10; i++)
                  {
                        uint32_t new_instance_ID = 0;

                        if(!chips_mesh.skinned)
                        {
                              AddMeshInstance(*scene, chips_ID, &new_instance_ID);
                        }
                        else
                        {
                              AddSkinnedMeshInstance(*scene, chips_ID, &new_instance_ID, chips_graph);
                        }
                              
                        uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
                        scene->transforms[new_instance_ID].translation = glm::vec3(i + 1,0,0);
                        scene->transforms[new_instance_ID].rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0,1,0));
                        entities.emplace_back(Entity(new_instance_ID));
                  }
            };

            //pilot
            // {
            //       MeshData mesh;
            //       std::ifstream is("../resources/pilot/bin/pilot.pbin", ios::binary);
            //       cereal::BinaryInputArchive i_archive(is);
            //       i_archive(mesh);
            //       uint32_t mesh_ID = LoadMesh(*m_scene, mesh);
            //       {
            //             uint32_t new_instance_ID = 0;

            //             if(!mesh.skinned)
            //             {
            //                   AddMeshInstance(*scene, mesh_ID, &new_instance_ID);
            //             }
            //             else
            //             {
            //                   AddSkinnedMeshInstance(*scene, mesh_ID, &new_instance_ID, std::move(chips_graph));
            //             }
                              
            //             uint32_t new_transform_ID = scene->instances[new_instance_ID].transform_ID;
            //             scene->transforms[new_instance_ID].translation = glm::vec3(0,1,0);
            //             scene->transforms[new_instance_ID].rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0,1,0));
            //             entities.emplace_back(Entity(new_instance_ID));
            //       }                 
            // }
            //;

            
                  
            AddSkybox(*m_scene, &current_skybox);
            
            Camera main_camera(glm::vec3(0.0f, 2.0f, -10.0f), glm::vec3(0,1,0), glm::vec3(1,0,0), 0.0f, 0.0f, CamType::flying);
            m_scene->main_camera_ID = m_scene->cameras.insert((main_camera));

            m_renderer->UpdateBuffers(scene);
      }

      void HandleEvent() override
      {
            
      }

      void Update(float dt) override
      {
            GameData* gd = static_cast<GameData*>(glfwGetWindowUserPointer(m_window));
            PlayerInput& player_input = gd->player_input;
            KeyInputs& key_inputs = player_input.key_inputs;
            MouseData& mouse_data = player_input.mouse_data;

            if(!gd->cursor_enabled)
            {
                  Camera& main_camera = m_scene->cameras[m_scene->main_camera_ID];
                  main_camera.UpdateFromInput(player_input, dt);
                  
                  if(key_inputs.space.pressed)
                  {
                        RemoveInstance(*m_scene, entities.back().instance_ID);      
                  }
            }
            
            if(key_inputs.tab.pressed)
            {
                  m_renderer->DrawDebugQuad();
            }
            if(key_inputs.shift.pressed)
            {
                  m_renderer->toggle_skinned();
            }

            
            // for(int i = 0; i < m_scene->animation_graphs.size(); i++)
            // {
            //       AnimationGraph& graph = m_scene->animation_graphs[i];
            //       TickAnimGraphSerial(graph, dt);
                  
            // }
            
            pJobSystem::Dispatch(m_scene->animation_graphs.size(),
                                1,
                                [this, dt]
                                (JobDispatchArgs args, Arena& arena)
            {
                  AnimationGraph& graph = m_scene->animation_graphs[args.jobIndex];
                  TickAnimGraph(arena, graph, dt, args.jobIndex);
            });

            pJobSystem::Wait();
      

            
      }

      void PhysicsUpdate(float dt) override
      {
            m_jolt->mPhysicsSystem->Update(dt, m_jolt->mCollisionSteps, m_jolt->mTempAllocator, m_jolt->mJobSystem);
            
      }
};

IGameMode* NewGameMode()
{
      return new GameMode{};
}
