#include "physics/physcollision.hpp"
#include "common/common.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <float.h>

// =====================================================================
// Narrowphase
// =====================================================================

// AABB vs AABB. Normal points a -> b along the axis of least penetration.
// The manifold is the 4 corners of the overlap rectangle on that face, which gives
// stable resting contact for axis aligned boxes (and sets up the same data an OBB clipper will produce later).
static FORCEINLINE bool AABBCollide(
    const vec3_t pa, const vec3_t ha,
    const vec3_t pb, const vec3_t hb,
    collision_event_t& ev )
{
    vec3_t d, pen;

    for (int i = 0; i < 3; i++)
    {
        d[i] = pb[i] - pa[i];
        pen[i] = ha[i] + hb[i] - fabsf( d[i] );

        if (pen[i] <= 0.0f)
            return false;
    }


    int axis = 0;
    if (pen[1] < pen[axis]) axis = 1;
    if (pen[2] < pen[axis]) axis = 2;
    #ifdef PHYS_PRINTS
    printf( "pa=(%f %f %f) ha=(%f %f %f) pb=(%f %f %f) hb=(%f %f %f) pen=(%f %f %f) axis=%d\n",
            pa[0], pa[1], pa[2], ha[0], ha[1], ha[2], pb[0], pb[1], pb[2], hb[0], hb[1], hb[2],
            pen[0], pen[1], pen[2], axis );
    #endif

    int u = (axis + 1) % 3;
    int v = (axis + 2) % 3;

    VectorSet( ev.normal, 0.0f, 0.0f, 0.0f );
    ev.normal[axis] = d[axis] > 0.0f ? 1.0f : -1.0f;

    // Overlap box
    vec3_t lo, hi;
    for (int i = 0; i < 3; i++)
    {
        lo[i] = fmaxf( pa[i] - ha[i], pb[i] - hb[i] );
        hi[i] = fminf( pa[i] + ha[i], pb[i] + hb[i] );
    }

    float mid = 0.5f * (lo[axis] + hi[axis]);
    uint32_t face = (uint32_t)axis * 2u + (d[axis] > 0.0f ? 1u : 0u);

    ev.count = 4;
    for (int i = 0; i < 4; i++)
    {
        contact_point_t& c = ev.points[i];
        c = contact_point_t();

        c.position[axis] = mid;
        c.position[u] = (i & 1) ? hi[u] : lo[u];
        c.position[v] = (i & 2) ? hi[v] : lo[v];
        c.depth = pen[axis];
        c.feature_id = face * 4u + (uint32_t)i;
    }

    return true;
}

// AABB (rotation locked) vs plane. Keeps the deepest corners, up to MAX_CONTACT_POINTS.
static FORCEINLINE bool AABBPlaneCollide(
    const vec3_t pos, const vec3_t halfs,
    const vec3_t n, float d,
    collision_event_t& ev )
{
    struct cand_t
    {
        vec3_t p;
        float depth;
        uint32_t id;
    };

    cand_t cands[8];
    int num = 0;

    for (uint32_t i = 0; i < 8; i++)
    {
        vec3_t p = {
            pos[0] + ((i & 1) ? halfs[0] : -halfs[0]),
            pos[1] + ((i & 2) ? halfs[1] : -halfs[1]),
            pos[2] + ((i & 4) ? halfs[2] : -halfs[2])
        };


        float depth = d - VectorDot( n, p ); // > 0 when behind the plane

        if (depth > 0.0f)
        {
            VectorCopy( p, cands[num].p );
            cands[num].depth = depth;
            cands[num].id = i;
            num++;
        }
    }

    if (num == 0)
        return false;

    ev.count = num < MAX_CONTACT_POINTS ? num : MAX_CONTACT_POINTS;

    for (int k = 0; k < ev.count; k++)
    {
        int best = k;
        for (int j = k + 1; j < num; j++)
            if (cands[j].depth > cands[best].depth)
                best = j;

        cand_t tmp = cands[k];
        cands[k] = cands[best];
        cands[best] = tmp;

        contact_point_t& c = ev.points[k];
        c = contact_point_t();

        VectorCopy( cands[k].p, c.position );
        c.depth = cands[k].depth;
        c.feature_id = cands[k].id;
    }

    VectorCopy( n, ev.normal );
    return true;
}
struct obb_t { vec3_t origin, halfs, axis[3]; };

static FORCEINLINE bool OBBPlaneCollide( const obb_t& box, const vec3_t n, float d, collision_event_t& ev )
{
    struct cand_t
    {
        vec3_t p;
        float depth;
        uint32_t id;
    };

    cand_t cands[8];
    int num = 0;

    for (uint32_t i = 0; i < 8; i++)
    {

        vec3_t p;
        VectorCopy( box.origin, p );
        for (int k = 0; k < 3; k++)
            VectorMA( p, ((i >> k) & 1 ? 1.0f : -1.0f) * box.halfs[k], box.axis[k], p );

        float depth = d - VectorDot( n, p ); // > 0 when behind the plane

        if (depth > 0.0f)
        {
            VectorCopy( p, cands[num].p );
            cands[num].depth = depth;
            cands[num].id = i;
            num++;
        }
    }

    if (num == 0)
        return false;

    ev.count = num < MAX_CONTACT_POINTS ? num : MAX_CONTACT_POINTS;

    for (int k = 0; k < ev.count; k++)
    {
        int best = k;
        for (int j = k + 1; j < num; j++)
            if (cands[j].depth > cands[best].depth)
                best = j;

        cand_t tmp = cands[k];
        cands[k] = cands[best];
        cands[best] = tmp;

        contact_point_t& c = ev.points[k];
        c = contact_point_t();

        VectorCopy( cands[k].p, c.position );
        c.depth = cands[k].depth;
        c.feature_id = cands[k].id;
    }

    VectorCopy( n, ev.normal );
    return true;

}



// > 0 means the projections overlap on L (L must be unit length)
static FORCEINLINE float SatPen( const obb_t& A, const obb_t& B, const vec3_t t, const vec3_t L )
{
    float ra = 0.0f, rb = 0.0f;
    for (int k = 0; k < 3; k++)
    {
        ra += A.halfs[k] * fabsf( VectorDot( A.axis[k], L ) );
        rb += B.halfs[k] * fabsf( VectorDot( B.axis[k], L ) );
    }
    return ra + rb - fabsf( VectorDot( t, L ) );
}

struct clip_vert_t { vec3_t p; uint32_t id; };

// Keep the part of the polygon where dot(pn, p) <= limit
static int ClipPoly( const clip_vert_t* in, int n, clip_vert_t* out,
                     const vec3_t pn, float limit, uint32_t plane )
{
    int m = 0;
    for (int i = 0; i < n; i++)
    {
        const clip_vert_t& a = in[i];
        const clip_vert_t& b = in[(i + 1) % n];
        float da = VectorDot( pn, a.p ) - limit;
        float db = VectorDot( pn, b.p ) - limit;

        if (da <= 0.0f)
            out[m++] = a;

        if ((da < 0.0f && db > 0.0f) || (da > 0.0f && db < 0.0f))
        {
            float s = da / (da - db);
            clip_vert_t v;
            for (int k = 0; k < 3; k++)
                v.p[k] = a.p[k] + s * (b.p[k] - a.p[k]);
            v.id = ((plane + 1u) << 4) | (a.id & 15u);
            out[m++] = v;
        }
    }
    return m;
}

// Reference box owns the face we clip against. rn = outward normal of that face.
static void BuildFaceManifold( const obb_t& ref, int refIdx, const vec3_t rn, uint32_t refFace,
                               const obb_t& inc, collision_event_t& ev )
{
    // Incident face: the face of 'inc' most anti-parallel to rn
    int k = 0; float best = -1.0f;
    for (int i = 0; i < 3; i++)
    {
        float d = fabsf( VectorDot( rn, inc.axis[i] ) );
        if (d > best) { best = d; k = i; }
    }
    float sgn = VectorDot( inc.axis[k], rn ) > 0.0f ? -1.0f : 1.0f;
    uint32_t incFace = (uint32_t)k * 2u + (sgn > 0.0f ? 1u : 0u);

    vec3_t fc;
    VectorMA( inc.origin, sgn * inc.halfs[k], inc.axis[k], fc );

    int iu = (k + 1) % 3, iv = (k + 2) % 3;
    static const float su[4] = { 1, -1, -1,  1 };
    static const float sv[4] = { 1,  1, -1, -1 };

    clip_vert_t bufA[16], bufB[16];
    clip_vert_t* cur = bufA; clip_vert_t* nxt = bufB;
    int n = 4;
    for (int i = 0; i < 4; i++)
    {
        VectorCopy( fc, cur[i].p );
        VectorMA( cur[i].p, su[i] * inc.halfs[iu], inc.axis[iu], cur[i].p );
        VectorMA( cur[i].p, sv[i] * inc.halfs[iv], inc.axis[iv], cur[i].p );
        cur[i].id = (uint32_t)i;
    }

    // Clip against the 4 side planes of the reference face
    uint32_t plane = 0;
    for (int s = 1; s <= 2; s++)
    {
        int ax = (refIdx + s) % 3;
        for (int sign = -1; sign <= 1; sign += 2, plane++)
        {
            vec3_t pn;
            VectorScale( ref.axis[ax], (float)sign, pn );
            float limit = VectorDot( pn, ref.origin ) + ref.halfs[ax];
            n = ClipPoly( cur, n, nxt, pn, limit, plane );
            clip_vert_t* t = cur; cur = nxt; nxt = t;
            if (n == 0) return;
        }
    }

    // Keep points that are below the reference face
    struct cand_t { vec3_t p; float depth; uint32_t id; };
    cand_t cands[16];
    int num = 0;
    for (int i = 0; i < n; i++)
    {
        vec3_t d;
        VectorSub( cur[i].p, ref.origin, d );
        float depth = ref.halfs[refIdx] - VectorDot( rn, d );
        if (depth < 0.0f) continue;

        cands[num].depth = depth;
        cands[num].id = cur[i].id;
        VectorMA( cur[i].p, 0.5f * depth, rn, cands[num].p ); // midway between the two surfaces
        num++;
    }
    if (num == 0) return;

    // Reduce to at most 4 well-spread points
    int pick[MAX_CONTACT_POINTS], np = 0;
    if (num <= MAX_CONTACT_POINTS)
    {
        for (int i = 0; i < num; i++) pick[np++] = i;
    }
    else
    {
        bool used[16] = {};
        int i0 = 0;
        for (int i = 1; i < num; i++) if (cands[i].depth > cands[i0].depth) i0 = i;
        used[i0] = true; pick[np++] = i0;

        int i1 = -1; float far2 = -1.0f;
        for (int i = 0; i < num; i++)
        {
            if (used[i]) continue;
            vec3_t d; VectorSub( cands[i].p, cands[i0].p, d );
            float l2 = VectorDot( d, d );
            if (l2 > far2) { far2 = l2; i1 = i; }
        }
        used[i1] = true; pick[np++] = i1;

        vec3_t e; VectorSub( cands[i1].p, cands[i0].p, e );
        int hi = -1, lo = -1; float chi = -FLT_MAX, clo = FLT_MAX;
        for (int i = 0; i < num; i++)
        {
            if (used[i]) continue;
            vec3_t d, c; VectorSub( cands[i].p, cands[i0].p, d );
            VectorCross( e, d, c );
            float v = VectorDot( c, rn );
            if (v > chi) { chi = v; hi = i; }
            if (v < clo) { clo = v; lo = i; }
        }
        pick[np++] = hi;
        if (lo != hi) pick[np++] = lo;
    }

    ev.count = np;
    for (int k2 = 0; k2 < np; k2++)
    {
        const cand_t& s = cands[pick[k2]];
        contact_point_t& c = ev.points[k2];
        c = contact_point_t();
        VectorCopy( s.p, c.position );
        c.depth = s.depth;
        c.feature_id = (refFace << 24) | (incFace << 16) | s.id;
    }
}

// Single contact between the two supporting edges
static void BuildEdgeManifold( const obb_t& A, int ia, const obb_t& B, int ib,
                               const vec3_t n, float depth, collision_event_t& ev )
{
    vec3_t PA, PB;
    VectorCopy( A.origin, PA );
    VectorCopy( B.origin, PB );
    for (int k = 0; k < 3; k++)
    {
        if (k != ia)
            VectorMA( PA, (VectorDot( A.axis[k], n ) > 0.0f ? 1.0f : -1.0f) * A.halfs[k], A.axis[k], PA );
        if (k != ib)
            VectorMA( PB, (VectorDot( B.axis[k], n ) > 0.0f ? -1.0f : 1.0f) * B.halfs[k], B.axis[k], PB );
    }

    const float* d1 = A.axis[ia];
    const float* d2 = B.axis[ib];
    vec3_t r; VectorSub( PA, PB, r );
    float b = VectorDot( d1, d2 );
    float c = VectorDot( d1, r );
    float f = VectorDot( d2, r );
    float denom = 1.0f - b * b;          // > 0, parallel edges were skipped

    float s = (b * f - c) / denom;
    float t = b * s + f;
    s = Clamp( s, -A.halfs[ia], A.halfs[ia] );
    t = Clamp( t, -B.halfs[ib], B.halfs[ib] );

    vec3_t qa, qb;
    VectorMA( PA, s, d1, qa );
    VectorMA( PB, t, d2, qb );

    ev.count = 1;
    contact_point_t& cp = ev.points[0];
    cp = contact_point_t();
    for (int k = 0; k < 3; k++) cp.position[k] = 0.5f * (qa[k] + qb[k]);
    cp.depth = depth;
    cp.feature_id = 0x80000000u | ((uint32_t)(ia * 3 + ib) << 8);
}

static bool OBBCollide( const obb_t& A, const obb_t& B, collision_event_t& ev )
{
    vec3_t t; VectorSub( B.origin, A.origin, t );

    // Face axes (3 from A, 3 from B)
    float bestFace = FLT_MAX; int faceOwner = 0, faceIdx = 0;
    for (int i = 0; i < 3; i++)
    {
        float p = SatPen( A, B, t, A.axis[i] );
        if (p <= 0.0f) return false;
        if (p < bestFace) { bestFace = p; faceOwner = 0; faceIdx = i; }
    }
    for (int j = 0; j < 3; j++)
    {
        float p = SatPen( A, B, t, B.axis[j] );
        if (p <= 0.0f) return false;
        if (p < bestFace) { bestFace = p; faceOwner = 1; faceIdx = j; }
    }

    // Edge axes (9 cross products)
    float bestEdge = FLT_MAX; int ei = -1, ej = -1; vec3_t edgeL = { 0, 0, 0 };
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
        {
            vec3_t L; VectorCross( A.axis[i], B.axis[j], L );
            float len2 = VectorDot( L, L );
            if (len2 < 1e-6f) continue;              // parallel edges, covered by face axes
            VectorScale( L, 1.0f / sqrtf( len2 ), L );

            float p = SatPen( A, B, t, L );
            if (p <= 0.0f) return false;
            if (p < bestEdge) { bestEdge = p; ei = i; ej = j; VectorCopy( L, edgeL ); }
        }

    // Prefer faces unless an edge is clearly better. Avoids flickering between the two.
    bool useEdge = ei >= 0 && bestEdge < bestFace * 0.95f - 1e-4f;

    if (useEdge)
    {
        if (VectorDot( edgeL, t ) < 0.0f) VectorScale( edgeL, -1.0f, edgeL );
        VectorCopy( edgeL, ev.normal );
        BuildEdgeManifold( A, ei, B, ej, ev.normal, bestEdge, ev );
        return true;
    }

    const obb_t& refBox = faceOwner == 0 ? A : B;
    const obb_t& incBox = faceOwner == 0 ? B : A;

    vec3_t L; VectorCopy( refBox.axis[faceIdx], L );
    // normal points A -> B
    if (VectorDot( L, t ) < 0.0f) VectorScale( L, -1.0f, L );
    VectorCopy( L, ev.normal );

    // outward normal of the reference face: A's faces point toward B, B's toward A
    vec3_t rn;
    if (faceOwner == 0) { VectorCopy( ev.normal, rn ); }
    else               { VectorScale( ev.normal, -1.0f, rn ); }

    uint32_t refFace = (faceOwner ? 6u : 0u) + (uint32_t)faceIdx * 2u
        + (VectorDot( refBox.axis[faceIdx], rn ) > 0.0f ? 1u : 0u);

    ev.count = 0;
    BuildFaceManifold( refBox, faceIdx, rn, refFace, incBox, ev );
    return ev.count > 0;
}

bool CPhysicsCollisionSolver::TestCollisionOBBvsOBB( CBodyblockManager& BM,
                                                     const physobjid_t& a, const physobjid_t& b, collision_event_t& ev )
{
    obb_t A, B;
    BM.GetOBB( a, A.origin, A.halfs, A.axis );
    BM.GetOBB( b, B.origin, B.halfs, B.axis );

    if (!OBBCollide( A, B, ev )) return false;

    ev.a = a; ev.b = b;
    ev.friction = 1.0f;
    ev.restitution = 1.0f;
    m_collisionEvents.push_back( ev );
    return true;
}


bool CPhysicsCollisionSolver::TestCollisionAABBvsAABB(
    CBodyblockManager& BlockManager,
    const physobjid_t& a,
    const physobjid_t& b,
    collision_event_t& event_out )
{
    aabb_t A, B;
    BlockManager.GetAABB( a, A.origin, A.halfs );
    BlockManager.GetAABB( b, B.origin, B.halfs );

    if (!AABBCollide( A.origin, A.halfs, B.origin, B.halfs, event_out ))
        return false;

    event_out.a = a;
    event_out.b = b;
    event_out.friction = m_defaultFriction;
    event_out.restitution = m_defaultRestitution;

    m_collisionEvents.push_back( event_out );
    return true;
}

bool CPhysicsCollisionSolver::TestCollisionAABBvsPlane(
    CBodyblockManager& BlockManager,
    const physobjid_t& body,
    const vec3_t pnorm,
    float pdist,
    collision_event_t& event_out )
{
    aabb_t aabb;
    BlockManager.GetAABB( body, aabb.origin, aabb.halfs );

    if (!AABBPlaneCollide( aabb.origin, aabb.halfs, pnorm, pdist, event_out ))
        return false;

    event_out.a = PHYSOBJ_WORLD;
    event_out.b = body;
    event_out.friction = m_defaultFriction;
    event_out.restitution = m_defaultRestitution;

    m_collisionEvents.push_back( event_out );
    return true;
}


bool CPhysicsCollisionSolver::TestCollisionOBBvsPlane( CBodyblockManager& BlockManager, const physobjid_t& body, const vec3_t pnorm, float pdist, collision_event_t& event_out )
{
    obb_t obb;
    BlockManager.GetOBB( body, obb.origin, obb.halfs, obb.axis );
    
    if (!OBBPlaneCollide( obb, pnorm, pdist, event_out )){
        return false;
    }
    event_out.a = PHYSOBJ_WORLD;
    event_out.b = body;
    event_out.friction = m_defaultFriction;
    event_out.restitution = m_defaultRestitution;

    m_collisionEvents.push_back( event_out );
    return true;
}



// =====================================================================
// Solver helpers
// =====================================================================

// out = R * diag(inv_inertia) * R^T * v
static FORCEINLINE void ApplyInvInertia( const solver_body_t& b, const vec3_t v, vec3_t out )
{
    const float* m = b.rot; // column i starts at m[4 * i]

    float l0 = (m[0] * v[0] + m[1] * v[1] + m[2] * v[2]) * b.inv_inertia[0];
    float l1 = (m[4] * v[0] + m[5] * v[1] + m[6] * v[2]) * b.inv_inertia[1];
    float l2 = (m[8] * v[0] + m[9] * v[1] + m[10] * v[2]) * b.inv_inertia[2];

    out[0] = m[0] * l0 + m[4] * l1 + m[8] * l2;
    out[1] = m[1] * l0 + m[5] * l1 + m[9] * l2;
    out[2] = m[2] * l0 + m[6] * l1 + m[10] * l2;
}

static FORCEINLINE void PointVelocity( const solver_body_t& b, const vec3_t r, vec3_t out )
{
    vec3_t wxr;
    VectorCross( b.angvel, r, wxr );
    VectorAdd( b.vel, wxr, out );
}

static FORCEINLINE float RelativeVelocityAlong(
    const solver_body_t& A, const solver_body_t& B,
    const vec3_t ra, const vec3_t rb, const vec3_t dir )
{
    vec3_t va, vb, rel;
    PointVelocity( A, ra, va );
    PointVelocity( B, rb, vb );
    VectorSub( vb, va, rel );
    return VectorDot( rel, dir );
}

static FORCEINLINE void ApplyImpulse(
    solver_body_t& A, solver_body_t& B,
    const vec3_t ra, const vec3_t rb, const vec3_t P )
{
    vec3_t t, dw;

    VectorMA( A.vel, -A.inv_mass, P, A.vel );
    VectorCross( ra, P, t );
    ApplyInvInertia( A, t, dw );
    VectorSub( A.angvel, dw, A.angvel );

    VectorMA( B.vel, B.inv_mass, P, B.vel );
    VectorCross( rb, P, t );
    ApplyInvInertia( B, t, dw );
    VectorAdd( B.angvel, dw, B.angvel );
}

// 1 / (effective inverse mass) for an impulse along dir applied at arms ra, rb.
static FORCEINLINE float EffectiveMass(
    const solver_body_t& A, const solver_body_t& B,
    const vec3_t ra, const vec3_t rb, const vec3_t dir )
{
    vec3_t c, t, termA, termB, sum;

    VectorCross( ra, dir, c );
    ApplyInvInertia( A, c, t );
    VectorCross( t, ra, termA );

    VectorCross( rb, dir, c );
    ApplyInvInertia( B, c, t );
    VectorCross( t, rb, termB );

    VectorAdd( termA, termB, sum );

    float k = A.inv_mass + B.inv_mass + VectorDot( dir, sum );
    return k > 1e-12f ? 1.0f / k : 0.0f;
}

static FORCEINLINE void BuildTangents( const vec3_t n, vec3_t t1, vec3_t t2 )
{
    if (fabsf( n[0] ) >= 0.57735f)
        VectorSet( t1, n[1], -n[0], 0.0f );
    else
        VectorSet( t1, 0.0f, n[2], -n[1] );

    VectorNormalise( t1 );
    VectorCross( n, t1, t2 );
}

static FORCEINLINE float Clampf( float v, float lo, float hi )
{
    return v < lo ? lo : (v > hi ? hi : v);
}

// =====================================================================
// Gather / scatter between the SoA blocks and the solver working set
// =====================================================================

int CPhysicsCollisionSolver::GatherSolverBody( CBodyblockManager& BlockManager, const physobjid_t& id )
{
    auto it = m_solverIndex.find( id );
    if (it != m_solverIndex.end())
        return it->second;

    solver_body_t sb;
    memset( &sb, 0, sizeof( sb ) );
    sb.id = id;
    MatrixIdentity( sb.rot );

    if (id != PHYSOBJ_WORLD)
    {
        blockid_t block;
        bodyid_t body;
        OBJ_ID_SEPARATE( id, body, block );

        CBodyblock& b = BlockManager.m_blocks[block];

        VectorSet( sb.pos, b.x[body], b.y[body], b.z[body] );
        VectorSet( sb.vel, b.vx[body], b.vy[body], b.vz[body] );
        VectorSet( sb.angvel, b.wx[body], b.wy[body], b.wz[body] );

        quat_t q = { b.qx[body], b.qy[body], b.qz[body], b.qw[body] };
        QuatToMatrix( q, sb.rot );

        float mass = 1.0f / b.inv_mass[body];

        if (mass > 0.0f)
        {
            sb.inv_mass = 1.0f / mass;

            if (!(b.state_flags[body] & BODYSTATE_FLAG_ROTATION_LOCKED))
            {
                // Solid box, half extents h:  I_x = m/3 * (hy^2 + hz^2), etc.
                float hx = b.hx[body], hy = b.hy[body], hz = b.hz[body];
                float ix = mass / 3.0f * (hy * hy + hz * hz);
                float iy = mass / 3.0f * (hx * hx + hz * hz);
                float iz = mass / 3.0f * (hx * hx + hy * hy);

                sb.inv_inertia[0] = ix > 0.0f ? 1.0f / ix : 0.0f;
                sb.inv_inertia[1] = iy > 0.0f ? 1.0f / iy : 0.0f;
                sb.inv_inertia[2] = iz > 0.0f ? 1.0f / iz : 0.0f;
            }
        }
    }

    int index = (int)m_solverBodies.size();
    m_solverBodies.push_back( sb );
    m_solverIndex[id] = index;
    return index;
}

void CPhysicsCollisionSolver::ScatterSolverBodies( CBodyblockManager& BlockManager )
{
    for (const solver_body_t& sb : m_solverBodies)
    {
        if (sb.inv_mass == 0.0f)
            continue; // static / world, nothing changed

        blockid_t block;
        bodyid_t body;
        OBJ_ID_SEPARATE( sb.id, body, block );

        CBodyblock& b = BlockManager.m_blocks[block];

        b.vx[body] = sb.vel[0];
        b.vy[body] = sb.vel[1];
        b.vz[body] = sb.vel[2];

        b.wx[body] = sb.angvel[0];
        b.wy[body] = sb.angvel[1];
        b.wz[body] = sb.angvel[2];
    }
}

// =====================================================================
// Solver
// =====================================================================

void CPhysicsCollisionSolver::SolveContacts( CBodyblockManager& BlockManager, float delta_time )
{
    if (m_collisionEvents.empty() || delta_time <= 0.0f)
        return;

    m_solverBodies.clear();
    m_solverIndex.clear();

    // Gather everything first so m_solverBodies never reallocates under a reference.
    for (collision_event_t& ev : m_collisionEvents)
    {
        ev.ia = GatherSolverBody( BlockManager, ev.a );
        ev.ib = GatherSolverBody( BlockManager, ev.b );
    }
    #ifdef PHYS_PRINTS
    for (auto& ev : m_collisionEvents)
        printf( "a=%u ia=%d invm=%f | b=%u ib=%d invm=%f\n",
                ev.a, ev.ia, m_solverBodies[ev.ia].inv_mass,
                ev.b, ev.ib, m_solverBodies[ev.ib].inv_mass );
    #endif

    CarryOverImpulses();
    PrepareContacts( delta_time );
    WarmStart();

    for (int i = 0; i < m_solverIterations; i++)
        IterateContacts();

    ScatterSolverBodies( BlockManager );
}

// Copy accumulated impulses from last tick's manifolds, matched by body pair + feature id.
void CPhysicsCollisionSolver::CarryOverImpulses( void )
{
    for (collision_event_t& ev : m_collisionEvents)
    {
        for (const collision_event_t& prev : m_prevCollisionEvents)
        {
            if (prev.a != ev.a || prev.b != ev.b)
                continue;

            if (VectorDot( prev.normal, ev.normal ) < 0.95f)
                break; // contact orientation changed, old impulses don't apply

            for (int i = 0; i < ev.count; i++)
            {
                for (int j = 0; j < prev.count; j++)
                {
                    if (ev.points[i].feature_id != prev.points[j].feature_id)
                        continue;

                    ev.points[i].normal_impulse = prev.points[j].normal_impulse;
                    ev.points[i].tangent_impulse[0] = prev.points[j].tangent_impulse[0];
                    ev.points[i].tangent_impulse[1] = prev.points[j].tangent_impulse[1];
                }
            }
            break;
        }
    }
}

void CPhysicsCollisionSolver::PrepareContacts( float delta_time )
{
    for (collision_event_t& ev : m_collisionEvents)
    {
        solver_body_t& A = m_solverBodies[ev.ia];
        solver_body_t& B = m_solverBodies[ev.ib];

        BuildTangents( ev.normal, ev.tangent[0], ev.tangent[1] );

        for (int i = 0; i < ev.count; i++)
        {
            contact_point_t& c = ev.points[i];

            VectorSub( c.position, A.pos, c.ra );
            VectorSub( c.position, B.pos, c.rb );

            c.normal_mass = EffectiveMass( A, B, c.ra, c.rb, ev.normal );
            c.tangent_mass[0] = EffectiveMass( A, B, c.ra, c.rb, ev.tangent[0] );
            c.tangent_mass[1] = EffectiveMass( A, B, c.ra, c.rb, ev.tangent[1] );

            // Target separating velocity: positional correction, or a bounce if hitting fast enough.
            float vn = RelativeVelocityAlong( A, B, c.ra, c.rb, ev.normal );

            c.bias = (m_baumgarte / delta_time) * fmaxf( c.depth - m_slop, 0.0f );

            if (vn < -m_restitutionThreshold)
                c.bias = fmaxf( c.bias, -ev.restitution * vn );
        }
    }
}

void CPhysicsCollisionSolver::WarmStart( void )
{

    for (collision_event_t& ev : m_collisionEvents)
    {
        solver_body_t& A = m_solverBodies[ev.ia];
        solver_body_t& B = m_solverBodies[ev.ib];

        for (int i = 0; i < ev.count; i++)
        {
            contact_point_t& c = ev.points[i];

            vec3_t P;
            VectorScale( ev.normal, c.normal_impulse, P );
            VectorMA( P, c.tangent_impulse[0], ev.tangent[0], P );
            VectorMA( P, c.tangent_impulse[1], ev.tangent[1], P );

            ApplyImpulse( A, B, c.ra, c.rb, P );
        }
    }
}

void CPhysicsCollisionSolver::IterateContacts( void )
{
    for (collision_event_t& ev : m_collisionEvents)
    {
        solver_body_t& A = m_solverBodies[ev.ia];
        solver_body_t& B = m_solverBodies[ev.ib];

        // Friction first, so non-penetration has the last word.
        for (int i = 0; i < ev.count; i++)
        {
            contact_point_t& c = ev.points[i];
            float max_friction = ev.friction * c.normal_impulse;

            for (int t = 0; t < 2; t++)
            {
                float vt = RelativeVelocityAlong( A, B, c.ra, c.rb, ev.tangent[t] );
                float lambda = -vt * c.tangent_mass[t];

                float old = c.tangent_impulse[t];
                c.tangent_impulse[t] = Clampf( old + lambda, -max_friction, max_friction );
                lambda = c.tangent_impulse[t] - old;

                vec3_t P;
                VectorScale( ev.tangent[t], lambda, P );
                ApplyImpulse( A, B, c.ra, c.rb, P );
            }
        }

        // Normal. The accumulated impulse is clamped, not the per-iteration one,
        // so a contact may take impulse back but never pull the bodies together.
        for (int i = 0; i < ev.count; i++)
        {
            contact_point_t& c = ev.points[i];

            float vn = RelativeVelocityAlong( A, B, c.ra, c.rb, ev.normal );
            float lambda = c.normal_mass * (-vn + c.bias);

            float old = c.normal_impulse;
            c.normal_impulse = fmaxf( old + lambda, 0.0f );
            lambda = c.normal_impulse - old;

            vec3_t P;
            VectorScale( ev.normal, lambda, P );
            ApplyImpulse( A, B, c.ra, c.rb, P );
        }
    }
}