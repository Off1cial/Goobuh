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
uniform vec3 uViewPos;

void main( void )
{
    vec4 worldPos = uModel * vec4(aPos, 1.0);

    gl_Position = projection * view * worldPos;

    float view_dist = length(uViewPos - worldPos.xyz);

    vNormal = normalize(mat3(uModel) * aNormal);
    vCol    = 100 * vec4(abs(vNormal), 1.0) / view_dist;
}