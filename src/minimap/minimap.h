#pragma once
#include <0_GlobalIncludes.h>
#include <curl/curl.h>
enum minimapStates : short { kmmInit = 0, kmmEmptyQueu, kReady, kDataReady, kDataLoading, kError };
struct minimapCTX {
        enum minimapStates state;
        GLuint vao;
        GLuint textureId;
        GLuint shaderId;
        GLuint newTextureId;
};
int renderMinimap(struct minimapCTX mmContexti, v3 *currentPos, v3 *frontVec, v3 * PointOfOrigin);
