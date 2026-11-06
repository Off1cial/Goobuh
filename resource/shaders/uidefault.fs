#version 330 core


in vec2 vPos;
in vec2 vUV;
in vec4 vCol;


uniform sampler2D uAlbedo;
out vec4 fragcol;


void main()
{
    fragcol = vCol * texture(uAlbedo, vUV);
}
