#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Renderer/DebugRendererSimple.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/ContactListener.h>


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>
#include <fstream>
#include <chrono>
#include <cmath>
#include <string.h>
#include <cstdarg>
#include <cstdio>
#include <thread>
#include <vector>
#include <string_view>
#include "common.hpp"

JPH_SUPPRESS_WARNINGS
using namespace JPH;
using namespace JPH::literals;
using namespace std;


// inline glm::vec3& j_to_glm_vec3(JPH::Vec3& jphVec3){return *reinterpret_cast<glm::vec3*>(&jphVec3);}
inline glm::vec3 j_to_glm_vec3(JPH::Vec3 jphVec3){return glm::vec3(jphVec3.GetX(), jphVec3.GetY(), jphVec3.GetZ());}

// inline glm::quat&  j_to_glm_quat(JPH::Quat& jphQuat){return *reinterpret_cast<glm::quat*>(&jphQuat);}
inline glm::quat j_to_glm_quat(JPH::Quat jphQuat){return glm::quat(jphQuat.GetX(), jphQuat.GetY(), jphQuat.GetZ(), jphQuat.GetW());}

// inline RVec3 glm_to_j_vec(glm::vec3& glm_vec3){return *reinterpret_cast<RVec3*>(&glm_vec3);}
inline RVec3 glm_to_j_vec(glm::vec3 glm_vec3){return RVec3(glm_vec3.x, glm_vec3.y, glm_vec3.z);}

// inline Quat glm_to_j_quat(glm::quat& q){return *reinterpret_cast<Quat*>(&q);}
inline Quat glm_to_j_quat(glm::quat q){return Quat(q.x, q.y, q.z, q.w);}

inline RMat44 glm_to_j_mat4(glm::mat4 m)
{
      return RMat44(
            JPH::Vec4(m[0][0], m[0][1], m[0][2], m[0][3]),
            JPH::Vec4(m[1][0], m[1][1], m[1][2], m[1][3]),
            JPH::Vec4(m[2][0], m[2][1], m[2][2], m[2][3]),
            JPH::Vec4(m[3][0], m[3][1], m[3][2], m[3][3])
      );
}



class pDebugRender : public JPH::DebugRendererSimple
{
public:
      pDebugRender()
      {
            Initialize();
            lines.reserve(10000000);
      }

      

      std::vector<DebugLineVertex> lines;

      void Clear()
      {
            lines.clear();
      }

      void DrawLine(RVec3Arg inFrom, RVec3Arg inTo, ColorArg inColor) override
      {
            lines.push_back({
                  float(inFrom.GetX()),
                  float(inFrom.GetY()),
                  float(inFrom.GetZ()),
                  (float)inColor.r,
                  (float)inColor.g,
                  (float)inColor.b

            });

            lines.push_back({
                  float(inTo.GetX()),
                  float(inTo.GetY()),
                  float(inTo.GetZ()),
                  (float)inColor.r,
                  (float)inColor.g,
                  (float)inColor.b
            });


      }

      void DrawTriangle(
            RVec3Arg inV1,
            RVec3Arg inV2,
            RVec3Arg inV3,
            ColorArg inColor,
            ECastShadow inCastShadow = ECastShadow::Off) override
      {
            DrawLine(inV1, inV2, inColor);
            DrawLine(inV2, inV3, inColor);
            DrawLine(inV3, inV1, inColor);
      }

      void DrawText3D(
            RVec3Arg inPosition,
            const string_view &inString,
            ColorArg inColor = Color::sWhite,
            float inHeight = 0.5f) override
      {
      }
};


static void TraceImpl(const char *inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	// Print to the TTY
	std::cout << buffer << std::endl;
}

static bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, uint inLine)
{
	// Print to the TTY
	std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr? inMessage : "") << std::endl;

	// Breakpoint
	return true;
};

static glm::mat4 InterpMatirx(glm::mat4 old_mat, glm::mat4 new_mat, float alpha)
{
      glm::vec3 old_pos = old_mat[3];
      glm::vec3 new_pos = new_mat[3];
      glm::vec3 interp_pos = glm::mix(old_pos, new_pos, alpha);

      glm::quat old_quat = glm::quat_cast(old_mat);
      glm::quat new_quat = glm::quat_cast(new_mat);
      glm::quat interp_rot = glm::normalize(glm::slerp(old_quat, new_quat, alpha));

      return glm::translate(glm::mat4(1.0f), interp_pos) *
             glm::toMat4(interp_rot);
}

namespace Layers
{
	static constexpr ObjectLayer NON_MOVING = 0;
	static constexpr ObjectLayer MOVING = 1;
	static constexpr ObjectLayer NUM_LAYERS = 2;
};

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
{
public:
	virtual bool					ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
	{
		switch (inObject1)
		{
		      case Layers::NON_MOVING:
			      return inObject2 == Layers::MOVING; // Non moving only collides with moving
		      case Layers::MOVING:
			      return true; // Moving collides with everything
		      default:
			      JPH_ASSERT(false);
			      return false;
		}
	}
};

// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
namespace BroadPhaseLayers
{
	static constexpr BroadPhaseLayer NON_MOVING(0);
	static constexpr BroadPhaseLayer MOVING(1);
	static constexpr uint NUM_LAYERS(2);
};

// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
{
public:
	BPLayerInterfaceImpl()
	{
		// Create a mapping table from object to broad phase layer
		mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
	}

	virtual uint					GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::NUM_LAYERS;
	}

	virtual BroadPhaseLayer			GetBroadPhaseLayer(ObjectLayer inLayer) const override
	{
		JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
		return mObjectToBroadPhase[inLayer];
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char *			GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
	{
		switch ((BroadPhaseLayer::Type)inLayer)
		{
		      case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
		      case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
		      default:													JPH_ASSERT(false); return "INVALID";
		}
	}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
	BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
{
public:
	virtual bool				ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
	{
		switch (inLayer1)
		{
		      case Layers::NON_MOVING:
			      return inLayer2 == BroadPhaseLayers::MOVING;
		      case Layers::MOVING:
			      return true;
		      default:
			      JPH_ASSERT(false);
			      return false;
		}
	}
};

// An example contact listener
class ContactListenerImpl : public ContactListener
{
public:
      bool log = false;
	// See: ContactListener
	virtual ValidateResult	OnContactValidate(const Body &inBody1, const Body &inBody2, RVec3Arg inBaseOffset, const CollideShapeResult &inCollisionResult) override
	{
            if(log)
            {
                  std::cout << "Contact validate callback" << std::endl;      
            }
		

		// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
		return ValidateResult::AcceptAllContactsForThisBodyPair;
	}

	virtual void			OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
	{
            if(log)
            {
                  std::cout << "A contact was added" << std::endl;      
            }
		
	}

	virtual void			OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
	{
            if(log)
            {
                  std::cout << "A contact was persisted" << std::endl;      
            }
		
	}

	virtual void			OnContactRemoved(const SubShapeIDPair &inSubShapePair) override
	{
            if(log)
            {
                  std::cout << "A contact was removed" << std::endl;      
            }
		
	}
};

// An example activation listener
class BodyActivationListenerImpl : public BodyActivationListener
{
public:
      bool log = false;
      
	virtual void		OnBodyActivated(const BodyID &inBodyID, uint64 inBodyUserData) override
	{
            if(log)
            {
                  std::cout << "A body got activated" << std::endl;      
            }
		
	}

	virtual void		OnBodyDeactivated(const BodyID &inBodyID, uint64 inBodyUserData) override
	{
            if(log)
            {
                  std::cout << "A body went to sleep" << std::endl;      
            }
	}
};



class GunzCharacter;

class pJolt
{
public:
      pJolt();

      // Body* AddMeshShapeFromModel(ObjModel model);
      

      PhysicsSettings                   	mPhysicsSettings;
      BodyManager::DrawSettings           mBodyDrawSettings;
      BPLayerInterfaceImpl              	mBroadPhaseLayerInterface;									
	ObjectVsBroadPhaseLayerFilterImpl   mObjectVsBroadPhaseLayerFilter;					
	ObjectLayerPairFilterImpl           mObjectVsObjectLayerFilter;								

      pDebugRender*                       mDebugRender = nullptr;
	TempAllocator *                   	mTempAllocator = nullptr;									
	JobSystem *                       	mJobSystem = nullptr;										
	JobSystem *                       	mJobSystemValidating = nullptr;								
	PhysicsSystem *                   	mPhysicsSystem = nullptr;									
	ContactListenerImpl *             	mContactListener = nullptr;
      BodyActivationListenerImpl*         mBodyActivationListener = nullptr;      

      int                                 mMaxConcurrentJobs = thread::hardware_concurrency();		
	float                               mUpdateFrequency = 60.0f;									
	int                                 mCollisionSteps = 2;


      void CreateSphere(glm::vec3 world_pos, float radius);
      void CreateBox(glm::vec3 world_pos, glm::vec3 dim);
};
