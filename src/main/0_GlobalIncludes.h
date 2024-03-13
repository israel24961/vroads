#pragma once
// clang-format off
#include <glad.h>
// clang-format on

#include "log.h"
#include <GLFW/glfw3.h>
#include <assert.h>
#include <ctype.h>
#include <easycringelib.h>
#include <float.h>
#include <libgen.h>
#include <log.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <search.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

typedef Vector2 v2;
#define v2str "(%f,%f)"
#define v2var(X) (X).x, (X).y
typedef Vector3 v3;
#define v3str "(%f,%f,%f)"
#define v3var(X) (X).x, (X).y, (X).z
typedef Vector4 v4;
#define v4str "(%f,%f,%f,%f)"
#define v4var(X) X.x, X.y, X.z, X.a
typedef Matrix m4;
#define m4str v4str v4str v4str v4str
#define m4var(X) X.m0, X.m4, X.m8, X.m12, X.m1, X.m5, X.m9, X.m13, X.m2, X.m6, X.m10, X.m14, X.m3, X.m7, X.m11, X.m15

typedef struct {
        v3 v3;
        v2 v2;
} v3v2;

GLuint vroadLoadTextureClamped(Image *img);
void texClamped();
void texLinear();
void texMipMap() ;
void texClamped$Linear();
void texClamped$Linear();
u32u32 vroadGenv3v2$vao$vbo(u32 count, v3v2 minimapVertexes[count]);
