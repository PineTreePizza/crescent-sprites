#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <glad/glad.h>

typedef struct Texture {
    GLuint id;
    unsigned char* data;
    GLsizei width;
    GLsizei height;
    int nrChannels;
    // format
    GLint internalFormat;
    GLint imageFormat;
    // configuration
    GLint wrapS;
    GLint wrapT;
    GLint filterMin;
    GLint filterMag;
    const char* fileName;
} Texture;

Texture* se_texture_create_texture(const char* filePath);
Texture* se_texture_create_solid_colored_texture(GLsizei width, GLsizei height, GLuint colorValue);

#ifdef __cplusplus
}
#endif
