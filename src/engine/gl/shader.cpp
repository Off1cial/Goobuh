#include "core/defs.h"
#include "core/error.h"
#include <engine/gl/gldefs.h>

GLint GL::Shader::CompileSource(ShaderStage stage, const char *path)
{
    GLint handle = -1;  
    char realpath[256];
    snprintf (realpath, 256, "%s%s", RESOURCE_DIR, path);
    
    FILE *fstream = fopen (realpath, "rb");
    if (!fstream)
    {
        Error (ERROR_SHADERCOMPILE "Failed to compile shader source: %s\n", path);
        return -1;
    }
    

    fclose (fstream);
    return handle;
}
