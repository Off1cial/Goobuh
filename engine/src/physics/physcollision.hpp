#pragma once

#include "physics/shared.h"
#include "physics/backend/blockmanager.hpp"
#include "math/vector.h"
#include "math/matrix.h"
#include "math/quat.h"
#include <stdint.h>
#include <vector>
#include <unordered_map>

#define MAX_CONTACT_POINTS 4

// One point of a contact manifold. Everything below "solver" is owned by the solver.
typedef struct contact_point_t
{
    vec3_t position;     // world space
    float depth;         // > 0 means penetrating
    uint32_t feature_id; // stable id so impulses can be matched across ticks (warm starting)

    // solver
    vec3_t ra, rb;       // contact arm from each body's centre
    float normal_mass;
    float tangent_mass[2];
    float bias;          // target separating velocity (position correction + bounce)

    float normal_impulse;     // accumulated, persists across ticks
    float tangent_impulse[2]; // accumulated, persists across ticks
} contact_point_t;

// A manifold between two bodies. Normal points from a -> b.
typedef struct collision_event_t
{
    physobjid_t a;
    physobjid_t b;

    vec3_t normal;
    float friction;
    float restitution;

    int count;
    contact_point_t points[MAX_CONTACT_POINTS];

    // solver
    int ia, ib;          // indices into the solver's gathered bodies
    vec3_t tangent[2];
} collision_event_t;

// Working copy of one body for the duration of a solve. Gathered from the SoA blocks,
// written back (velocity + angular velocity only) afterwards.
typedef struct solver_body_t
{
    physobjid_t id;
    vec3_t pos;
    vec3_t vel;
    vec3_t angvel;
    mat4 rot;                // rotation matrix, column-major
    vec3_t inv_inertia;      // diagonal of the inverse inertia tensor, body space
    float inv_mass;          // 0 = static
} solver_body_t;

class CPhysicsCollisionSolver
{
public:
    // --- Narrowphase. Return true/false and, on true, push a manifold to m_collisionEvents.
    bool TestCollisionAABBvsAABB( CBodyblockManager& BlockManager, const physobjid_t& a, const physobjid_t& b, collision_event_t& event_out );
    bool TestCollisionOBBvsOBB( CBodyblockManager& BM,
                                                         const physobjid_t& a, const physobjid_t& b, collision_event_t& ev );
    // Plane is  dot(pnorm, p) = pdist, pnorm points away from the solid side.
    // The plane is body A (PHYSOBJ_WORLD), so event.normal == pnorm with no flip.
    bool TestCollisionAABBvsPlane( CBodyblockManager& BlockManager, const physobjid_t& body, const vec3_t pnorm, float pdist, collision_event_t& event_out );
    bool TestCollisionOBBvsPlane( CBodyblockManager& BlockManager, const physobjid_t& body, const vec3_t pnorm, float pdist, collision_event_t& event_out );
    // --- Solve every manifold in m_collisionEvents. Velocity-level only: positions are integrated by the manager.
    void SolveContacts( CBodyblockManager& BlockManager, float delta_time );

    // Tunables. Defaults assume 1 unit = 1 metre; scale lengths if your world uses another unit.
    int   m_solverIterations = 10;
    float m_baumgarte = 0.2f;   // positional correction strength (0..1)
    float m_slop = 0.005f; // allowed overlap, stops resting jitter
    float m_restitutionThreshold = 1.0f;   // approach speed below which nothing bounces
    float m_defaultFriction = 0.5f;
    float m_defaultRestitution = 0.2f;

protected:
    std::vector<collision_event_t> m_collisionEvents;     // This tick
    std::vector<collision_event_t> m_prevCollisionEvents; // Last tick, for warm starting

    void ResetCollisions( void )
    {
        m_prevCollisionEvents.swap( m_collisionEvents );
        m_collisionEvents.clear();
    }

private:
    std::vector<solver_body_t> m_solverBodies;
    std::unordered_map<physobjid_t, int> m_solverIndex;

    int  GatherSolverBody( CBodyblockManager& BlockManager, const physobjid_t& id );
    void ScatterSolverBodies( CBodyblockManager& BlockManager );

    void CarryOverImpulses( void );
    void PrepareContacts( float delta_time );
    void WarmStart( void );
    void IterateContacts( void );
};