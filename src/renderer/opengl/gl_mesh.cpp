#include "renderer/opengl/gl_types.h"

using namespace GL;

void MeshInit(size_t vertices, size_t indices, GL::MeshData &out)
{
  out.vertices.resize(vertices);
  out.indices.resize(indices);
  glGenBuffers(1, &out.vbo);
  glGenBuffers(1, &out.ebo);
  glGenVertexArrays(1, &out.vao);
}



void MeshUpload(GL::MeshData &data, GLenum usage)
{
  glBindVertexArray(data.vao);

    // VBO
  glBindBuffer(GL_ARRAY_BUFFER, data.vbo);
  glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(Vertex), data.vertices.data(), usage );
  // EBO
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(GLuint), data.indices.data(), usage);

  // pos
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)OFFSETOF(Vertex, pos)); 
  // normal
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)OFFSETOF(Vertex, normal));
  // colour
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)OFFSETOF(Vertex, col));
  // uv
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)OFFSETOF(Vertex, uv));


  glBindVertexArray(0);
}

void MeshDraw(MeshData& data, GLenum draw_mode)
{
  glBindVertexArray(data.vao);
  glDrawElements(draw_mode, data.indices.size(), GL_UNSIGNED_INT, data.indices.data());
  glBindVertexArray(0);
}