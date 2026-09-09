#ifndef JOLT_PHYSICS_H
#define JOLT_PHYSICS_H
#include <stdint.h>

#ifdef JPH_DEBUG
#undef JPH_DEBUG
#endif

#ifdef JPH_ENABLE_ASSERTS
#undef JPH_ENABLE_ASSERTS
#endif


#include "math/vector.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t body_id_t;

void Physics_Init(void);
void Physics_Shutdown(void);
void Physics_Update(float delta_time);

body_id_t Physics_CreateBoxBody(vec3_t position, vec3_t half_extent, uint8_t is_dynamic);
void Physics_GetBodyTransform(body_id_t id, vec3_t *out_pos, quat_t *out_rot);

#ifdef __cplusplus
}
#endif
#endif
