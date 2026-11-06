#include "engine/gui/gui.h"
#include "engine/rendering/gl.h"

#include "engine/rendering/renderer.h"
#include "engine/system/window.h"
    

GLuint gui_vao;
glbuff_t gui_vbo;
glbuff_t gui_ebo;
glbuff_t gui_instance_vbo;
GLsizeiptr gui_vbo_bytes;
unsigned int gui_ebo_offset;

shader_t *gui_shader;
gluniform_t uScreensize;

#define GUI_MAX_INSTANCES 4096

typedef struct instance_t
{
    float pos[2];
    float size[2];
    byte col[4];
} instance_t;

static instance_t gui_instances[GUI_MAX_INSTANCES];
static unsigned int gui_numinstances = 0;


static uivertex_t gRect[4] =
{
    {.pos = { -0.5, -0.5  }, .uv = {0, 1}, .col = {255,255,255,255}},
    {.pos = {  0.5, -0.5  }, .uv = {1, 1}, .col = {255,255,255,255}},
    {.pos = {  0.5,  0.5  }, .uv = {1, 0}, .col = {255,255,255,255}},
    {.pos = { -0.5,  0.5  }, .uv = {0, 0}, .col = {255,255,255,255}},
};

static unsigned int gRectInds[6] = { 0, 1, 2, 2, 3, 0 };

static inline void update_screensize( void )
{

    uScreensize.Vector2.xy[0] = WIN_WIDTH;
    uScreensize.Vector2.xy[1] = WIN_HEIGHT;
}

bool GUI_Init( void )
{
    GL_CreateVAO (&gui_vao);
    // Reuse the static size for the dynamic buffer
    GL_CreateBuffer (&gui_vbo, GLBUFF_VBO, STATIC_VBO_SIZE, NULL, GLUSAGE_DYNAMIC);
    glBindVertexArray (gui_vao);
    glBindBuffer (GL_ARRAY_BUFFER, gui_vbo.handle);
    glEnableVertexAttribArray (0);
    glEnableVertexAttribArray (1);
    glEnableVertexAttribArray (2);
    glVertexAttribPointer( 
            0, 2, 
            GL_FLOAT, GL_FALSE, 
            sizeof(uivertex_t), (void*)OFFSETOF(uivertex_t, pos));

    glVertexAttribPointer( 
            1, 2, 
            GL_FLOAT, GL_FALSE, 
            sizeof(uivertex_t), (void*)OFFSETOF(uivertex_t, uv));

    glVertexAttribPointer( 
            2, 4, 
            GL_UNSIGNED_BYTE, GL_TRUE, 
            sizeof(uivertex_t), (void*)OFFSETOF(uivertex_t, col));

    // Create the GUI EBO
    GL_CreateBuffer ( &gui_ebo, GLBUFF_EBO, STATIC_EBO_SIZE, NULL, GLUSAGE_DYNAMIC);
    glBindVertexArray (gui_vao);
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, gui_ebo.handle);
    
    // Instancing
    GL_CreateBuffer (&gui_instance_vbo, GLBUFF_VBO, STATIC_VBO_SIZE, NULL, GLUSAGE_DYNAMIC);
    glBindVertexArray (gui_vao);
    glBindBuffer (GL_ARRAY_BUFFER, gui_instance_vbo.handle);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer( 
            3, 2, 
            GL_FLOAT, GL_FALSE, 
            sizeof(instance_t), (void*)OFFSETOF(instance_t, pos));

    glVertexAttribDivisor(3, 1);

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(
            4, 2,
            GL_FLOAT, GL_FALSE,
            sizeof(instance_t), (void*)OFFSETOF(instance_t, size));

    glVertexAttribDivisor(4, 1);

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(
            5, 4,
            GL_UNSIGNED_BYTE, GL_TRUE,
            sizeof(instance_t), (void*)OFFSETOF(instance_t, col));
    
    glVertexAttribDivisor(5, 1);

    glBindBuffer(GL_ARRAY_BUFFER, gui_vbo.handle);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(gRect), gRect);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gui_ebo.handle);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(gRectInds), gRectInds);


    // GUI shader
    gui_shader = R_CreateShader("shaders/uidefault.vs", "shaders/uidefault.fs");
    if (!gui_shader) return false;
    // Find the screensize uniform
    bool found = GL_GetUniform (gui_shader, "uScreensize", GLUNIFORM_VEC2, &uScreensize);
    if (!found) return false;
    
    update_screensize();

    return true;
}

void GUI_FrameStart( void )
{
    // Maybe move this to some form of window-resize callback
    update_screensize();
    R_UseShader(gui_shader);
    GL_UploadUniform (gui_shader, &uScreensize);
}



void GUI_FrameEnd( void )
{
   if (!gui_numinstances)
       return;
    
   glDisable(GL_CULL_FACE);
   glBindVertexArray(gui_vao);
   glBindBuffer(GL_ARRAY_BUFFER, gui_instance_vbo.handle);

   glBufferSubData(
           GL_ARRAY_BUFFER,
           0,
           gui_numinstances * sizeof(instance_t),
           gui_instances
           );
   glDrawElementsInstanced(
        GL_TRIANGLES,
        6,
        GL_UNSIGNED_INT,
        0,
        gui_numinstances
        );

   glEnable(GL_CULL_FACE);
   gui_numinstances = 0;
}



void GUI_DrawRect (
        float x, float y,
        float w, float h,
        byte r, byte g, byte b, byte a
        )
{
    if (gui_numinstances >= GUI_MAX_INSTANCES)
        return;

    instance_t* inst = &gui_instances[gui_numinstances++];
    inst->pos[0] = x + w * 0.5f;
    inst->pos[1] = y + h * 0.5f;


    inst->size[0] = w;
    inst->size[1] = h;

    inst->col[0] = r;
    inst->col[1] = g;
    inst->col[2] = b;
    inst->col[3] = a;
}

