#include "minimap.h"
#include "overpassQueries.h"

u64u64 getTileNumber(float lat, float lon, i32 zoom)
{
        u64 xtile = (u64)((lon + 180.0) / 360.0 * (1 << zoom));
        u64 ytile = (u64)((1.0 - log(tan(lat * M_PI / 180.0) + 1.0 / cos(lat * M_PI / 180.0)) / M_PI) / 2.0 * (1 << zoom));
        return (u64u64){.a = xtile, .b = ytile};
}
su64 TileURL(u64 zoom, u64 x, u64 y)
{
        const char *url = "https://a.tile.openstreetmap.org/%d/%d/%d.png";
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

const v3v2 minimapVertexes[] = {
    {.5,.5, 0, .25,.25},
    {.9,.5, 0, .75,.25},
    {.5,.9, 0, .25,.75},
    {.5,.9, 0, .25,.75},
    {.9,.5, 0, .75,.25},
    {.9,.9, 0, .75,.75},
    
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

int renderMinimap(struct minimapCTX mmContexti, v3 *currentPos, v3 *frontVec, v3 * PointOfOrigin)
{
        static struct minimapCTX mmContext = {.state = kmmInit};
        switch (mmContext.state) {
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
                                mmContext.state = kError;
                                break;
                        }
                        
                        // fragment Shader
                        var fragment = glCreateShader(GL_FRAGMENT_SHADER);
                        glShaderSource(fragment, 1, &fgStr, NULL);
                        glCompileShader(fragment);
                        if (checkCompileErrors(fragment, "FRAGMENT") == 0) {
                                Le("Error loading fragment shader");
                                mmContext.state = kError;
                                break;
                        }
                        // shader Program
                        var ProgID = glCreateProgram();
                        glAttachShader(ProgID, vertex);
                        glAttachShader(ProgID, fragment);
                        glLinkProgram(ProgID);
                        if (checkCompileErrors(ProgID, "PROGRAM") == 0) {
                                Le("Error linking program");
                                mmContext.state = kError;
                                break;
                        }

                        var minimapShaderId = ProgID;
                        Lw("Loaded minimap shader: %d", minimapShaderId);
                        mmContext.shaderId = minimapShaderId;
                        GLuint vao, vbo;
                        glGenVertexArrays(1, &vao);
                        glGenBuffers(1, &vbo);
                        glBindVertexArray(vao);
                        glBindBuffer(GL_ARRAY_BUFFER, vbo);
                        glBufferData(GL_ARRAY_BUFFER, sizeof(v3v2)*6, minimapVertexes, GL_STATIC_DRAW);
                        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(v3v2), (void *)0);
                        glEnableVertexAttribArray(0);
                        // Find the index of textcoords
                        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(v3v2), (void *)sizeof(v3));
                        glEnableVertexAttribArray(1);
                        glEnableVertexAttribArray(0);
                        mmContext.vao = vao;
                        mmContext.state = kmmEmptyQueu;
                }
                break;
        case kmmEmptyQueu:
        case kDataReady:
                // Load the texture
                {
                        Lw("Context: vao: %d, texture: %d", mmContext.vao, mmContext.textureId);
                        const i32 zoom = 10;
                        var latLong = WorldCoordsToLatLong(currentPos->x, currentPos->z, (v2){PointOfOrigin->x, PointOfOrigin->z});
                        var tile = getTileNumber(latLong.x, latLong.y, 10);
                        var tileImage = getTileImage(zoom, tile.a, tile.b);
                        assert(tileImage.datac != NULL);
                        var img = LoadImageFromMemory(".png", tileImage.data, tileImage.size);
                        ImageFlipVertical(&img);
                        assert(img.data != NULL);
                        L("Loaded image : %d, %d", img.width, img.height);
                        GLuint minimapTexId;
                        glGenTextures(1, &minimapTexId);
                        glBindTexture(GL_TEXTURE_2D, minimapTexId);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                        // set texture filtering parameters
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width, img.height, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data);
                        glGenerateMipmap(GL_TEXTURE_2D);
                        glUseProgram(mmContext.shaderId);
                        UnloadImage(img);
                        mmContext.textureId = minimapTexId;
                        mmContext.state = kReady;
                        Lw("Loaded texture: %d", minimapTexId);
                }
                break;
        case kDataLoading:
                break;
        case kReady:
                // Draw minimap triangles with the texture
                glUseProgram(mmContext.shaderId);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mmContext.textureId);
                glUniform1f(glGetUniformLocation(mmContext.shaderId, "rotationRadians"), atan2(frontVec->z, frontVec->x));
                glBindVertexArray(mmContext.vao);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glBindVertexArray(0);
                break;
        case kError:
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
