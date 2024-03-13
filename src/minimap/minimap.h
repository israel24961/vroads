#pragma once
#include <0_GlobalIncludes.h>
#include <curl/curl.h>
enum minimapStates : short { kmmInit = 0, kmmEmptyQueu, kReady, kDataReady, kDataLoading, kError };
struct minimapCTX {
        enum minimapStates state;
        u64 kFramesSkippedDownloading;
        i32 zoom;
        v3 translation;
        v3 scale;
        v3 rotation;

        // internal
        GLuint vao;
        GLuint textureIds;
        GLuint markerTextureId;
        GLuint shaderId;
        GLuint newTextureId;
        u64u64 textureSize;
        //Historic
        u32 oldZoom;
        u32u32 OldTileXY;



        u64 frameCount;
};
int renderMinimap(struct minimapCTX *mmContexti, v3 *currentPos, v3 *frontVec, v3 * PointOfOrigin);
