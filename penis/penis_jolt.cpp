#include "penis_jolt.hpp"
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>


pJolt::pJolt()
{
      Trace = TraceImpl;

      RegisterDefaultAllocator();
	Factory::sInstance = new Factory();
      
      RegisterTypes();
      mTempAllocator = new TempAllocatorImpl(10 * 1024 * 1024);
      
      mJobSystem = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);


      uint cMaxBodies = 1024;	
	uint cNumBodyMutexes = 0;
	uint cMaxBodyPairs = 1024;
	uint cMaxContactConstraints = 1024;

	mPhysicsSystem = new PhysicsSystem;
      mPhysicsSystem->Init(cMaxBodies,
                           cNumBodyMutexes,
                           cMaxBodyPairs,
                           cMaxContactConstraints,
                           mBroadPhaseLayerInterface,
                           mObjectVsBroadPhaseLayerFilter,
                           mObjectVsObjectLayerFilter);


      
      
	mContactListener = new ContactListenerImpl;
      mBodyActivationListener = new BodyActivationListenerImpl;
      
      mDebugRender = new pDebugRender;

      mBodyDrawSettings.mDrawShape = true;
      mBodyDrawSettings.mDrawShapeWireframe = true;
      mBodyDrawSettings.mDrawShapeColor = BodyManager::EShapeColor::MotionTypeColor;


}

// void pJolt::AddMeshShapeFromModel(ObjModel model)
// {
//       VertexList verts;
//       int vert_size = model.meshes[0].verts.size();
//       verts.resize(vert_size);
//       for(int i = 0; i < verts.size(); i++)
//       {
//             verts[i] =(Float3(model.meshes[0].verts[i].position.x,
//                                model.meshes[0].verts[i].position.y,
//                                model.meshes[0].verts[i].position.z));
//       }
      
//       IndexedTriangleList triangles;
//       triangles.reserve(model.meshes[0].indices.size() / 3);

//       for (size_t i = 0; i + 2 < model.meshes[0].indices.size(); i += 3)
//       {
//             triangles.push_back(IndexedTriangle(
//                                                 (uint32)model.meshes[0].indices[i + 0],
//                                                 (uint32)model.meshes[0].indices[i + 1],
//                                                 (uint32)model.meshes[0].indices[i + 2]
//                                                 ));
//       }

//       MeshShapeSettings mesh_settings(verts, triangles);
//       ShapeSettings::ShapeResult shape_result = mesh_settings.Create();
//       RefConst<Shape> model_shape = shape_result.Get();
//       BodyCreationSettings model_settings(model_shape, RVec3(0,0,0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
//       Body *model_body = mPhysicsSystem->GetBodyInterface().CreateBody(model_settings);
//       mPhysicsSystem->GetBodyInterface().AddBody(model_body->GetID(), EActivation::DontActivate);

// }





// Body* pJolt::AddMeshShapeFromModel(ObjModel model)
// {
      
//       int vert_count = 0;
//       int tri_count = 0;
//       for(int m = 0; m < model.mesh_count; m ++)
//       {
//             vert_count += model.meshes[m].verts.size();
//             tri_count += model.meshes[m].indices.size() / 3;
//       }

//       VertexList verts;
//       verts.reserve(vert_count);
//       IndexedTriangleList triangles;
//       triangles.reserve(tri_count);
      
//       int vert_index_start_offset = 0;
//       int tri_index_start_offset = 0;

//       for(int m = 0; m < model.meshes.size(); m ++)
//       {
//             for(int i = 0; i < model.meshes[m].verts.size(); i++)
//             {
//                   verts.push_back(Float3(model.meshes[m].verts[i].position.x,
//                                          model.meshes[m].verts[i].position.y,
//                                          model.meshes[m].verts[i].position.z));
//             }
//             for (size_t i = 0; i + 2 < model.meshes[m].indices.size(); i += 3)
//             {

//                   triangles.push_back(IndexedTriangle(vert_index_start_offset + (uint32)model.meshes[m].indices[i + 0],
//                                                       vert_index_start_offset + (uint32)model.meshes[m].indices[i + 1],
//                                                       vert_index_start_offset + (uint32)model.meshes[m].indices[i + 2]
//                                                       ));
//             }

//             vert_index_start_offset += model.meshes[m].vert_count;
//             tri_index_start_offset += (model.meshes[m].indices.size() / 3);
//       }
      
      

//       MeshShapeSettings mesh_settings(verts, triangles);
//       ShapeSettings::ShapeResult shape_result = mesh_settings.Create();
//       RefConst<Shape> model_shape = shape_result.Get();
//       BodyCreationSettings model_settings(model_shape, RVec3(0,0,0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
//       Body *model_body = mPhysicsSystem->GetBodyInterface().CreateBody(model_settings);
//       mPhysicsSystem->GetBodyInterface().AddBody(model_body->GetID(), EActivation::DontActivate);

//       return model_body;

// }
