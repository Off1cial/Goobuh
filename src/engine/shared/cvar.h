#pragma once

#include <stdint.h>

#define CVAR_MAXCVARS 1024

#define CVAR_TEMP       1 // Can be changed by the user, but will reset upon restart
#define CVAR_CHEAT      2 // Can only be changed when cheats are enabled
#define CVAR_CLIENT     4 // Determines client behaviour, such as sensitivty <float>
#define CVAR_SERVER     8 // Determines server behaviour, e.g sv_cheats <int>
#define CVAR_ADMIN      16 // Requires server admin to change, such as sv_cheats <int>
#define CVAR_READONLY   32
#define CVAR_ARCHIVE    64  // Store the var in a .cfg to save



typedef struct cvar_s
{
  char       *name;
  char       *string;    // Cvar value as string
  uint64_t    flags;


  float       value;     // atof(string)
  int         value_int; // atoi(string)

  float value_min;
  float value_max;
  
  float* linked; // Linked value in memory for quick access e.g sv_airaccelerate
  struct cvar_s* hashNext;
  struct cvar_s* next;
} cvar_t;





const char* Cvar_ValueString(const char* name);
float Cvar_ValueFloat(const char* name);
int Cvar_ValueInteger(const char* name);

void Cvar_RegisterLinked(const char *name, const char *defaultValue, uint64_t flags, float *linked);

void Cvar_InitAll(void);

