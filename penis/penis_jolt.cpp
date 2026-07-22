#include "penis_jolt.hpp"
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>


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
