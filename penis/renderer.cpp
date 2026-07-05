#include "renderer.hpp"
#include "scene.hpp"
#include "preamble.glsl"
#include "shaderset.h"
#include "shader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Renderer : public IRenderer
{
public:

      Scene* m_scene;
      ShaderSet m_shaders;
      Shader unlit;
      
      void Init(Scene* scene) override
      {
            m_scene = scene;
            unlit.ShaderInit("../shaders/unlit.vs", "../shaders/unlit.fs");
      }

      void Paint() override
      {
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 300.0f);
            glm::mat4 view = glm::lookAt(glm::vec3(0.0f), glm::vec3(0,0,1), glm::vec3(0,1,0));
            unlit.use();

            unlit.setMat4("projection", projection);
            unlit.setMat4("view", view);
            unlit.setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0,0,4)));
            
            for(uint32_t instance_ID : m_scene->instances)
            {
                  const Instance* instance = &m_scene->instances[instance_ID];
                  const Mesh* mesh = &m_scene->meshes[instance->mesh_ID];
                  
                  glBindVertexArray(mesh->mesh_VAO);
                  glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_SHORT, 0);
            }
      }
};

IRenderer* NewRenderer()
{
      return new Renderer();
}
