#include "public/engine/input.hpp"
#include "common/common.h"

typedef float vec3_t[3];
typedef struct camera_t camera_t;


#define MOUSE_BUTTON_FLAG(b) (1u << ((b)-1))

class CInput : public IInput 
{
public:
    CInput() { Init(); }
    ~CInput() = default;

    bool KeyDown( keycode_t key ) const override { return m_keysCurrent[key]; }
    bool KeyUp( keycode_t key ) const override   { return !m_keysCurrent[key]; }
    bool KeyTap( keycode_t key ) const override  { return m_keysCurrent[key] && !m_keysPrevious[key]; }
    bool KeyRelease( keycode_t key ) const override { return !m_keysCurrent[key] && m_keysPrevious[key]; }

    bool MouseDown( mbutton_t button ) const override
    {
        return (m_mouseCurrent & MOUSE_BUTTON_FLAG( button )) != 0;
    }

    bool MouseClick( mbutton_t button ) const override
    {
        return (m_mouseCurrent & MOUSE_BUTTON_FLAG( button )) != 0 &&
            (m_mousePrevious & MOUSE_BUTTON_FLAG( button )) == 0;
    }

    bool MouseRelease( mbutton_t button ) const override
    {
        return (m_mouseCurrent & MOUSE_BUTTON_FLAG( button )) == 0 &&
            (m_mousePrevious & MOUSE_BUTTON_FLAG( button )) != 0;
    }

    bool IsMouseLocked( void ) const override { return m_mouselocked; }
    void GetMousePosition( float* x, float* y ) const override;

    camera_t* GetCamera( void ) const override { return m_camera; }
    void CameraLook( void );
    void SetCamera( camera_t* camera ) override { m_camera = camera; }

    void MoveCamera( vec3_t dir, float scale ); // For debugging purposes

    void Poll( void );

private:

    void Init( void );

    uint8_t m_keysCurrent[SDL_SCANCODE_COUNT];
    uint8_t m_keysPrevious[SDL_SCANCODE_COUNT];

    SDL_MouseButtonFlags m_mouseCurrent;
    SDL_MouseButtonFlags m_mousePrevious;

    bool m_mouselocked = true;
    float m_mx = 0.0f;
    float m_my = 0.0f;
    float m_mxrel = 0.0f;
    float m_myrel = 0.0f;

    camera_t* m_camera = nullptr;
    float m_camerasens = 0.001f;
};
