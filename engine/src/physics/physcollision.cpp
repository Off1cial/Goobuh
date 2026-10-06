#include "physics/physcollision.hpp"

#include <stdio.h>
/*
|A.x - B.x| <= A.half.x + B.half.x
|A.y - B.y| <= A.half.y + B.half.y
|A.z - B.z| <= A.half.z + B.half.z

*/



static FORCEINLINE bool AABBIntersect(
    float x0, float y0, float z0,
    float hx0, float hy0, float hz0,
    float x1, float y1, float z1,
    float hx1, float hy1, float hz1
)
{
    #ifdef PHYS_PRINTS

    printf("pos0: (%0.2f, %0.2f, %0.2f)\n", x0, y0, z0);
    printf("pos1: (%0.2f, %0.2f, %0.2f)\n", x1, y1, z1);

    printf( "halfs0: (%0.2f, %0.2f, %0.2f)\n", hx0, hy0, hz0 );
    printf( "halfs1: (%0.2f, %0.2f, %0.2f)\n", hx1, hy1, hz1 );

    #endif



    if (fabsf(x0 - x1) > hx0 + hx1)
        return false;

    if (fabsf(y0 - y1) > hy0 + hy1)
        return false;

    if (fabsf(z0 - z1) > hz0 + hz1)
        return false;

    return true;
}


bool CPhysicsCollisionSolver::TestBroadphaseCollision( CBodyblockManager& BlockManager, const physobjid_t& a, const physobjid_t& b )
{
    blockid_t block0, block1;
    bodyid_t body0, body1;
    OBJ_ID_SEPARATE( a, body0, block0 );
    OBJ_ID_SEPARATE( b, body1, block1 );


    CBodyblock* pBlock0 = &BlockManager.m_blocks[block0];
    CBodyblock* pBlock1 = &BlockManager.m_blocks[block1];

    if (!AABBIntersect(
        pBlock0->x[body0], pBlock0->y[body0], pBlock0->z[body0],
        pBlock0->hx[body0], pBlock0->hy[body0], pBlock0->hz[body0],

        pBlock1->x[body1], pBlock1->y[body1], pBlock1->z[body1],
        pBlock1->hx[body1], pBlock1->hy[body1], pBlock1->hz[body1]
    ))
    {
        return false;
    }
    
    return true;
}

bool CPhysicsCollisionSolver::TestCollision( CBodyblockManager& BlockMananger, const physobjid_t& a, const physobjid_t& b, collision_event_t& event_out )
{
    if (!TestBroadphaseCollision( BlockMananger, a, b )){ return false; }

    #ifdef PHYS_PRINTS
    printf("Collision\n");
    #endif

    // Populate the collision event....


    return true;
}