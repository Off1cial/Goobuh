#include "public/engine/input.hpp"
#include "common/common.h"

typedef float vec3_t[3];
typedef struct camera_t camera_t;

class CInput : public IInput 
{
public:
    CInput() { Init(); }
    ~CInput() = default;

    bool KeyDown( key_t key ) const override { return m_keysCurrent[key]; }
    bool KeyUp( key_t key ) const override   { return !m_keysCurrent[key]; }
    bool KeyTap( key_t key ) const override  { return m_keysCurrent[key] && !m_keysPrevious[key]; }
    bool KeyRelease( key_t key ) const override { return !m_keysCurrent[key] && m_keysPrevious[key]; }


    bool IsMouseLocked( void ) const override { return m_mouselocked; }
    void GetMousePosition( float* x, float* y ) const override;

    camera_t* GetCamera( void ) const { return m_camera; }
    void SetCamera( camera_t* camera ) { m_camera = camera; }
    void AimCamera( void );
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