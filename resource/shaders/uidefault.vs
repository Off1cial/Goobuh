#version 330 core

#extension GL_ARB_bindless_texture : require

layout (location = 0 ) in vec2 aPos;
layout (location = 1 ) in vec2 aUV;
layout (location = 2 ) in vec4 aCol;


layout (location = 3 ) in vec2 instPos;
layout (location = 4 ) in vec2 instSize;
layout (location = 5 ) in vec4 instCol;

uniform vec2 uScreensize;


out vec2 vPos;
out vec2 vUV;
out vec4 vCol;


void main()
{
    vec2 pos = aPos * instSize + instPos;

    pos.x = (pos.x / uScreensize.x) * 2.0 - 1.0;
    pos.y = 1.0 - (pos.y / uScreensize.y) * 2.0;

    gl_Position = vec4(pos, 0.0, 1.0);

    vUV = aUV;
    vCol = aCol * instCol;
}
