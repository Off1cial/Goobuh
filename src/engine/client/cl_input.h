#ifndef CL_INPUT_H
#define CL_INPUT_H


#include <SDL3/SDL.h>
#define KEY_COUNT SDL_SCANCODE_COUNT

extern bool in_keydown[KEY_COUNT];


void KeyUpdate( void );
void KeyEvent( int code, int down, int repeat );

void KeyBind( char* cmd, int key );

#endif