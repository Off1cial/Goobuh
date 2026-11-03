#version 330 core


#include "shared/light.glsl"

in vec3 vPos;
in vec3 vNormal;
in vec2 vUV;
in vec4 vCol;

out vec4 fragCol;


layout( std140 ) uniform uUniformBuff
{
    mat4    view;                  // 0
    mat4    projection;            // 64
    int     lightCount;            // 128
    light_t DLights[MAX_DLIGHTS];  // 144 (std140 pads the gap automatically)
};
vec3 light_color( int i )
{
    uint c = DLights[i].col;
    return vec3( float( c        & 0xFFu),
                 float((c >> 8)  & 0xFFu),
                 float((c >> 16) & 0xFFu)) / 255.0;
}

vec3 calc_light_env( int i, vec3 N )
{
    vec3  L     = -normalize( DLights[i].direction );
    float ndotl = max( dot( N, L ), 0.0 );
    return light_color( i ) * DLights[i].intensity * ndotl;
}

vec3 calc_light_point( int i, vec3 N )
{
    vec3  toLight = DLights[i].position - vPos;
    float dist    = length( toLight );
    vec3  L       = toLight / max( dist, 1e-4 );

    // Inverse-square, windowed so it reaches zero at radius
    float r      = max( DLights[i].radius, 1e-4 );
    float window = clamp( 1.0 - pow( dist / r, 4.0 ), 0.0, 1.0 );
    float atten  = window * window / ( dist * dist + 1.0 );

    float ndotl = max( dot( N, L ), 0.0 );
    return light_color( i ) * DLights[i].intensity * ndotl * atten;
}

vec3 calc_light_spot( int i, vec3 N )
{
    vec3  toLight = DLights[i].position - vPos;
    float dist    = length( toLight );
    vec3  L       = toLight / max( dist, 1e-4 );

    // Angle between the spot axis and the direction from light to fragment
    float theta = dot( -L, normalize( DLights[i].direction ) );
    float cut   = DLights[i].cutoff;

    // Soft edge over a small band inside the cone
    float cone  = smoothstep( cut, cut + 0.05, theta );

    float ndotl = max( dot( N, L ), 0.0 );
    float atten = 1.0 / ( dist * dist + 1.0 );
    return light_color( i ) * DLights[i].intensity * ndotl * atten * cone;
}

vec3 CalculateLighting( void )
{
    vec3 N     = normalize( vNormal );
    vec3 total = vec3( 0.0 );

    int count = min( lightCount, MAX_DLIGHTS );
    for ( int i = 0; i < count; i++ )
    {
        switch ( DLights[i].type )
        {
        case LIGHT_SPOT:  total += calc_light_spot ( i, N ); break;
        case LIGHT_ENV:   total += calc_light_env  ( i, N ); break;
        case LIGHT_POINT: total += calc_light_point( i, N ); break;
        default: break;
        }
    }
    return total;
}


void main( void )
{
    vec3 lit = CalculateLighting() + 0.05f;
    fragCol  = vec4( vCol.rgb * lit, vCol.a );

}
 