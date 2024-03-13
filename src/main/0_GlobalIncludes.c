#include "0_GlobalIncludes.h"
GLuint vroadLoadTextureClamped(Image *img)
{
        GLuint texId = 0;
        glGenTextures(1, &texId);
        assert(texId != 0);
        glBindTexture(GL_TEXTURE_2D, texId);
        // Don't repeat
        // set texture filtering parameters
        texClamped();
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->width, img->height, 0, GL_RGB, GL_UNSIGNED_BYTE, img->data);
        glGenerateMipmap(GL_TEXTURE_2D);
        return texId;
}
void texClamped()
{
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
}
void texLinear()
{
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
void texMipMap() { glGenerateMipmap(GL_TEXTURE_2D); }

void texClamped$Linear()
{
        texClamped();
        texLinear();
}

// Needs to glEnableVertexAttribArray(0);
u32u32 vroadGenv3v2$vao$vbo(u32 count, v3v2 minimapVertexes[count])
{
        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        assert(vao != 0);
        assert(vbo != 0);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(v3v2) * count, minimapVertexes, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(v3v2), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(v3v2), (void *)sizeof(v3));
        glEnableVertexAttribArray(1);
        return (u32u32){vao, vbo};
}
