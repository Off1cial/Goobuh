#include "input.hpp"
#include "camera.h"

#include <SDL3/SDL.h>

void CInput::Poll( void )
{
    memcpy(m_keysPrevious, m_keysCurrent, sizeof(uint8_t) * KEY_COUNT);
    const bool* keys = SDL_GetKeyboardState(nullptr);

    if (keys) memcpy(m_keysCurrent, keys, KEY_COUNT );

    m_mousePrevious = m_mouseCurrent;

    m_mouseCurrent = (m_mouselocked)
    ? SDL_GetRelativeMouseState( &m_mxrel, &m_myrel )
    : SDL_GetMouseState( &m_mx, &m_my );
}

void CInput::Init( void )
{
    memset(m_keysCurrent, 0, KEY_COUNT );
    memset(m_keysPrevious, 0, KEY_COUNT );
    
}


void CInput::GetMousePosition( float* x, float* y ) const
{
    *x = m_mx; *y = m_my;
}



void CInput::CameraLook( void )
{
    camera_look( m_camera, m_mxrel, m_myrel, m_camerasens );
}
void CInput::MoveCamera( vec3_t dir, float scale )
{
    VectorMA(m_camera->origin, scale, dir, m_camera->origin);
}

