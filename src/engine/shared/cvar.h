#ifndef CVAR_H
#define CVAR_H


typedef struct cvar_t
{
    char    *name;
    char    *string; // value
    float   value;

    struct cvar_t* next;
} cvar_t;


void Cvar_Register( cvar_t *cvar );

float Cvar_ValueNumeric( char *name );
char *Cvar_ValueString( char *name );
cvar_t* Cvar_Find( char* name );


void Cvar_Cleanup( void ); // Frees all cvars


#endif
