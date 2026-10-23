#version 330 core

layout( location = 0) in vec3 aPos;
layout( location = 1) in vec3 aNormal;
layout( location = 2) in vec2 aUV;
layout( location = 3) in vec4 aCol;

out vec3 vPos;
out vec3 vNormal;
out vec2 vUV;
out vec4 vCol;


layout( std140 ) uniform uMatrices
{
    mat4 view;
    mat4 projection;
};

uniform mat4 uModel;

void main( void )
{
    gl_Position = projection * view * uModel * vec4(aPos, 1.0 );
    vCol = aCol;

    //gl_Position = vec4(aPos, 1.0);
}
