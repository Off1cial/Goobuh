#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout(set = 0, binding = 0) uniform sampler2D textures[];

layout(location = 0) in vec4 in_col;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec4 out_col;

layout(push_constant) uniform constants
{
  mat4 projection;
  mat4 view;
  mat4 model;
  uint64_t vertexBuffer;   // same size/alignment as the buffer reference
  uint tex_id;
} pc;

void main()
{
  vec4 texel = texture(textures[nonuniformEXT(pc.tex_id)], in_uv);
  out_col = in_col * texel;
}