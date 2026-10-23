#version 330 core

in vec3 vPos;
in vec3 vNormal;
in vec2 vUV;
in vec4 vCol;

out vec4 fragCol;

void main( void )
{
    fragCol = vCol;
}
