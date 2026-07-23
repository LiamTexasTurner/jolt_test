#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRenderer.h>
#include <Jolt/Core/Reference.h>
#include <glad/glad.h>
#include <shaderset.h>
#include <glm/glm.hpp>
#include <shader.h>

using namespace JPH;

class pDebugRenderer final : public DebugRenderer
{
public:

      
      pDebugRenderer();
      
      virtual void DrawLine(RVec3Arg inFrom, RVec3Arg inTo, ColorArg inColor) override;
      
	virtual void DrawTriangle(RVec3Arg inV1, RVec3Arg inV2, RVec3Arg inV3, ColorArg inColor, ECastShadow inCastShadow = ECastShadow::Off) override;
      
	virtual Batch CreateTriangleBatch(const Triangle *inTriangles, int inTriangleCount) override;
      
	virtual Batch CreateTriangleBatch(const Vertex *inVertices, int inVertexCount, const uint32 *inIndices, int inIndexCount) override;
      
	virtual void DrawGeometry(RMat44Arg inModelMatrix,
                                const AABox &inWorldSpaceBounds,
                                float inLODScaleSq,
                                ColorArg inModelColor,
                                const GeometryRef &inGeometry,
                                ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode) override;
      
	virtual void DrawText3D(RVec3Arg inPosition,
                              const string_view &inString,
                              ColorArg inColor,
                              float inHeight) override;
      
      Shader debug_line_SP;

      glm::mat4 view;
      glm::mat4 projection;
};

class pRenderPrimitive : public RefTarget<pRenderPrimitive>, public RefTargetVirtual
{
public:
      virtual ~pRenderPrimitive() override = default;

      GLuint VAO;
      GLuint VBO;
      GLuint EBO;

      int vertex_count;
      int indices_count;

      virtual void AddRef() override { RefTarget<pRenderPrimitive>::AddRef(); }
	virtual void Release() override{ RefTarget<pRenderPrimitive>::Release(); }
};
