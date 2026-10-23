#include "engine/client/cl_input.h"

#include <SDL3/SDL_events.h>
#include "engine/system/window.h"
#include "common/common.h"

#include "engine/client/client.h"
#include "math/vector.h"

bool in_keydown[KEY_COUNT];
int in_keyrepeat[KEY_COUNT];

char* in_keybinds[KEY_COUNT];

float in_mousedx, in_mousedy;

void KeyEvent( int code, int down, int repeat )
{
    in_keydown[code] = down;
    in_keyrepeat[code] = (repeat) ? in_keyrepeat[code] + repeat : 0;
    Con_printf("[key] code=%d, down=%d, repeat=%d\n", code, down, repeat );
}


void IN_MouseEvent( void )
{
    cl.viewangles[PITCH] -= in_mousedy * 0.002f;
    cl.viewangles[YAW] -= in_mousedx * 0.002f;

    cl.viewangles[PITCH] = 
      (cl.viewangles[PITCH] > 89.0f * RAD ) ? RAD * 89.0f : cl.viewangles[PITCH];
    cl.viewangles[PITCH] = 
      (cl.viewangles[PITCH] < RAD * -89.0f ) ? RAD * -89.0f : cl.viewangles[PITCH];
    
    

    in_mousedx = 0;
    in_mousedy = 0;
}

void IN_Update( void )
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
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                Con_printf("Click: %d\n", event.button.button  );
                KeyEvent( 
                        event.button.button + SDL_SCANCODE_COUNT - 1, 
                        event.button.down, 
                        event.button.clicks);
                break;
            }
            case SDL_EVENT_MOUSE_MOTION:
            {
                in_mousedx += event.motion.xrel;
                in_mousedy += event.motion.yrel;
                break;
            }
            case SDL_EVENT_WINDOW_RESIZED:
            {
                Sys_WindowResize();        
                break;
            }
            case SDL_EVENT_QUIT:
                //CL_Shutdown();
                //Sys_Exit();
                gSys_window->should_close = true;
                break;
        }
    }
    IN_MouseEvent();
}
