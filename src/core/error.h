#ifndef __CORE_ERROR_H__
#define __CORE_ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif


#define ERROR_SHADERCOMPILE "[ShaderCompilation]: "
#define ERROR_VULKAN "[Vulkan]: "

void Error (const char* fmt, ...);


#ifdef __cplusplus
}
#endif

#endif
