#include "engine/shared/cvar.h"
#include "common/hash.h"
#include "common/common.h"
#include <stdlib.h>
#include <stdio.h>

#define CVAR_BUCKETS 64

cvar_t* cvar_buckets[CVAR_BUCKETS];

void add_to_bucket( cvar_t* cvar, int bucket )
{
    cvar_t **link = &cvar_buckets[bucket];
    while (*link)
        link = &(*link)->next;

    *link = cvar;
}


void Cvar_Register( cvar_t *cvar )
{
    if (!cvar->name)
    {
        Con_printf("Cant register cvar with no name!\n");
        return;
    }
    if (!cvar->string)
    {
        Con_printf("Cant register cvar with no string!\n");
        return;
    }


    if (Cvar_Find( cvar->name ))
    {
        Con_printf("Cant register cvar '%s', already exits\n", cvar->name );
        return;
    }
    long len = strlen( cvar->string );
    char *new_string = malloc(len + 1);
    memcpy(new_string, cvar->string, len );
    new_string[len] = 0;
    cvar->string = new_string;

    int bucket = Hash_String( cvar->name ) % CVAR_BUCKETS;
    add_to_bucket( cvar, bucket );
}

cvar_t *Cvar_Find( char* name )
{
    int bucket = Hash_String( name ) % CVAR_BUCKETS;
    
    cvar_t *first;

    for (first = cvar_buckets[bucket]; first; first = first->next )
    {
        if (Q_stricmp(first->name, name) == 0)
        {
            return first;
        }
    }
    return NULL;
}


float Cvar_ValueNumeric( char *name )
{
    cvar_t* variable = Cvar_Find( name );
    if (!variable) return 0;

    return variable->value;
}

char* Cvar_ValueString( char *name )
{
    cvar_t* variable = Cvar_Find( name );
    if (!variable) return NULL;
    return variable->string;
}


void Cvar_Set( char* name, char* value )
{
    if (!name || !value) return;
    cvar_t *var = Cvar_Find( name );
    if (!var)
    {
        Con_printf("No such cvar: '%s'\n", name );
        return;
    }
    long len = strlen( value ); // What if it isnt null-terminated
                                // value isnt a literal, its from the console
    char* new_value = malloc( len + 1 );
    if (!new_value)
    {
        Sys_Crash("Cvar allocation failed - out of memory\n");
        return;
    }
    if ( var->string ) free(var->string); 
    memcpy(new_value, value, len + 1 );
    var->string = new_value;
    var->string[len] = 0;
    var->value = atof( var->string );
}

void Cvar_SetValue( char* name, float value )
{
    char val[ 32 ];
    snprintf(val, 32, "%f", value );

    Cvar_Set(name, val );
}




void Cvar_Cleanup( void )
{
    int bucket;
    cvar_t *var;

    for (bucket = 0; bucket < CVAR_BUCKETS; bucket++)
    {
        var = cvar_buckets[bucket];
        while (var)
        {
            if (var->string) free(var->string);
            memset(var, 0, sizeof(cvar_t));
            var = var->next;
        }
    }
}
