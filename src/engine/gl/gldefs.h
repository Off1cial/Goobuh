#ifndef __GL_DEFS_H__
#define __GL_DEFS_H__

#include <glad/glad.h>

namespace GL
{

enum class BufferType
{ VBO, EBO, UBO, SSBO };

enum class BufferUsage
{ Static, Dynamic };

struct Buffer
{
    BufferType type;
    BufferUsage usage;
    GLuint handle;
    GLuint binding;
    GLsizeiptr size;
};

enum class ShaderStage
{ Vertex, Geometry, Fragment, Compute };

class Shader
{
public:
    Shader (ShaderStage stage, const char *path);
    ~Shader();

    
        
    GLuint Handle() const;
private:
    
    GLint CompileSource (ShaderStage stage, const char *path);

    ShaderStage m_stage;
    GLuint m_handle;
};

class Program
{
public:
    Program();
    ~Program();

    void Attach (const Shader &shader);
    void Link();

    void Bind() const;
private:
    GLuint m_handle;
};

inline void Program::Attach (const Shader &shader)
{
    glAttachShader (m_handle, shader.Handle());
}

inline void Program::Link()
{
    glLinkProgram (m_handle);
}

inline void Program::Bind() const
{
    glUseProgram (m_handle);
}

};

#endif
