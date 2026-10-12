#ifndef SYS_ALLOCATOR_H
#define SYS_ALLOCATOR_H

#include <stdint.h>
#include <stdbool.h>

// Problem: Systems can call malloc/calloc and leave the memory unmanaged.

// Proposed solution: A general allocator which provides arenas that are allocated once, and have lifetimes.

// Defined in common.h

void Sys_ArenaShutdownAll( void );


#endif