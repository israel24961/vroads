#include "minimap.h"
#include "overpassQueries.h"
#include "rlgl.h"

GLuint LoadTextureMarker(Image *marker)
{
        GLuint markerTexId;
        glGenTextures(1, &markerTexId);
        assert(markerTexId != 0);
        glBindTexture(GL_TEXTURE_2D, markerTexId);
        texClamped$Linear();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, marker->width, marker->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, marker->data);
        return markerTexId;
}

/**
 *  \brief Gets the tile number for a given lat/long and zoom level
 *
 *  The index increases left to right, top to bottom
 *  Reference: https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
 *
 * \param lat Latitude
 *  \return {.x = latTile, .y = longTile}
 **/
f32f32 getTileNumber(float lat, float lon, i32 zoom)
{
        var xtile = ((lon + 180.0) / 360.0 * (1 << zoom));
        var ytile = ((1.0 - log(tan(lat * M_PI / 180.0) + 1.0 / cos(lat * M_PI / 180.0)) / M_PI) / 2.0 * (1 << zoom));
        return (f32f32){.y = xtile, .x = ytile};
}

struct relatedTiles {
        i32 zoom;
        u64 ntiles;
        u64u64 tiles[];
};

/**
 * @brief Get the tile numbers object
 * Returns the tile for a given lat/long and zoom level
 * and the ones surrounding it at a given  distance
 * The order is left to right, top to bottom
 * @param tile Center tile (latTile, lonTile)
 * @param zoom Zoom level
 * @param distance Distance from the center tile
 * @param origin Pointer to the origin struct
 * @return struct relatedTiles*
 */
struct relatedTiles __attr(malloc) * getTileNumbers(f32f32 tile, i64 distance)
{
        var center = tile;
        u64 ntiles = pow((2 * distance + 1), 2);
        assert(ntiles > 0);
        struct relatedTiles *result = malloc(sizeof(u64u64) + ntiles * sizeof(u64u64));
        assert(result != NULL);
        result->ntiles = ntiles;

        Lw("Allocated %lu tiles", ntiles);
        var index = 0;
        for (i64 i = -distance; i <= distance; i++) {
                for (i64 j = -distance; j <= distance; j++) {
                        var tile = (f32f32){.x = center.x + i, .y = center.y + j};
                        result->tiles[index++] = (u64u64){.a = (u64)tile.x, .b = (u64)tile.y};
                }
        }
        return result;
}
su64 TileURL(u64 zoom, u64 x, u64 y)
{
        const char *url = "https://tile.openstreetmap.org/%d/%d/%d.png";
        su64 urlQuery;
        urlQuery.size = asprintf(&urlQuery.datac, url, zoom, y, x);
        assert(urlQuery.size != -1);
        return urlQuery;
}

static size_t write_data(void *_r buffer, size_t size, size_t nmemb, void *_r userp)
{
        if (size * nmemb == 0)
                return 0;
        size_t realsize = size * nmemb;
        su64 *mem = userp;
        char *ptr = realloc(mem->data, mem->size + realsize + 1);
        if (!ptr) {
                Le("not enough memory (realloc returned NULL)\n");
                return 0;
        }
        mem->datav = ptr;
        memcpy(&(mem->data[mem->size]), buffer, realsize);
        mem->size += realsize;
        mem->data[mem->size] = 0; // Null ended

        return realsize;
}

su64 downloadImage(su64 urlQuery)
{
        CURLcode ret;
        CURL *hnd;
        struct curl_slist *slist1;
        slist1 = NULL;
        slist1 = curl_slist_append(slist1, "accept: image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8");
        slist1 = curl_slist_append(slist1, "accept-language: en-US,en;q=0.6");
        slist1 = curl_slist_append(slist1, "user-agent: vroads");

        hnd = curl_easy_init();
        curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
        curl_easy_setopt(hnd, CURLOPT_URL, urlQuery.datac);
        curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
        curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/8.6.0");
        curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
        curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
        curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(hnd, CURLOPT_TIMEOUT, 30L);

        su64 dmem = {.data = malloc(0), .size = 0};
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &dmem);
        L("Downloading image from %s", urlQuery.datac);

        ret = curl_easy_perform(hnd);
        L("Downloaded image from %s", urlQuery.datac);
        L("Code: %d", ret);
        if (ret) {
                Le("curl_easy_perform() failed: %s\n", curl_easy_strerror(ret));
                free(dmem.data);
                return (su64){};
        }

        curl_easy_cleanup(hnd);
        hnd = NULL;
        curl_slist_free_all(slist1);
        slist1 = NULL;
        return dmem;
}
su64 getTileImage(i32 zoom, i32 x, i32 y)
{
        var urlQuery = TileURL(zoom, x, y);
        Lw("Requesting image (z:%d, x:%d, y:%d) from %s", zoom, x, y, urlQuery.datac);
        var dmem = downloadImage(urlQuery);
        free(urlQuery.data);
        return dmem;
}

// const v3v2 minimapVertexes[] = {
//     {.5,.5, 0, .25,.25},
//     {.9,.5, 0, .75,.25},
//     {.5,.9, 0, .25,.75},
//     {.5,.9, 0, .25,.75},
//     {.9,.5, 0, .75,.25},
//     {.9,.9, 0, .75,.75},
// };
// Entire scree
const v3v2 minimapVertexes[] = {
    // clang-format off
    {-.5, -.5, 0, 0, 1},   /// 
    {.5, -.5, 0, 1, 1},   /// 
    {-.5, .5, 0, 0, 0},   /// 
    {-.5, .5, 0, 0, 0},   /// 
    {.5, -.5, 0, 1, 1},   /// 
    {.5, .5, 0, 1, 0},   ///
    // clang-format on
};

bool checkCompileErrors(GLuint shader, const char *const type)
{
        GLint success;
        GLchar infoLog[1024];
        if (strcmp(type, "PROGRAM") != 0) {
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                if (!success) {
                        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                        Le("ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s\n", type, infoLog);
                }
        } else {
                glGetProgramiv(shader, GL_LINK_STATUS, &success);
                if (!success) {
                        glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                        Le("ERROR::PROGRAM_LINKING_ERROR of type: %s\n%s\n", type, infoLog);
                }
        }
        return success;
}
f32f32 currentTileFromLatLong(struct minimapCTX *mmContext, v3 *currentPos, v3 *frontVec, v3 *PointOfOrigin)
{
        var latLong = WorldCoordsToLatLong(currentPos->x, currentPos->z, (v2){PointOfOrigin->x, PointOfOrigin->z});
        var tile = getTileNumber(latLong.x, latLong.y, mmContext->zoom);
        return tile;
}

bool AreDifferentTilesUF(u32u32 tileA, f32f32 tileB)
{
        var IsSameA = tileA.a == (u32)tileB.a;
        var IsSameB = tileA.b == (u32)tileB.b;
        return !(IsSameA && IsSameB);
}

int initMinimap(struct minimapCTX *mmContext)
{
        if (mmContext->state != kmmInit)
                return 0;
        const char *vxStr __clean = LoadFileText("resources/minimapvs.glsl");
        const char *fgStr __clean = LoadFileText("resources/minimapfs.glsl");
        // vertex shader
        var vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vxStr, NULL);
        glCompileShader(vertex);
        if (checkCompileErrors(vertex, "VERTEX") == 0) {
                Le("Error loading vertex shader");
                mmContext->state = kError;
                return 1;
        }
        // fragment Shader
        var fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fgStr, NULL);
        glCompileShader(fragment);
        if (checkCompileErrors(fragment, "FRAGMENT") == 0) {
                Le("Error loading fragment shader");
                mmContext->state = kError;
                return 1;
        }
        // shader Program
        var ProgID = glCreateProgram();
        glAttachShader(ProgID, vertex);
        glAttachShader(ProgID, fragment);
        glLinkProgram(ProgID);
        if (checkCompileErrors(ProgID, "PROGRAM") == 0) {
                Le("Error linking program");
                mmContext->state = kError;
                return 1;
        }

        // Load the position of the frame
        var minimapShaderId = ProgID;
        Lw("Loaded minimap shader: %d", minimapShaderId);
        mmContext->shaderId = minimapShaderId;
        var vGen = vroadGenv3v2$vao$vbo(6, minimapVertexes);
        var vao = vGen.a;
        var vbo = vGen.b;

        // Load the texture that represents the person in the minimap
        var marker = LoadImage("resources/marker.png"); // RGBA
        ImageFlipVertical(&marker);
        GLuint markerTexId = LoadTextureMarker(&marker);
        UnloadImage(marker);
        mmContext->markerTextureId = markerTexId;
        Lw("Loaded marker texture: %d", markerTexId);
        glUseProgram(mmContext->shaderId);
        glUseProgram(0);

        // Next State
        mmContext->vao = vao;
        mmContext->state = kmmEmptyQueu;
        return 0;
}
struct MinimapTileTextures {
        union {
                u32 arr[9];
                struct {
                        u32 topLeft;
                        u32 topCenter;
                        u32 topRight;

                        u32 centerLeft;
                        u32 center;
                        u32 centerRight;

                        u32 bottomLeft;
                        u32 bottomCenter;
                        u32 bottomRight;
                } positions;
        };
};
bool reqZTileisEmpty(struct reqZTile t)
{
        var isZoomZero = t.zoom == 0;
        var isTileZero = t.tile.a == 0 && t.tile.b == 0;
        return isZoomZero && isTileZero;
}

bool isEmptyTexZoomTile(struct texZoomTile t)
{
        var isreqZTileEmpty = reqZTileisEmpty(t.zoomTile);
        var isTextureIdZero = t.textureId == 0;
        return isreqZTileEmpty && isTextureIdZero;
}
#include <threads.h>

// Will free *arg at the end
int downloadImageThread(void *arg)
{
        struct {
                struct reqZTile tile;
                struct minimapPNGsManager *ctx;
        } *args = arg;
        var tile = &args->tile;
        var ctx = args->ctx;
        var urlQuery = TileURL(tile->zoom, tile->tile.a, tile->tile.b);
        Lw("Requesting image (z:%d, x:%d, y:%d) from %s", tile->zoom, tile->tile.a, tile->tile.b, urlQuery.datac);
        var dmem = downloadImage(urlQuery);
        free(urlQuery.data);

        // Returned
        ctx->requestedTile = *tile;
        ctx->requestedImage = LoadImageFromMemory(".png", dmem.data, dmem.size);
        ctx->downloadSt = kDownloaded;

        free(args);
        free(dmem.data);
        return 0;
}
bool reqZTileEquals (struct reqZTile *a, struct reqZTile *b)
{
    bool isZoomEqual = a->zoom == b->zoom;
    bool isTileEqual = a->tile.a == b->tile.a && a->tile.b == b->tile.b;
    return isZoomEqual && isTileEqual;
}

struct MinimapTileTextures downloadingMinimapsManager(struct minimapPNGsManager *const ctx, u32 len, struct reqZTile requestedTiles[len])
{
        struct MinimapTileTextures present = {};
        struct reqZTile tilesToRequest[len] = {};
        u32 nTilesToRequest = 0;
        // Check if the tile is already downloaded
        // Sorted imgTexTile by zoom
        for (var i = 0; i < len; i++) {
                struct reqZTile foundreqTile;
                bool found = false;
                for (var j = 0; j < ctx->nITT; j++) {
                        if (reqZTileEquals(&ctx->imgTexTile[j].zoomTile, &requestedTiles[i])) {
                                found = true;
                                foundreqTile = requestedTiles[i];
                                break;
                        }
                }
                if (!found) {
                        tilesToRequest[nTilesToRequest++] = requestedTiles[i];
                        // Unload texture
                        glDeleteTextures(1, &ctx->imgTexTile[i].textureId);
                        present.arr[i] = 0;
                } else {
                        present.arr[i] = ctx->imgTexTile[i].textureId;
                }
        }
        switch (ctx->downloadSt) {
        case kEmpty:
                if (nTilesToRequest > 0) {
                        thrd_t testThread;
                        struct temp {
                                struct reqZTile tile;
                                struct minimapPNGsManager *ctx;
                        };

                        struct temp *arg = malloc(sizeof(struct temp));
                        assert(arg != NULL);
                        arg->ctx = ctx;
                        arg->tile = tilesToRequest[0];

                        ctx->downloadSt = kDownloading;
                        thrd_create(&testThread, downloadImageThread, arg);
                }
                break;
        case kDownloading:
                break;
        case kDownloaded:
                // Check if the tile is requestedTile
                {
                        bool isRequestedTile = false;
                        int index;
                        for (var i = 0u; i < len; i++) {
                                var rqTile = &requestedTiles[i];
                                var ctxZT = &ctx->requestedTile;
                                var isZoomEqual = ctxZT->zoom == rqTile->zoom;
                                var isTileEqual = ctxZT->tile.a == rqTile->tile.a && ctxZT->tile.b == rqTile->tile.b;
                                if (isZoomEqual && isTileEqual) {
                                        isRequestedTile = true;
                                        index = i;
                                        break;
                                }
                        }
                        if (!isRequestedTile) {
                                UnloadImage(ctx->requestedImage);
                                ctx->downloadSt = kEmpty;
                                break;
                        }
                        L("Found downloaded tile zoom:%d, %d %d", ctx->requestedTile.zoom, ctx->requestedTile.tile.a, ctx->requestedTile.tile.b);
                        var img = ctx->requestedImage;
                        var imgTexId = vroadLoadTextureClamped(&img);
                        ctx->imgTexTile[index].zoomTile = ctx->requestedTile;

                        ctx->imgTexTile[index].textureId = imgTexId; // Save the texture
                        present.arr[index] = imgTexId;
                        UnloadImage(img);
                        ctx->downloadSt = kEmpty;
                }
        default:
        }

        return present;
}

int renderMinimap(struct minimapCTX *mmContext, v3 *currentPos, v3 *frontVec, v3 *PointOfOrigin)
{
        mmContext->frameCount++;
        initMinimap(mmContext);

        var tile = currentTileFromLatLong(mmContext, currentPos, frontVec, PointOfOrigin);
        __clean var tNs  = getTileNumbers(tile, 1);
        struct reqZTile requestedTiles[tNs->ntiles];
        for (var i = 0; i < tNs->ntiles; i++) {
                var t = tNs->tiles[i];
                requestedTiles[i] = (struct reqZTile){.zoom = mmContext->zoom, .tile = (u32u32){.a = t.a, .b = t.b}};
        }
        var tileTxts = downloadingMinimapsManager(mmContext->pngManager, tNs->ntiles, requestedTiles);

        var tileDecimals = (f32f32){.a = tile.a - (i32)tile.a, .b = tile.b - (i32)tile.b};
        // Lw("Portion of current tile: (%f32, %f32)", tileDecimals.a, tileDecimals.b);
        glUseProgram(mmContext->shaderId);
        glUniform2f(5, mmContext->translation.x, mmContext->translation.y); // Translate Window
                                                                            //
        // Draw the minimap
        rlSetUniformMatrix(4, MatrixScale(mmContext->scale.x, mmContext->scale.y, mmContext->scale.z));
        rlSetUniformMatrix(3, MatrixRotateZ(0));
        glUniform2f(6, tileDecimals.a, tileDecimals.b); // Center image
        // glUniform2f(6, 0, 0); // Center image
        glUniform1i(glGetUniformLocation(mmContext->shaderId, "topLeftTexture"), 0);
        glUniform1i(glGetUniformLocation(mmContext->shaderId, "topCenterTexture"), 1);
        glUniform1i(glGetUniformLocation(mmContext->shaderId, "topRightTexture"), 2);
        glUniform1i(glGetUniformLocation(mmContext->shaderId, "leftTexture"), 3);
        glUniform1i(glGetUniformLocation(mmContext->shaderId, "mainTexture"), 4);
        glUniform1i(glGetUniformLocation(mmContext->shaderId, "rightTexture"), 5);
        glUniform1i(glGetUniformLocation(mmContext->shaderId, "bottomLeftTexture"), 6);
        glUniform1i(glGetUniformLocation(mmContext->shaderId, "bottomTexture"), 7);
        glUniform1i(glGetUniformLocation(mmContext->shaderId, "bottomRightTexture"), 8);



        for (var i = 0; i < 9; i++) {
                var texId = GL_TEXTURE0 + i;
                glActiveTexture(texId);
                glBindTexture(GL_TEXTURE_2D, tileTxts.arr[i]);
        }

        glBindVertexArray(mmContext->vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Deactivate GL_TEXTURE1

        // Draw the marker
        // Marker
        rlSetUniformMatrix(4, MatrixScale(.025, .025, 1));
        rlSetUniformMatrix(3, MatrixRotateZ(atan2(frontVec->z, -frontVec->x)));
        // Center image
        glUniform2f(6, .5, .5);
        // Rotation
        for (var i = 0; i < 9; i++) {
                var texId = GL_TEXTURE0 + i;
                glActiveTexture(texId);
                glBindTexture(GL_TEXTURE_2D, mmContext->markerTextureId);
        }
        //glUniform1i(glGetUniformLocation(mmContext->shaderId, "mainTexture"), 0);
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, mmContext->markerTextureId);
        // glActiveTexture(GL_TEXTURE1);
        // glBindTexture(GL_TEXTURE_2D, mmContext->markerTextureId);

        glBindVertexArray(mmContext->vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);
        mmContext->state = kmmEmptyQueu;
        glUseProgram(0);
        return 0;
}
