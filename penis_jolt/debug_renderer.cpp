#include "debug_renderer.hpp"
#include "../penis/preamble.glsl"
#include <iostream>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>



inline glm::mat4 j_mat4_to_glm(const JPH::RMat44& m)
{
      JPH::Vec4 column_0 = m.GetColumn4(0);
      JPH::Vec4 column_1 = m.GetColumn4(1);
      JPH::Vec4 column_2 = m.GetColumn4(2);
      JPH::RVec3 translation = m.GetTranslation();

      return glm::mat4(
            glm::vec4(column_0.GetX(), column_0.GetY(), column_0.GetZ(), column_0.GetW()),
            glm::vec4(column_1.GetX(), column_1.GetY(), column_1.GetZ(), column_1.GetW()),
            glm::vec4(column_2.GetX(), column_2.GetY(), column_2.GetZ(), column_2.GetW()),
            glm::vec4(static_cast<float>(translation.GetX()),
                      static_cast<float>(translation.GetY()),
                      static_cast<float>(translation.GetZ()),
                      1.0f)
      );
}



pDebugRenderer::pDebugRenderer()
{
      DebugRenderer::Initialize();
      debug_line_SP.ShaderInit("../shaders/debug_line.vert", "../shaders/debug_line.frag");
}
            
void pDebugRenderer::DrawLine(RVec3Arg inFrom, RVec3Arg inTo, ColorArg inColor) 
{
      float x = 0;
}

void pDebugRenderer::DrawTriangle(RVec3Arg inV1, RVec3Arg inV2, RVec3Arg inV3, ColorArg inColor, ECastShadow inCastShadow) 
{
      float x = 0;
}


DebugRenderer::Batch pDebugRenderer::CreateTriangleBatch(const Triangle *inTriangles, int inTriangleCount) 
{
      DebugRenderer::Batch batch;
      return batch;
}


DebugRenderer::Batch pDebugRenderer::CreateTriangleBatch(const Vertex *inVertices, int inVertexCount, const uint32 *inIndices, int inIndexCount) 
{
      pRenderPrimitive* prim = new pRenderPrimitive{};

      prim->vertex_count = inVertexCount;
      prim->indices_count = inIndexCount;

      glGenVertexArrays(1, &prim->VAO);
      glGenBuffers(1, &prim->VBO);
      glGenBuffers(1, &prim->EBO);

      glBindVertexArray(prim->VAO);
      
      glBindBuffer(GL_ARRAY_BUFFER, prim->VBO);
      glBufferData(GL_ARRAY_BUFFER, inVertexCount * sizeof(Vertex), inVertices, GL_STATIC_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim->EBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, inIndexCount * sizeof(uint32_t), inIndices, GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
      glEnableVertexAttribArray(0);

	return prim;
}


void pDebugRenderer::DrawGeometry(RMat44Arg inModelMatrix,
                                  const AABox &inWorldSpaceBounds,
                                  float inLODScaleSq,
                                  ColorArg inModelColor,
                                  const GeometryRef &inGeometry,
                                  ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode) 
{
      
      pRenderPrimitive* prim = dynamic_cast<pRenderPrimitive*>(inGeometry.GetPtr()->mLODs[(inGeometry.GetPtr()->mLODs.size() - 1)].mTriangleBatch.GetPtr());
      // pRenderPrimitive* prim = dynamic_cast<pRenderPrimitive*>(inGeometry.GetPtr()->mLODs[0].mTriangleBatch.GetPtr());
      
      GLuint VAO = prim->VAO;
      GLuint VBO = prim->VBO;

      
      debug_line_SP.use();      
      
      glBindVertexArray(VAO);      
      
      glm::mat4 model = j_mat4_to_glm(inModelMatrix);
            
      debug_line_SP.setMat4("model", model);
      debug_line_SP.setMat4("view", view);
      debug_line_SP.setMat4("projection", projection);

      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      glDrawElements(GL_TRIANGLES, (GLsizei)prim->indices_count, GL_UNSIGNED_INT, nullptr);

      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}



void pDebugRenderer::DrawText3D(RVec3Arg inPosition,
                                const string_view &inString,
                                ColorArg inColor,
                                float inHeight)
{
      float x = 0;
}


