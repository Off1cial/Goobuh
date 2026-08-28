#pragma once

#include <glad/glad.h>
#include <vector>
#include "renderer/interface/r_types.hpp"

namespace GL
{
  struct MeshData
  {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    GLuint vao, vbo, ebo;
  };

  struct Shader
  {
    GLuint program;
  };

  class GLBuffer
  {
  public:
    GLuint handle = 0;
    GLsizeiptr size = 0;
    GLenum target = 0;

    GLBuffer(GLsizeiptr size, GLenum target)
        : size(size), target(target)
    {
      glGenBuffers(1, &handle);
    }

    void Bind() const;
    void SetData(void* data);

    ~GLBuffer()
    {
      if (handle)
        glDeleteBuffers(1, &handle);
    }
  };

  void MeshInit(size_t vertices, size_t indices, MeshData &out);
  void MeshUpload(MeshData &data, GLenum usage);
  void MeshDraw(MeshData& data, GLenum draw_mode);

};
