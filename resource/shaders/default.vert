#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec4 out_col;
layout (location = 1) out vec2 out_uv;

struct Vertex {

	vec3 position;
	vec3 normal;
	vec4 col;
  vec2 uv;
}; 

layout(buffer_reference, std430) readonly buffer VertexBuffer{ 
	Vertex vertices[];
};

//push constants block
layout( push_constant ) uniform constants
{	
  mat4 projection;
	mat4 view;
  mat4 model;
	VertexBuffer vertexBuffer;
} PushConstants;

void main() 
{	
	//load vertex data from device adress
	Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

	//output data
	gl_Position = PushConstants.projection * PushConstants.view * PushConstants.model * vec4(v.position, 1.0f);
	out_col = v.col;
  out_uv.x = v.uv.x;
	out_uv.y = v.uv.y;
}
