#include "renderer.hpp"
#include "scene.hpp"
#include "preamble.glsl"
#include "shaderset.h"
#include "shader.h"

#include "GLFW/glfw3.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <iostream>

class Renderer : public IRenderer
{
public:

      Scene* m_scene;
      
      Shader unlit;


      ShaderSet m_shaders;
      GLuint* m_scene_SP;
      
      void Init(Scene* scene) override
      {
            m_scene = scene;
            unlit.ShaderInit("../shaders/unlit.vs", "../shaders/unlit.fs");

            m_shaders.SetVersion("440");
            m_shaders.SetPreambleFile("../penis/preamble.glsl");
            m_scene_SP = m_shaders.AddProgramFromExts({"../shaders/scene.vert", "../shaders/scene.frag"});

            
      }

      void Paint() override
      {

            glClearColor(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            m_shaders.UpdatePrograms();

            

            glm::vec3 eye = glm::vec3(sinf(glfwGetTime()) * 2.0f,2, -10);
            glm::vec3 up = glm::vec3(0,1,0);
            
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 300.0f);
            glm::mat4 view = glm::lookAt(eye, eye + glm::vec3(0,0,1), up);

            glm::mat4 VP = projection * view;

            glUseProgram(*m_scene_SP);

            glUniform3fv(SCENE_CAMERAPOS_UNIFORM_LOCATION, 1, glm::value_ptr(eye));

            glEnable(GL_DEPTH_TEST);
            for(uint32_t instance_ID : m_scene->instances)
            {
                  const Instance* instance = &m_scene->instances[instance_ID];
                  const Mesh* mesh = &m_scene->meshes[instance->mesh_ID];
                  Transform* transform = &m_scene->transforms[instance->transform_ID];
                  transform->scale = glm::vec3(1.0f);
      
                  glm::mat4 MW = glm::mat4(1.0f);
                  MW = translate(-transform->rotation_origin) * MW;
                  MW = mat4_cast(transform->rotation) * MW;
                  MW = translate(transform->rotation_origin) * MW;
                  MW = scale(transform->scale) * MW;
                  MW = translate(transform->translation) * MW;

                  glm::mat4 MVP = VP * MW;
      
                  glm::mat3 N_MW = glm::mat4(1.0f);
                  N_MW = mat3_cast(transform->rotation) * N_MW;
                  N_MW = glm::mat3(scale(1.0f / transform->scale)) * N_MW;
      
                  glUniformMatrix4fv(SCENE_MW_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(MW));
                  glUniformMatrix4fv(SCENE_N_MW_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(N_MW));
                  glUniformMatrix4fv(SCENE_MVP_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(MVP));

                  glActiveTexture(GL_TEXTURE0 + SCENE_DIFFUSE_MAP_TEXTURE_BINDING);

                  glBindVertexArray(mesh->mesh_VAO);
                  for(size_t mesh_draw_index = 0; mesh_draw_index < mesh->draw_commands.size(); mesh_draw_index++)
                  {
                        const GLDrawElementsIndirectCommand* draw_cmd = &mesh->draw_commands[mesh_draw_index];
                        const Material* material = &m_scene->materials[mesh->material_IDs[mesh_draw_index]];

                        glActiveTexture(GL_TEXTURE0 + SCENE_DIFFUSE_MAP_TEXTURE_BINDING);
                        if(material->diffuse_map_ID == -1)
                        {
                              glBindTexture(GL_TEXTURE_2D, 0);
                              glUniform1i(SCENE_HAS_DIFFUSE_MAP_UNIFORM_LOCATION, 0);
                        }
                        else
                        {
                              const DiffuseMap* diffuse_map = &m_scene->diffuse_maps[material->diffuse_map_ID];
                              glBindTexture(GL_TEXTURE_2D, diffuse_map->DiffuseMapTO);
                              glUniform1i(SCENE_HAS_DIFFUSE_MAP_UNIFORM_LOCATION, 1);
                        }

                        glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES,
                                                                      draw_cmd->count,
                                                                      GL_UNSIGNED_INT,
                                                                      (GLvoid*)(sizeof(uint32_t) * draw_cmd->firstIndex),
                                                                      draw_cmd->primCount,
                                                                      draw_cmd->baseVertex,
                                                                      draw_cmd->baseInstance);
                                                                       
                  
                  }
                  
                  glBindVertexArray(0);
            }
            // glDepthFunc(GL_LESS);
            glDisable(GL_DEPTH_TEST);
      }
};
      
IRenderer* NewRenderer()
{
      return new Renderer();
}
