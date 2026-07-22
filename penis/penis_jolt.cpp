#include "penis_jolt.hpp"
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Constraints/PointConstraint.h>
#include <Jolt/Physics/Constraints/FixedConstraint.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>
#include <Jolt/Physics/Constraints/SliderConstraint.h>
#include <Jolt/Physics/Constraints/ConeConstraint.h>
#include <Jolt/Physics/Constraints/SwingTwistConstraint.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/ObjectStream/ObjectStreamIn.h>
#include <Jolt/ObjectStream/ObjectStreamOut.h>

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

RagdollSettings* pJolt::CreateRagdoll()
{
	// Create skeleton
	Ref<Skeleton> skeleton = new Skeleton;
	uint lower_body = skeleton->AddJoint("LowerBody");
	uint mid_body = skeleton->AddJoint("MidBody", lower_body);
	uint upper_body = skeleton->AddJoint("UpperBody", mid_body);
	/*uint head =*/ skeleton->AddJoint("Head", upper_body);
	uint upper_arm_l = skeleton->AddJoint("UpperArmL", upper_body);
	uint upper_arm_r = skeleton->AddJoint("UpperArmR", upper_body);
	/*uint lower_arm_l =*/ skeleton->AddJoint("LowerArmL", upper_arm_l);
	/*uint lower_arm_r =*/ skeleton->AddJoint("LowerArmR", upper_arm_r);
	uint upper_leg_l = skeleton->AddJoint("UpperLegL", lower_body);
	uint upper_leg_r = skeleton->AddJoint("UpperLegR", lower_body);
	/*uint lower_leg_l =*/ skeleton->AddJoint("LowerLegL", upper_leg_l);
	/*uint lower_leg_r =*/ skeleton->AddJoint("LowerLegR", upper_leg_r);

	// Create shapes for limbs
	Ref<Shape> shapes[] = {
		new CapsuleShape(0.15f, 0.10f),		// Lower Body
		new CapsuleShape(0.15f, 0.10f),		// Mid Body
		new CapsuleShape(0.15f, 0.10f),		// Upper Body
		new CapsuleShape(0.075f, 0.10f),	// Head
		new CapsuleShape(0.15f, 0.06f),		// Upper Arm L
		new CapsuleShape(0.15f, 0.06f),		// Upper Arm R
		new CapsuleShape(0.15f, 0.05f),		// Lower Arm L
		new CapsuleShape(0.15f, 0.05f),		// Lower Arm R
		new CapsuleShape(0.2f, 0.075f),		// Upper Leg L
		new CapsuleShape(0.2f, 0.075f),		// Upper Leg R
		new CapsuleShape(0.2f, 0.06f),		// Lower Leg L
		new CapsuleShape(0.2f, 0.06f),		// Lower Leg R
	};

	// Positions of body parts in world space
	RVec3 positions[] = {
		RVec3(0, 1.15f, 0),					// Lower Body
		RVec3(0, 1.35f, 0),					// Mid Body
		RVec3(0, 1.55f, 0),					// Upper Body
		RVec3(0, 1.825f, 0),				// Head
		RVec3(-0.425f, 1.55f, 0),			// Upper Arm L
		RVec3(0.425f, 1.55f, 0),			// Upper Arm R
		RVec3(-0.8f, 1.55f, 0),				// Lower Arm L
		RVec3(0.8f, 1.55f, 0),				// Lower Arm R
		RVec3(-0.15f, 0.8f, 0),				// Upper Leg L
		RVec3(0.15f, 0.8f, 0),				// Upper Leg R
		RVec3(-0.15f, 0.3f, 0),				// Lower Leg L
		RVec3(0.15f, 0.3f, 0),				// Lower Leg R
	};

	// Rotations of body parts in world space
	Quat rotations[] = {
		Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI),		 // Lower Body
		Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI),		 // Mid Body
		Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI),		 // Upper Body
		Quat::sIdentity(),									 // Head
		Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI),		 // Upper Arm L
		Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI),		 // Upper Arm R
		Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI),		 // Lower Arm L
		Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI),		 // Lower Arm R
		Quat::sIdentity(),									 // Upper Leg L
		Quat::sIdentity(),									 // Upper Leg R
		Quat::sIdentity(),									 // Lower Leg L
		Quat::sIdentity()									 // Lower Leg R
	};

	// World space constraint positions
	RVec3 constraint_positions[] = {
		RVec3::sZero(),				// Lower Body (unused, there's no parent)
		RVec3(0, 1.25f, 0),			// Mid Body
		RVec3(0, 1.45f, 0),			// Upper Body
		RVec3(0, 1.65f, 0),			// Head
		RVec3(-0.225f, 1.55f, 0),	// Upper Arm L
		RVec3(0.225f, 1.55f, 0),	// Upper Arm R
		RVec3(-0.65f, 1.55f, 0),	// Lower Arm L
		RVec3(0.65f, 1.55f, 0),		// Lower Arm R
		RVec3(-0.15f, 1.05f, 0),	// Upper Leg L
		RVec3(0.15f, 1.05f, 0),		// Upper Leg R
		RVec3(-0.15f, 0.55f, 0),	// Lower Leg L
		RVec3(0.15f, 0.55f, 0),		// Lower Leg R
	};

	// World space twist axis directions
	Vec3 twist_axis[] = {
		Vec3::sZero(),				// Lower Body (unused, there's no parent)
		Vec3::sAxisY(),				// Mid Body
		Vec3::sAxisY(),				// Upper Body
		Vec3::sAxisY(),				// Head
		-Vec3::sAxisX(),			// Upper Arm L
		Vec3::sAxisX(),				// Upper Arm R
		-Vec3::sAxisX(),			// Lower Arm L
		Vec3::sAxisX(),				// Lower Arm R
		-Vec3::sAxisY(),			// Upper Leg L
		-Vec3::sAxisY(),			// Upper Leg R
		-Vec3::sAxisY(),			// Lower Leg L
		-Vec3::sAxisY(),			// Lower Leg R
	};

	// Constraint limits
	float twist_angle[] = {
		0.0f,		// Lower Body (unused, there's no parent)
		5.0f,		// Mid Body
		5.0f,		// Upper Body
		90.0f,		// Head
		45.0f,		// Upper Arm L
		45.0f,		// Upper Arm R
		45.0f,		// Lower Arm L
		45.0f,		// Lower Arm R
		45.0f,		// Upper Leg L
		45.0f,		// Upper Leg R
		45.0f,		// Lower Leg L
		45.0f,		// Lower Leg R
	};

	float normal_angle[] = {
		0.0f,		// Lower Body (unused, there's no parent)
		10.0f,		// Mid Body
		10.0f,		// Upper Body
		45.0f,		// Head
		90.0f,		// Upper Arm L
		90.0f,		// Upper Arm R
		0.0f,		// Lower Arm L
		0.0f,		// Lower Arm R
		45.0f,		// Upper Leg L
		45.0f,		// Upper Leg R
		0.0f,		// Lower Leg L
		0.0f,		// Lower Leg R
	};

	float plane_angle[] = {
		0.0f,		// Lower Body (unused, there's no parent)
		10.0f,		// Mid Body
		10.0f,		// Upper Body
		45.0f,		// Head
		45.0f,		// Upper Arm L
		45.0f,		// Upper Arm R
		90.0f,		// Lower Arm L
		90.0f,		// Lower Arm R
		45.0f,		// Upper Leg L
		45.0f,		// Upper Leg R
		60.0f,		// Lower Leg L (cheating here, a knee is not symmetric, we should have rotated the twist axis)
		60.0f,		// Lower Leg R
	};

	// Create ragdoll settings
	RagdollSettings *settings = new RagdollSettings;
	settings->mSkeleton = skeleton;
	settings->mParts.resize(skeleton->GetJointCount());
	for (int p = 0; p < skeleton->GetJointCount(); ++p)
	{
		RagdollSettings::Part &part = settings->mParts[p];
		part.SetShape(shapes[p]);
		part.mPosition = positions[p];
		part.mRotation = rotations[p];
		part.mMotionType = EMotionType::Dynamic;
		part.mObjectLayer = Layers::MOVING;

		// First part is the root, doesn't have a parent and doesn't have a constraint
		if (p > 0)
		{
			SwingTwistConstraintSettings *constraint = new SwingTwistConstraintSettings;
			constraint->mDrawConstraintSize = 0.1f;
			constraint->mPosition1 = constraint->mPosition2 = constraint_positions[p];
			constraint->mTwistAxis1 = constraint->mTwistAxis2 = twist_axis[p];
			constraint->mPlaneAxis1 = constraint->mPlaneAxis2 = Vec3::sAxisZ();
			constraint->mTwistMinAngle = -DegreesToRadians(twist_angle[p]);
			constraint->mTwistMaxAngle = DegreesToRadians(twist_angle[p]);
			constraint->mNormalHalfConeAngle = DegreesToRadians(normal_angle[p]);
			constraint->mPlaneHalfConeAngle = DegreesToRadians(plane_angle[p]);
			part.mToParent = constraint;
		}
	}

	// Optional: Stabilize the inertia of the limbs
	settings->Stabilize();

	// Optional: Calculate constraint priorities to give more priority to the root
	settings->CalculateConstraintPriorities();

	// Disable parent child collisions so that we don't get collisions between constrained bodies
	settings->DisableParentChildCollisions();

	// Calculate the map needed for GetBodyIndexToConstraintIndex()
	settings->CalculateBodyIndexToConstraintIndex();

	return settings;
}
Ragdoll* pJolt::AddRagdoll()
{
      RagdollSettings * ragdoll_settings = CreateRagdoll();
      Ragdoll* ragdoll = ragdoll_settings->CreateRagdoll(0, 0, mPhysicsSystem);
      ragdoll->AddToPhysicsSystem(EActivation::Activate);
      return ragdoll;
}
