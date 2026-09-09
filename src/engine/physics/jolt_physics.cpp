#include "jolt_physics.h"

#ifdef JPH_DEBUG
#undef JPH_DEBUG
#endif

#ifdef JPH_ENABLE_ASSERTS
#undef JPH_ENABLE_ASSERTS
#endif

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

using namespace JPH;



// --- Broadphase / object layer boilerplate ---
// Jolt requires you to define your own layer scheme. Minimal two-layer setup:
namespace Layers {
  static constexpr ObjectLayer STATIC = 0;
  static constexpr ObjectLayer DYNAMIC = 1;
  static constexpr ObjectLayer NUM_LAYERS = 2;
};

namespace BroadPhaseLayers {
  static constexpr BroadPhaseLayer STATIC(0);
  static constexpr BroadPhaseLayer DYNAMIC(1);
  static constexpr uint32_t NUM_LAYERS = 2;
};

class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface {
public:
  virtual uint32_t GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NUM_LAYERS; }
  virtual BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer layer) const override {
    return layer == Layers::STATIC ? BroadPhaseLayers::STATIC : BroadPhaseLayers::DYNAMIC;
  }
};

class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter {
public:
  virtual bool ShouldCollide(ObjectLayer, BroadPhaseLayer) const override { return true; }
};

class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter {
public:
  virtual bool ShouldCollide(ObjectLayer a, ObjectLayer b) const override {
    if (a == Layers::STATIC) return b == Layers::DYNAMIC;
    return true;
  }
};

// --- Global state ---
static PhysicsSystem *g_physics_system = nullptr;
static TempAllocatorImpl *g_temp_allocator = nullptr;
static JobSystemThreadPool *g_job_system = nullptr;
static BPLayerInterfaceImpl g_bp_layer_interface;
static ObjectVsBroadPhaseLayerFilterImpl g_ov_bp_filter;
static ObjectLayerPairFilterImpl g_ov_ov_filter;

void Physics_Init(void)
{
  RegisterDefaultAllocator();
  Factory::sInstance = new Factory();
  RegisterTypes();

  g_temp_allocator = new TempAllocatorImpl(10 * 1024 * 1024); // 10MB scratch
  g_job_system = new JobSystemThreadPool(
      cMaxPhysicsJobs, cMaxPhysicsBarriers,
      (int)std::thread::hardware_concurrency() - 1);

  const uint32_t max_bodies = 4096;
  const uint32_t num_body_mutexes = 0; // default
  const uint32_t max_body_pairs = 4096;
  const uint32_t max_contact_constraints = 2048;

  g_physics_system = new PhysicsSystem();
  g_physics_system->Init(
      max_bodies, num_body_mutexes, max_body_pairs, max_contact_constraints,
      g_bp_layer_interface, g_ov_bp_filter, g_ov_ov_filter);

  g_physics_system->SetGravity(Vec3(0.0f, -9.81f, 0.0f));
}

void Physics_Update(float delta_time)
{
  const int collision_steps = 1; // increase if you see tunneling at high speed
  g_physics_system->Update(delta_time, collision_steps, g_temp_allocator, g_job_system);
}

void Physics_Shutdown(void)
{
  delete g_physics_system;
  delete g_job_system;
  delete g_temp_allocator;
  UnregisterTypes();
  delete Factory::sInstance;
  Factory::sInstance = nullptr;
}

body_id_t Physics_CreateBoxBody(vec3_t position, vec3_t half_extent, uint8_t is_dynamic)
{
  BodyInterface &bi = g_physics_system->GetBodyInterface();

  BoxShapeSettings shape_settings(Vec3(half_extent[0], half_extent[1], half_extent[2]));
  ShapeSettings::ShapeResult shape_result = shape_settings.Create();
  ShapeRefC shape = shape_result.Get();

  BodyCreationSettings bcs(
      shape,
      RVec3(position[0], position[1], position[2]),
      Quat::sIdentity(),
      is_dynamic ? EMotionType::Dynamic : EMotionType::Static,
      is_dynamic ? Layers::DYNAMIC : Layers::STATIC);

  Body *body = bi.CreateBody(bcs);
  bi.AddBody(body->GetID(), EActivation::Activate);

  return body->GetID().GetIndexAndSequenceNumber();
}

void Physics_GetBodyTransform(body_id_t id, vec3_t out_pos, quat_t out_rot)
{
  BodyID body_id(id);
  BodyInterface &bi = g_physics_system->GetBodyInterface();

  RVec3 pos = bi.GetPosition(body_id);
  Quat rot = bi.GetRotation(body_id);

  out_pos[0] = pos.GetX(); out_pos[1] = pos.GetY(); out_pos[2] = pos.GetZ();
  out_rot[0] = rot.GetX(); out_rot[1] = rot.GetY(); out_rot[2] = rot.GetZ(); out_rot[3] = rot.GetW();
}