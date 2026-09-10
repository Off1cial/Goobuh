#include "engine/physics/backend/physics.h"
#include <stdlib.h>
#include <string.h>

static size_t cache_line = 0x40; 

#if defined(_WIN32)

#include <malloc.h>

// Windows aligned allocation
#define ALIGNED_ALLOC(alignment, size) \
    _aligned_malloc((size), (alignment))

#define ALIGNED_REALLOC(ptr, old_size, new_size, alignment) \
    _aligned_realloc((ptr), (new_size), (alignment))

#define ALIGNED_FREE(ptr) \
    _aligned_free((ptr))

#else

// -------------------------
// POSIX / MSYS2 / Linux fallback
// -------------------------
#define ALIGNED_ALLOC(alignment, size) \
  aligned_alloc(alignment, size)

#define ALIGNED_REALLOC(ptr, old_size, new_size, alignment) \
    aligned_realloc(((void**)&ptr), (old_size), (new_size), alignment)

#define ALIGNED_FREE(ptr) \
    free((ptr))

#endif

// Strictly POSIX/MSYS2/Linux - Windows already has this?
static inline void* aligned_realloc(void** ptr, size_t old_size, size_t new_size, size_t alignment){
  if (!ptr) return NULL;
  

  if (new_size == 0){ free(*ptr); *ptr = NULL; return NULL; }

  if (!(*ptr)){
    *ptr = aligned_alloc(alignment, new_size);
    return *ptr;
  }

  void* new_ptr = aligned_alloc(alignment, new_size);

  memcpy(new_ptr, *ptr, old_size < new_size ? old_size : new_size);
  free(*ptr);
  *ptr = new_ptr;

  return new_ptr;
}

#ifndef OFFSETOF
#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
#endif



static inline struct bodyblock_t alloc_bodyblock(void){
  struct bodyblock_t block = {0}; 
  // 9=  number of floating-point pointers in the block - e.g x,y,z ,vx,vy etc
  size_t total_size = (sizeof(Float) * PHYS_BODYBLOCK_SIZE * 9) + sizeof(collider_t) * PHYS_BODYBLOCK_SIZE;
  block.base = ALIGNED_ALLOC(total_size, cache_line);

  size_t offset = PHYS_BODYBLOCK_SIZE;
  if (!block.base){ block.base = NULL; return block; }
  memset(block.base, 0, total_size);

  block.x = block.base;
  block.y = (Float*)block.base + offset;
  block.z = (Float*)block.base + offset * 2;

  block.vx = (Float*)block.base + offset * 3;
  block.vy = (Float*)block.base + offset * 4;
  block.vz = (Float*)block.base + offset * 5;

  block.fx = (Float*)block.base + offset * 6;
  block.fy = (Float*)block.base + offset * 7;
  block.fz = (Float*)block.base + offset * 8;
  block.colliders = (collider_t*)block.base + sizeof(Float) * PHYS_BODYBLOCK_SIZE * 9;

  return block;
}

static inline void free_bodyblock(struct bodyblock_t* block){
  if (block->base) ALIGNED_FREE(block->base); 
  memset(block, 0, sizeof(struct bodyblock_t));
}

static inline uint8_t block_full(struct bodyblock_t block){
  return block.count >= PHYS_BODYBLOCK_SIZE;
}

struct physworld_t _world = {0};

static struct bodyblock_t* _new_block(void){
  if (_world.numblocks >= _world.capblocks){
    size_t old_size = sizeof(struct bodyblock_t) * _world.numblocks;
    size_t new_cap = _world.capblocks * 2;
    size_t new_size = sizeof(struct bodyblock_t) * new_cap;

    struct bodyblock_t* block_arr = ALIGNED_REALLOC(
        _world.blocks,
        old_size,
        new_size,
        cache_line
        );
    if (!block_arr) return NULL;
    _world.blocks = block_arr;
    _world.capblocks = new_cap;
  }
  _world.blocks[_world.numblocks] = alloc_bodyblock();
  return &_world.blocks[_world.numblocks++];
}

static struct bodyblock_t* get_block(void){
  if (!_world.numblocks || block_full(_world.blocks[_world.numblocks - 1])){
    return _new_block();
  }
  return &_world.blocks[_world.numblocks - 1];
}


static inline void set_pos(struct bodyblock_t* block, const uint32_t id_in_block, vec3_t pos){
  block->x[id_in_block] = pos[0];
  block->y[id_in_block] = pos[1];
  block->z[id_in_block] = pos[2];
}

static inline void set_vel(struct bodyblock_t* block, const uint32_t id_in_block, vec3_t vel){
  block->vx[id_in_block] = vel[0];
  block->vy[id_in_block] = vel[1];
  block->vz[id_in_block] = vel[2];
}


static inline void set_force(struct bodyblock_t* block, const uint32_t id_in_block, vec3_t force){
  block->fx[id_in_block] = force[0];
  block->fy[id_in_block] = force[1];
  block->fz[id_in_block] = force[2];
}


bodyid_t _phys_create_bodybox(vec3_t origin, vec3_t half_extents){
  /*
  if (block_full(_world.blocks[_world.numblocks]) || !_world.numblocks){
    _world.blocks[_world.numblocks++] = alloc_bodyblock(); 

  }
  struct bodyblock_t* block = &_world.blocks[_world.numblocks];
  */
  struct bodyblock_t* block = get_block();
  bodyid_t id = block->count;

  collider_t box_collider = {0};
  box_collider.type = COL_BOX;
  VectorCopy(half_extents, box_collider.data.box.extents);

  set_pos(block, id, origin);
  set_vel(block, id, VEC_ZERO);
  set_force(block, id, VEC_ZERO);
  block->colliders[id] = box_collider;
  return block->count++;
}

bodyid_t _phys_create_bodysphere(vec3_t origin, Float radius){
  struct bodyblock_t* block = get_block();
  bodyid_t id = block->count;

  collider_t sphere_collider = {0};
  sphere_collider.type = COL_SPHERE;
  sphere_collider.data.sphere.radius = radius;
  
  set_pos(block, id, origin);
  set_vel(block, id, VEC_ZERO);
  set_force(block, id, VEC_ZERO);
  block->colliders[id] = sphere_collider;
  return block->count++;
}






void _phys_init_world(void){
  _world.numblocks = 0;
  _world.capblocks = 2;
  size_t c = sizeof(struct bodyblock_t) * _world.capblocks; // Start with 2 blocks
  _world.blocks = ALIGNED_ALLOC(cache_line, c);
  _world.numbodies = 0;
  _world.capbodies = 1024;
  
}

void _phys_shutdown(void){
  for (uint32_t i = 0; i < _world.numblocks; i++){
    free_bodyblock(&_world.blocks[i]);
  }
}


