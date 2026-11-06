#include "common/common.h"
#include "common/hash.h"
#include "engine/rendering/renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <limits.h>

#define FOURCC_DXT1 0x31545844 // "DXT1"
#define FOURCC_DXT3 0x33545844 // "DXT3"
#define FOURCC_DXT5 0x35545844 // "DXT5"

static char* resourcepath = ASSET_DIR "textures/";

static void R_RegisterImage( image_t* image );

static unsigned int DDS_ReadU32(const byte *data)
{
    return (unsigned int)data[0]
         | ((unsigned int)data[1] << 8)
         | ((unsigned int)data[2] << 16)
         | ((unsigned int)data[3] << 24);
}

static size_t DDS_MipSize(int width, int height, int blocksize)
{
    size_t blocks_x = (width + 3) / 4;
    size_t blocks_y = (height + 3) / 4;

    if (blocks_x < 1) blocks_x = 1;
    if (blocks_y < 1) blocks_y = 1;

    return blocks_x * blocks_y * blocksize;
}



image_t *R_LoadDDS( const char* filename )
{
    // Build path
    char filepath[256];
    snprintf (filepath, 256, "%s%s", resourcepath, filename);

    FILE *fptr = fopen (filepath, "rb");
    if (!fptr)
    {
        Con_printf ("Failed to find image texture: %s\n", filename);
        return NULL;
    }

    // Check format
    char code[4];
    if ( (fread (code, 1, 4, fptr) != 4) 
        || code[0] != 'D' 
        || code[1] != 'D'
        || code[2] != 'S'
        || code[3] != ' ')
    {
        fclose (fptr);
        Con_printf ("Image '%s' from '%s' is not a valid .dds file\n", filename, filepath);
        return NULL;
    }
    // Read .dds header
    byte header[124];
    if (fread (header, 1, 124, fptr) != 124)
    {
        fclose(fptr);
        Con_printf ("Image '%s' from '%s' is not a valid .dds file\n", filename, filepath);
        return NULL;
    }


    if (DDS_ReadU32(&header[0]) != 124)
    {
        Con_printf("LoadDDS: Invalid DDS header size\n");
        fclose(fptr);
        return NULL;
    }



    unsigned int height  = DDS_ReadU32(&header[8]);
    unsigned int width   = DDS_ReadU32(&header[12]);
    unsigned int mipmaps = DDS_ReadU32(&header[24]);
    unsigned int fourcc  = DDS_ReadU32(&header[80]);


    if (!width || !height || width > INT_MAX || height > INT_MAX)
    {
        Con_printf("LoadDDS: Invalid DDS dimensions: %s\n", filename);
        fclose(fptr);
        return NULL;
    }

    GLenum format;
    int blocksize;
    // Find compression format
    switch (fourcc)
    {
        case FOURCC_DXT1:
        {
            format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            blocksize = 8;
            break;
        }
        case FOURCC_DXT3:
        {
            format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            blocksize = 16;
            break;
        }
        case FOURCC_DXT5:
        {
            format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            blocksize = 16;
            break;
        }
        default:
        {
            Con_printf("Unsupported DDS compression format found in '%s'\n", filename);
            fclose (fptr);
            return NULL;
        }
    }

    if (!mipmaps)
        mipmaps = 1;
    
    unsigned int maxmips = 1;
    unsigned int maxdimension = MAX(width, height);

    while (maxdimension > 1 ){ maxdimension >>= 1; maxmips++;}

    if (mipmaps > maxmips)
    {
        Con_printf("LoadDDS: Invalid mipmap count\n" );
        fclose(fptr);
        return NULL;
    }


    long bufsize = DDS_MipSize(width, height, blocksize);
    byte *data = malloc(bufsize);

    if (!data)
    {
        Con_printf("Failed to allocate %zlB to texture\n", bufsize);
        fclose(fptr);
        return NULL;
    }

    GLuint texnum;
    glGenTextures(1, &texnum);
    glBindTexture(GL_TEXTURE_2D, texnum);

    int mipwidth = width, mipheight = height;
    
    for (unsigned int i = 0; i < mipmaps; i++)
    {
        size_t size = DDS_MipSize(mipwidth, mipheight, blocksize);

        if (fread(data, 1, size, fptr) != size)
        {
            Con_printf("LoadDDS: Incomplete DDS mipmap data\n", filename);
            free(data);
            fclose(fptr);
            glDeleteTextures(1, &texnum);
            return NULL;
        }

        glCompressedTexImage2D(
                GL_TEXTURE_2D,
                i,
                format,
                mipwidth, mipheight,
                0, (GLsizei)size,
                data
                );
        
        mipwidth >>= 1;
        mipheight >>= 1;
        if (mipwidth < 1) mipwidth = 1;
        if (mipheight < 1) mipheight = 1;

    }
    
    free(data);
    fclose(fptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            mipmaps > 1 ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmaps - 1);

    // Temporary copy incase of bindless failure
    image_t* image = calloc(1, sizeof(image_t));

    if (!image)
    {
        glDeleteTextures(1,  &texnum);
        Sys_Crash("LoadDDS: Failed to allocate memory to image\n");
        return NULL;
    }



    snprintf(image->name, sizeof(image->name), "%s", filename);
    for ( char* c = image->name; c; c++ )
    {
        if (*c == '.')
        {
            *c = '\0';
            break;
        }
    }

    image->width = width;
    image->height = height;
    image->glnum = texnum;



    // Bindless OpenGL
    image->bindless = glGetTextureHandleARB(texnum);
GLenum err = glGetError();

printf("GetTextureHandle error: 0x%x\n", err);
    if (image->bindless == 0)
    {
        Con_printf("LoadDDS: Failed to acquire bindless texture handle\n");
        glDeleteTextures(1, &texnum);
        free(image);
        return NULL;
        
    }

    glMakeTextureHandleResidentARB(image->bindless);
    
    // Make Hunk copy
    image_t *cpy = Hunk_Alloc(sizeof(image_t));
    memcpy (cpy, image, sizeof(image_t));
    free(image);
    
    Con_printf("Successfully loaded image \'%s\'\n", filename );

    R_RegisterImage( cpy);
    return cpy;
};


void R_FreeImage( image_t* image )
{
    if (!image)
        return;

    if (image->bindless)
        glMakeTextureHandleNonResidentARB(image->bindless);

    if (image->glnum)
        glDeleteTextures(1, &image->glnum);

}



// Image hashing
#define IMAGE_BUCKETS 64

image_t* img_buckets[IMAGE_BUCKETS];


static inline void add_to_bucket( image_t* image, int bucket )
{
    image_t **link = &img_buckets[bucket];
    while (*link)
        link = &(*link)->next;
    *link = image;
}


static inline void R_RegisterImage( image_t* image )
{
    int bucket = Hash_String (image->name) % IMAGE_BUCKETS;
    add_to_bucket (image, bucket);
    Con_printf("RegisterImage: Image registered with name: %s\n", image->name);
}

image_t *R_FindImage( const char* name )
{
    int bucket = Hash_String (name) % IMAGE_BUCKETS;
    image_t* img;
    
    for (img = img_buckets[bucket]; img; img = img->next)
    {
        if (Q_stricmp(img->name, name) == 0)
        {
            return img;
        }
    }
    return NULL;
}

