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
 *  The index increases left to right, bottom to top,
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
 * @param lat Latitude
 * @param lon Longitude
 * @param zoom Zoom level
 * @param distance Distance from the center tile
 */
struct relatedTiles __attr(malloc) * getTileNumbers(f32 lat, f32 lon, i32 zoom, i64 distance)
{
        var center = getTileNumber(lat, lon, zoom);
        var ntiles = 1 + 8 * distance;
        struct relatedTiles *result = malloc(sizeof(u64u64) + ntiles * sizeof(u64u64));
        result->ntiles = ntiles;

        Lw("Allocated %lu bytes for %lu tiles", result->ntiles, ntiles);
        result->zoom = zoom;
        for (i64 i = 0; i < ntiles; i++) {
                var x = center.a + i % 3 - 1;
                var y = center.b + i / 3 - 1;
                result->tiles[i].a = x;
                result->tiles[i].b = y;
        }
        return result;
}
su64 TileURL(u64 zoom, u64 x, u64 y)
{
        const char *url = "https://tile.openstreetmap.org/%d/%d/%d.png";
        su64 urlQuery;
        urlQuery.size = asprintf(&urlQuery.datac, url, zoom, x, y);
        assert(urlQuery.size != -1);
        return urlQuery;
}

static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
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

bool checkCompileErrors(GLuint shader, const char *type)
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
int renderMinimap(struct minimapCTX *mmContext, v3 *currentPos, v3 *frontVec, v3 *PointOfOrigin)
{
        mmContext->frameCount++;
        switch (mmContext->state) {
        case kmmEmptyQueu: {
                // Here we should check if the old data should be discarded
                var hasZoomChanged = mmContext->oldZoom != mmContext->zoom;
                var currTile = currentTileFromLatLong(mmContext, currentPos, frontVec, PointOfOrigin);
                var hasTileChange = AreDifferentTilesUF(mmContext->OldTileXY, currTile);

                if (hasZoomChanged || hasTileChange) {
                        Lw("Zoom changed from %d to %d", mmContext->oldZoom, mmContext->zoom);
                        Lw("Tile changed from (%d, %d) to (%.2f, %.2f)", mmContext->OldTileXY.a, mmContext->OldTileXY.b, currTile.a, currTile.b);
                        goto kDataReadyBlock;
                } else {
                        goto kReadyBlock;
                }

        } break;
        kDataReadyBlock:
        case kDataReady:
                // Load the texture
                {
                        // check if the texture is already Loaded
                        if (mmContext->textureIds != 0) {
                                glDeleteTextures(1, &mmContext->textureIds);
                                Lw("Deleted texture: %d", mmContext->textureIds);
                        }
                        if (mmContext->textureIdsLeft != 0) {
                                glDeleteTextures(1, &mmContext->textureIdsLeft);
                                Lw("Deleted left texture: %d", mmContext->textureIdsLeft);
                        }

                        Lw("Context: vao: %d, texture: %d...", mmContext->vao, mmContext->textureIds);
                        var latLong = WorldCoordsToLatLong(currentPos->x, currentPos->z, (v2){PointOfOrigin->x, PointOfOrigin->z});
                        var tile = getTileNumber(latLong.x, latLong.y, mmContext->zoom);
                        // var tNs = getTileNumbers(latLong.x, latLong.y, mmContext->zoom, 1);

                        L("Tile: (%.2f, %.2f)", tile.a, tile.b);
                        // This is inverted because I use a/x as lat and b/y as long
                        var tileImage = getTileImage(mmContext->zoom, tile.b, tile.a);
                        var tileImageLeft = getTileImage(mmContext->zoom, tile.b, tile.a - 1);

                        assert(tileImage.datac != NULL);
                        var img = LoadImageFromMemory(".png", tileImage.data, tileImage.size);
                        var imgLeft = LoadImageFromMemory(".png", tileImageLeft.data, tileImageLeft.size);

                        assert(img.data != NULL);
                        assert(imgLeft.data != NULL);

                        L("Loaded image : %d, %d", img.width, img.height);
                        L("Loaded left image : %d, %d", imgLeft.width, imgLeft.height);

                        GLuint minimapTexId = vroadLoadTextureClamped(&img);
                        GLuint minimapTexIdLeft = vroadLoadTextureClamped(&imgLeft);

                        mmContext->textureIds = minimapTexId;
                        mmContext->textureIdsLeft = minimapTexIdLeft;

                        L("Loaded texture: %d", minimapTexId);
                        L("Loaded left texture: %d", minimapTexIdLeft);
                        UnloadImage(img);
                        UnloadImage(imgLeft);

                        mmContext->oldZoom = mmContext->zoom;
                        mmContext->OldTileXY = (u32u32){tile.a, tile.b};
                        mmContext->state = kReady;
                }
                break;
        case kDataLoading:
                break;
        kReadyBlock:
        case kReady: {
                // Check current tile
                //  Draw minimap triangles with the texture
                // var latLong = WorldCoordsToLatLong(currentPos->x, currentPos->z, (v2){PointOfOrigin->x, PointOfOrigin->z});
                var tile = currentTileFromLatLong(mmContext, currentPos, frontVec, PointOfOrigin);
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
                glUniform1i(glGetUniformLocation(mmContext->shaderId, "mainTexture"), 0);
                glUniform1i(glGetUniformLocation(mmContext->shaderId, "topTexture"), 1);

                //
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mmContext->textureIds);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, mmContext->textureIdsLeft);

                glBindVertexArray(mmContext->vao);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                //Deactivate GL_TEXTURE1


                // Draw the marker
                // Marker
                rlSetUniformMatrix(4, MatrixScale(.025, .025, 1));
                rlSetUniformMatrix(3, MatrixRotateZ(atan2(frontVec->z, -frontVec->x)));
                // Center image
                glUniform2f(6, .5, .5);
                // Rotation
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mmContext->markerTextureId);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, mmContext->markerTextureId);

                glBindVertexArray(mmContext->vao);
                glDrawArrays(GL_TRIANGLES, 0, 6);

                glBindVertexArray(0);
                mmContext->state = kmmEmptyQueu;
        } break;
        case kError:
                break;
        case kmmInit:
                // Init vao and vbo
                {
                        const char *vxStr __clean = LoadFileText("resources/minimapvs.glsl");
                        const char *fgStr __clean = LoadFileText("resources/minimapfs.glsl");
                        // vertex shader
                        var vertex = glCreateShader(GL_VERTEX_SHADER);
                        glShaderSource(vertex, 1, &vxStr, NULL);
                        glCompileShader(vertex);
                        if (checkCompileErrors(vertex, "VERTEX") == 0) {
                                Le("Error loading vertex shader");
                                mmContext->state = kError;
                                break;
                        }
                        // fragment Shader
                        var fragment = glCreateShader(GL_FRAGMENT_SHADER);
                        glShaderSource(fragment, 1, &fgStr, NULL);
                        glCompileShader(fragment);
                        if (checkCompileErrors(fragment, "FRAGMENT") == 0) {
                                Le("Error loading fragment shader");
                                mmContext->state = kError;
                                break;
                        }
                        // shader Program
                        var ProgID = glCreateProgram();
                        glAttachShader(ProgID, vertex);
                        glAttachShader(ProgID, fragment);
                        glLinkProgram(ProgID);
                        if (checkCompileErrors(ProgID, "PROGRAM") == 0) {
                                Le("Error linking program");
                                mmContext->state = kError;
                                break;
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
                }
                break;
        default:
                assert(false);
                break;
        }
        // var positionx = GetScreenWidth() * 6 / 8;
        // var positiony = GetScreenHeight() * 1 / 10;
        // var scale = .5;
        // Rectangle source = {0.0f, 0.0f, (float)minimapText.width, (float)minimapText.height};
        // Rectangle dest = {positionx, positiony, (float)minimapText.width * scale, (float)minimapText.height * scale};
        // v2 origin = {0, 0};
        // var color = WHITE;
        //
        // rlRotatef(atan2(frontVec->z, frontVec->x) * RAD2DEG, 0, 0, 1);
        // DrawTexturePro(minimapText, source, dest, origin, 0, color);
        return 1;
}
