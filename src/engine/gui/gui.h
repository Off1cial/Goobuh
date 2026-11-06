#ifndef GUI_H
#define GUI_H

#include "common/common.h"

typedef struct uivertex_t
{
    float pos[2];
    float uv[2];
    byte col[4];
} uivertex_t;


bool GUI_Init( void );

void GUI_FrameStart( void );
void GUI_FrameEnd( void );

void GUI_DrawRect (
        float x, float y,
        float w, float h,
        byte r, byte g, byte b, byte a);


#endif
