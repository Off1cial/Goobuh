#version 450


layout (location = 0) in vec4 in_col;
layout (location = 1) in vec2 in_uv;

layout (location = 0) out vec4 out_col;
layout (location = 1) out vec2 out_uv;


void main()
{
  out_col = in_col;
  out_uv = in_uv; 
}