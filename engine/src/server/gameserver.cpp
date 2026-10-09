
#include "common/common.h"
#include "server/gameserver.hpp"
#include "math/vector.h"
#include "server/player.hpp"
#include "entity.h"

void CGameServer::InitPlayer( CServerPlayer* pPlayer )
{
    pPlayer->physbody = CreatePhysicsObject(
            VEC_ZERO, VEC_ZERO,
            (vec3_t){ CM2UNITS(50), CM2UNITS(180), CM2UNITS(50)},
            VEC_ZERO, 80.0f
            );
}



