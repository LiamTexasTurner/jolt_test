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
      GLuint* m_shadow_map_SP;
      GLuint* m_debug_depth_map_SP;
      GLuint* m_blit_texture_SP;
      GLuint* m_blit_test_SP;

      GLuint back_buffer_FBO;
      GLuint back_buffer_CT;
      GLuint back_buffer_DT;
      unsigned int SCR_WIDTH;
      unsigned int SCR_HEIGHT;

      GLuint shadow_map_FBO;
      GLuint shadow_map_T;
      const unsigned int SHADOW_WIDTH = 4096 * 4, SHADOW_HEIGHT = 4096 * 4;
      bool render_shadow_map = false;

      GLuint m_null_vao;

      void DrawDebugQuad(){render_shadow_map = !render_shadow_map;}
      
      
      void Init(Scene* scene) override
      {
            m_scene = scene;
            unlit.ShaderInit("../shaders/unlit.vs", "../shaders/unlit.fs");

            m_shaders.SetVersion("440");
            m_shaders.SetPreambleFile("../penis/preamble.glsl");
            m_scene_SP = m_shaders.AddProgramFromExts({"../shaders/scene.vert", "../shaders/scene.frag"});
            m_shadow_map_SP = m_shaders.AddProgramFromExts({"../shaders/shadow_mapping_depth.vert", "../shaders/shadow_mapping_depth.frag"});
            m_debug_depth_map_SP = m_shaders.AddProgramFromExts({"../shaders/blit.vert", "../shaders/debug_depth_map.frag"});
            m_blit_texture_SP = m_shaders.AddProgramFromExts({"../shaders/blit.vert", "../shaders/blit_texture.frag"});
            m_blit_test_SP = m_shaders.AddProgramFromExts({"../shaders/blit.vert", "../shaders/blit_test.frag"});


            glGenVertexArrays(1, &m_null_vao);
            glBindVertexArray(m_null_vao);
            glBindVertexArray(0);
      }

      void Resize(int width, int height) override
      {
            SCR_WIDTH = width;
            SCR_HEIGHT = height;
            glGenFramebuffers(1, &shadow_map_FBO);
            glGenTextures(1, &shadow_map_T);
            
            glBindTexture(GL_TEXTURE_2D, shadow_map_T);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  
            // attach depth texture as FBO's depth buffer
            glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_FBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_map_T, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
      }

      void Paint() override
      {

            glClearColor(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            m_shaders.UpdatePrograms();

            glEnable(GL_CULL_FACE);      
            glCullFace(GL_BACK);
            glFrontFace(GL_CCW);



            glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 300.0f);

            Camera* cam = &m_scene->cameras[m_scene->main_camera_ID];

            glm::mat4 VP = projection * cam->GetViewMatrix();

            float near_plane = -1.0f, far_plane = 20.5f;
            float ortho_size = 20.0f;
            glm::vec3 light_pos = glm::vec3(-2.0f, 10.0f, -1.0f);
            
            glm::mat4 light_projection = glm::ortho(-ortho_size, ortho_size, -ortho_size, ortho_size, near_plane, far_plane);
            glm::mat4 light_view = glm::lookAt(light_pos, 
                                               glm::vec3(0.0f), 
                                               glm::vec3( 0.0f, 1.0f,  0.0f));
            glm::mat4 light_space_matrix = light_projection * light_view;

            float intensity = 10.0f;

            glm::vec3 dir_light_col = glm::vec3(78.0f + intensity,
                                                77.0f + intensity,
                                                92.0f + intensity) / 100.0f;

            //render shadow
            glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_FBO);
            glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
            glEnable(GL_DEPTH_TEST);     
            glClear(GL_DEPTH_BUFFER_BIT);

            glUseProgram(*m_shadow_map_SP);
            


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

                  glUniformMatrix4fv(SHADOW_MAP_MW_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(MW));
                  glUniformMatrix4fv(SHADOW_MAP_LIGHT_SPACE_MATRIX_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(light_space_matrix));

                  glBindVertexArray(mesh->mesh_VAO);
                  for(size_t mesh_draw_index = 0; mesh_draw_index < mesh->draw_commands.size(); mesh_draw_index++)
                  {
                        const GLDrawElementsIndirectCommand* draw_cmd = &mesh->draw_commands[mesh_draw_index];
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
            glUseProgram(0);
            glDisable(GL_DEPTH_TEST);
            
            
            
            
            glBindFramebuffer(GL_FRAMEBUFFER, 0);



            //render scene
            glUseProgram(*m_scene_SP);
            glUniform3fv(SCENE_CAMERAPOS_UNIFORM_LOCATION, 1, glm::value_ptr(cam->translation));
            glEnable(GL_DEPTH_TEST);
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
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
                  glUniformMatrix4fv(SCENE_LIGHT_SPACE_MATRIX_UNIFORM_LOCATION, 1, GL_FALSE, value_ptr(light_space_matrix));
                  glUniform3fv(SCENE_LIGHT_POS, 1, value_ptr(light_pos));
                  glUniform3fv(SCENE_LIGHT_COLOR, 1, value_ptr(dir_light_col));

                  glActiveTexture(GL_TEXTURE0 + SCENE_SHADOW_MAP_TEXTURE_BINDING);
                  glBindTexture(GL_TEXTURE_2D, shadow_map_T);
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
            glUseProgram(0);
            glDisable(GL_DEPTH_TEST);

      
            if(render_shadow_map)
            {
                  glDisable(GL_CULL_FACE);
                  glViewport(0,0,SCR_WIDTH, SCR_HEIGHT);
                  glUseProgram(*m_debug_depth_map_SP);
                  glBindVertexArray(m_null_vao);
                  glActiveTexture(GL_TEXTURE0 + DEBUG_DEPTH_MAP_TEXURE_BINDING);
                  glBindTexture(GL_TEXTURE_2D, shadow_map_T);
                  glUniform1f(DEBUG_DEPTH_MAP_NEAR_PLANE, near_plane);
                  glUniform1f(DEBUG_DEPTH_MAP_FAR_PLANE, far_plane);
                  glDrawArrays(GL_TRIANGLES, 0, 3);
            }            
      }
};
      
IRenderer* NewRenderer()
{
      return new Renderer();
}
