#version 330 core

layout( location = 0) in vec3 aPos;
layout( location = 1) in vec3 aNormal;
layout( location = 2) in vec2 aUV;
layout( location = 3) in vec4 aCol;

out vec3 vPos;
out vec3 vNormal;
out vec2 vUV;
out vec4 vCol;

#include "shared/light.glsl"

layout( std140 ) uniform uUniformBuff
{
    mat4    view;                  // 0
    mat4    projection;            // 64
    int     lightCount;            // 128
    light_t DLights[MAX_DLIGHTS];  // 144 (std140 pads the gap automatically)
};

uniform mat4 uModel;



void main( void )
{
    vec4 worldPos = uModel * vec4(aPos, 1.0);

    gl_Position = projection * view * worldPos;
    vPos = worldPos.xyz;
    vNormal = normalize(mat3(uModel) * aNormal);
    vCol = vec4( normalize(aNormal) * 0.5 + 0.1, 1.0 );
    vUV = aUV;
}
