#pragma once
#include <0_GlobalIncludes.h>
#include <curl/curl.h>
enum minimapStates : short { kmmInit = 0, kmmEmptyQueu, kDraw, kTileCheck, kDataLoading, kError };
enum DownloadSM {
        kEmpty = 0,
        kDownloading,
        kDownloaded,
};
struct reqZTile {
        u32 zoom;
        u32u32 tile;
};

struct texZoomTile {
        struct reqZTile zoomTile;
        u32 textureId;
        su64 image;
};
struct minimapPNGsManager {
        // Internal
        enum DownloadSM downloadSt;
        Image requestedImage;
        struct reqZTile requestedTile;
        u32 nITT;
        struct texZoomTile imgTexTile[];
};
struct minimapCTX {
        enum minimapStates state;
        u64 kFramesSkippedDownloading;
        i32 zoom;
        v3 translation;
        v3 scale;
        v3 rotation;

        // internal
        GLuint vao;
        GLuint markerTextureId;
        GLuint shaderId;
        GLuint newTextureId;
        u64u64 textureSize;
        //Historic
        u32 oldZoom;
        u32u32 OldTileXY;
        u64 frameCount;
        struct minimapPNGsManager* pngManager;
};
int renderMinimap(struct minimapCTX *mmContexti, v3 *currentPos, v3 *frontVec, v3 * PointOfOrigin);
