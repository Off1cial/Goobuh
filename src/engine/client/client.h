#ifndef CLIENT_H
#define CLIENT_H


#include "math/vector.h"

#define CL_ARENA_SIZE 1024 * 1024

bool CL_Init( void );
void CL_Shutdown( void );
void CL_Frame( void );
void CL_Think( void );

typedef struct clientstate_t
{
    vec3_t viewangles;
} clientstate_t;

extern clientstate_t cl;


// Communication

#endif
