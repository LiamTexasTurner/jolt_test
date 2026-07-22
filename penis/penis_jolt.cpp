#include "penis_jolt.hpp"
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

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

void pJolt::CreateSphere(glm::vec3 world_pos, float radius)
{
      BodyCreationSettings sphere_settings(new SphereShape(0.5f),
                                           glm_to_j_vec(world_pos),
                                           Quat::sIdentity(),
                                           EMotionType::Dynamic,
                                           Layers::MOVING);
      
      BodyID sphere_ID = mPhysicsSystem->GetBodyInterface().CreateAndAddBody(sphere_settings, EActivation::Activate);

}
void pJolt::CreateBox(glm::vec3 world_pos, glm::vec3 dim)
{
      BoxShapeSettings floor_shape_settings(Vec3(5.0f, 0.2f, 5.0f));
	floor_shape_settings.SetEmbedded(); 
	ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
	ShapeRefC floor_shape = floor_shape_result.Get(); 
	BodyCreationSettings floor_settings(floor_shape, RVec3(0.0_r, -1.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
      BodyInterface& body_interface = mPhysicsSystem->GetBodyInterface();
	Body *floor = body_interface.CreateBody(floor_settings); 
	body_interface.AddBody(floor->GetID(), EActivation::DontActivate);
}
