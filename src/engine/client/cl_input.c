#include "engine/client/cl_input.h"

#include <stdio.h>
#include "common/common.h"

#include "engine/client/client.h"

bool in_keydown[KEY_COUNT];
int in_keyrepeat[KEY_COUNT];

char* in_keybinds[KEY_COUNT];

void KeyEvent( int code, int down, int repeat )
{
    in_keydown[code] = down;
    in_keyrepeat[code] = (repeat) * (in_keyrepeat[code] + 1);
}


void KeyUpdate( void )
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
            {
                KeyEvent(event.key.scancode, event.key.down, event.key.repeat );
                break;
            }
            case SDL_EVENT_QUIT:
                //CL_Shutdown();
                Sys_Exit();
                break;
        }
    }

}