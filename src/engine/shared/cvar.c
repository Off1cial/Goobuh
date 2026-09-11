#include "engine/shared/cvar.h"
#include "engine/shared/hash.h"
#include "common/common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define HASH_BUCKET_COUNT 256

cvar_t cvar_indexes[CVAR_MAXCVARS];
int cvar_numIndexes;

static cvar_t *cvarHashTable[HASH_BUCKET_COUNT];

// cvar.c — the real storage for these

static cvar_t *Cvar_Find(const char *name)
{
  uint32_t bucket = Hash_Bucket(Hash_String(name), HASH_BUCKET_COUNT);

  for (cvar_t *var = cvarHashTable[bucket]; var; var = var->hashNext)
  {
    if (!Q_stricmp(var->name, name))
      return var;
  }
  return NULL;
}

static cvar_t *Cvar_RegisterInternal(const char *name, const char *defaultValue, uint64_t flags)
{
  cvar_t *existing = Cvar_Find(name);
  if (existing)
    return existing;

  if (cvar_numIndexes >= CVAR_MAXCVARS)
  {
    printf("[CVAR]: cvar_indexes full, cannot register '%s'\n", name);
    return NULL;
  }

  cvar_t *cv = &cvar_indexes[cvar_numIndexes++];
  memset(cv, 0, sizeof(cvar_t));

  size_t namelen = strlen(name);
  if (namelen >= sizeof(cv->name))
    namelen = sizeof(cv->name) - 1;
  memcpy(cv->name, name, namelen);
  cv->name[namelen] = '\0';

  size_t vallen = strlen(defaultValue);
  if (vallen >= sizeof(cv->string))
    vallen = sizeof(cv->string) - 1;
  memcpy(cv->string, defaultValue, vallen);
  cv->string[vallen] = '\0';

  cv->value = (float)atof(cv->string);
  cv->value_int = (int)atoi(cv->string);
  cv->flags = flags;

  uint32_t bucket = Hash_Bucket(Hash_String(name), HASH_BUCKET_COUNT);
  cv->hashNext = cvarHashTable[bucket];
  cvarHashTable[bucket] = cv;

  return cv;
}

void Cvar_Register(const char *name, const char *defaultValue, uint64_t flags)
{
  Cvar_RegisterInternal(name, defaultValue, flags);
}

// New: for cvars that also need direct extern-float access
void Cvar_RegisterLinked(const char *name, const char *defaultValue, uint64_t flags, float *linked)
{
  cvar_t *cv = Cvar_RegisterInternal(name, defaultValue, flags);
  if (!cv)
    return;
  cv->linked = linked;
  if (linked)
    *linked = cv->value; // sync immediately on registration
}

/*
void Cvar_Register(const char* name, const char* defaultValue, uint64_t flags)
{
    if (Cvar_Find(name))
        return;

    if (cvar_numIndexes >= CVAR_MAXCVARS)
    {
        printf("[CVAR]: cvar_indexes full, cannot register '%s'\n", name);
        return;
    }

    cvar_t* cv = &cvar_indexes[cvar_numIndexes++];
    memset(cv, 0, sizeof(cvar_t));

    // Q_strncpy(dst, src, strlen(src)) never null-terminates unless dst
    // happens to already be zeroed AND src is shorter than dst's capacity.
    // Size off the DESTINATION buffer, and terminate explicitly.
    size_t namelen = strlen(name);
    if (namelen >= sizeof(cv->name)) namelen = sizeof(cv->name) - 1;
    memcpy(cv->name, name, namelen);
    cv->name[namelen] = '\0';

    size_t vallen = strlen(defaultValue);
    if (vallen >= sizeof(cv->string)) vallen = sizeof(cv->string) - 1;
    memcpy(cv->string, defaultValue, vallen);
    cv->string[vallen] = '\0';

    cv->value = atof(cv->string);
    cv->value_int = atoi(cv->string);
    cv->flags = flags;

    uint32_t bucket = Hash_Bucket(Hash_String(name), HASH_BUCKET_COUNT);
    cv->hashNext = cvarHashTable[bucket];
    cvarHashTable[bucket] = cv;
}
*/

const char *Cvar_ValueString(const char *name)
{
  cvar_t *cv = Cvar_Find(name);
  if (!cv)
    return "";
  return cv->string;
}

float Cvar_ValueFloat(const char *name)
{
  cvar_t *cv = Cvar_Find(name);
  return cv ? cv->value : 0.0f;
}

int Cvar_ValueInteger(const char *name)
{
  cvar_t *cv = Cvar_Find(name);
  return cv ? cv->value_int : 0;
}

void Cvar_SetFloat(const char *name, float value)
{
  cvar_t *cv = Cvar_Find(name);
  if (!cv)
    return;

  if (cv->flags & CVAR_READONLY)
  {
    printf("[CVAR]: '%s' is read-only\n", name);
    return;
  }

  cv->value = value;
  cv->value_int = (int)value;
  snprintf(cv->string, sizeof(cv->string), "%g", value);

  if (cv->linked)
    *cv->linked = value;
}

/*
void Cvar_InitAll()
{
    Cvar_Register("sensitivity", "1.0", CVAR_ARCHIVE | CVAR_CLIENT);

    // Network
    Cvar_Register("cl_updaterate", "20", CVAR_ARCHIVE | CVAR_CLIENT);
    Cvar_Register("cl_interp", "0.1", CVAR_ARCHIVE | CVAR_CLIENT);
}

*/

void Cvar_InitAll(void)
{
  /*
  Cvar_RegisterLinked("sensitivity", "1.0", CVAR_ARCHIVE | CVAR_CLIENT, &cl_sensitivity);

  Cvar_RegisterLinked("sv_accelerate", "10", CVAR_SERVER, &sv_accelerate);
  Cvar_RegisterLinked("sv_airaccelerate", "10", CVAR_SERVER, &sv_airaccelerate);
  Cvar_RegisterLinked("sv_stopspeed", "100", CVAR_SERVER, &sv_stopspeed);
  Cvar_RegisterLinked("sv_maxspeed", "320", CVAR_SERVER, &sv_maxspeed);
  Cvar_RegisterLinked("sv_friction", "2.0", CVAR_SERVER, &sv_friction);
  Cvar_RegisterLinked("sv_gravity", "600", CVAR_SERVER, &sv_gravity);

  Cvar_Register("cl_updaterate", "20", CVAR_ARCHIVE | CVAR_CLIENT);
  Cvar_Register("cl_interp", "0.1", CVAR_ARCHIVE | CVAR_CLIENT);
  */
}

