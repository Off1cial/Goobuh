#include "engine/shared/cvar.h"

#include <stdlib.h>

#define CVAR( cname, varname, str, val ) cvar_t cname = {.name = varname, .string = str, .value = val, .next = NULL}

CVAR( cvar_map, "map", "", 0.0f );
