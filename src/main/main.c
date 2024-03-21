#include "main.h"
#include "minimap.h"
#include "overpassQueries.h"
#define null NULL

var PointOfOrigin = (v3){40.42965716240298f, 0, -3.6858290101741833f};
Color invertColor(Color c) { return (Color){255 - c.r, 255 - c.g, 255 - c.b, c.a}; }
GLFWwindow *window;
typedef float Radian;
typedef float Grades;
int gameStuff(void *args);
f32 processCam(CameraMode cm, Camera *cam);

typedef struct {
        v3v2 *ver_texcoords;
        u32_3 *indices;
        u32 vertexCount;
        u32 indexCount;
} LiteMesh;
typedef struct {
        i64i64 i64i64;
        v3 v3;
        m4 transform;
        v3 nextPost;
} i64i64v3;

typedef pathPoints *PathsList;

struct renderRoadsArgs {
        PathsList **roadsAsPaths;
        bool *RoadsInMeters;
        LiteMesh **loadedRoadsLP;
        GLuint vao, vbo, ebo;
};
int renderRoads(struct renderRoadsArgs *args);

i64i64v3 runningGuyNF(struct ModelAnimation *anims, Model *model, v3 *frontVec, v3 *currentPos);
enum g_CKind { dot, circle, greendot };
void g_CDrawFactory(enum g_CKind kind, v2 center)
{
        struct g_C resp;
        Color color;
        switch (kind) {
        case dot:
                resp = (struct g_C){.center = center, .radius = .01};
                color = ORANGE;
                break;
        case greendot:
                resp = (struct g_C){.center = center, .radius = .01};
                color = GREEN;
                break;
        case circle:
                resp = (struct g_C){.center = center, .radius = .1};
                color = BLUE;
                break;
        default:
                break;
        }
        DRAWC(resp, color);
}

struct Point {
        double Lat;
        double Lon;
        char *str;
};
struct Point str2Point(char *strpoint)
{
        char *lastT = NULL;
        struct Point p;
        p.str = strpoint;
        p.Lat = strtod(strpoint + 1, &lastT);
        strpoint = lastT;
        p.Lon = strtod(strpoint + 1, &lastT);
        return p;
}
double PointDistance(struct Point *P1, struct Point *P2) { return haversine_distance(P1->Lat, P1->Lon, P1->Lat, P2->Lon); }
#define STR(X) #X
int ui_width, ui_height;
void prev()
{
        ui_width = width * 1 / 12;
        ui_height = height * 1 / 12;
}
char *get_area(float distance, double center_lat, double center_lon)
{
        char *cmdTemplate = "m4 -DBOTTOM_LEFT_LAT=%.6f -DBOTTOM_LEFT_LON=%.6f "
                            "-DCIRCLE_METERS=%.2f "
                            "./resources/queryArea.m4";
        char *cmd = NULL;
        asprintf(&cmd, cmdTemplate, center_lat, center_lon, distance);
        var cmdStr = read_command(cmd);
        return cmdStr;
}
char *get_bbox(double bottom_left_lat, double bottom_left_lon, double top_right_lat, double top_right_lon)
{
        char *cmdTemplate = "m4 -DBOTTOM_LEFT_LAT=%.6f -DBOTTOM_LEFT_LON=%.6f "
                            "-DTOP_RIGHT_LAT=%.6f -DTOP_RIGHT_LON=%.6f "
                            "./resources/queryBBOX.m4";
        char *cmd = NULL;
        asprintf(&cmd, cmdTemplate, bottom_left_lat, bottom_left_lon, top_right_lat, top_right_lon);
        return read_command(cmd);
}
struct inputArgs {
        Camera3D *cam;
};
struct proyectile {
        Vector3 pos;
        Vector3 speed;

        // Initial time
        time_t it;
        time_t ft;
        float aliveSecs;
};
struct proyectile *proyectiles = NULL;
int nProyectiles = 10;
void fireProyectile(Vector3 pos, Vector3 target)
{
        // Initialize proyectiles, if not initialized
        struct proyectile defaultProyectile = {.aliveSecs = 1000};
        ;
        if (proyectiles == NULL) {
                proyectiles = calloc(sizeof(struct proyectile), nProyectiles);
                for (int i = 0; i < nProyectiles; i++) {
                        proyectiles[i] = defaultProyectile;
                }
        }

        // Check if there is a proyectile that can be reused
        float scaledSpeed = 25;
        bool isReused = false;
        for (int i = 0; i < nProyectiles; i++) {
                if (proyectiles[i].ft < time(NULL)) {
                        proyectiles[i].pos = pos;
                        proyectiles[i].speed = Vector3Scale(target, scaledSpeed);
                        proyectiles[i].it = time(NULL);
                        proyectiles[i].ft = proyectiles[i].it + (int)proyectiles[i].aliveSecs;
                        isReused = true;
                        break;
                }
        }
        if (isReused)
                return;
        else {
                // Remove the oldest proyectile
                struct proyectile *oldestProyectile = &proyectiles[0];
                for (int i = 1; i < nProyectiles; i++) {
                        if (proyectiles[i].it < oldestProyectile->it) {
                                oldestProyectile = &proyectiles[i];
                        }
                }
                oldestProyectile->pos = pos;
                oldestProyectile->speed = Vector3Scale(target, scaledSpeed);
                oldestProyectile->it = time(NULL);
                oldestProyectile->ft = oldestProyectile->it + (int)oldestProyectile->aliveSecs;
        }
}
void drawProyectiles(float dt)
{
        if (proyectiles == NULL)
                return;
        for (int i = 0; i < nProyectiles; i++) {
                if (proyectiles[i].ft > time(NULL)) {
                        proyectiles[i].pos.x += proyectiles[i].speed.x * dt;
                        proyectiles[i].pos.y += proyectiles[i].speed.y * dt;
                        proyectiles[i].pos.z += proyectiles[i].speed.z * dt;
                        DrawSphere(proyectiles[i].pos, 0.1, RED);
                }
        }
}
void processInput(struct inputArgs args)
{
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                // shot from the model

                fireProyectile(args.cam->target,
                               Vector3Normalize((Vector3){args.cam->target.x - args.cam->position.x, 0, args.cam->target.z - args.cam->position.z}));
        }
}
#include <fcntl.h>
#define ESC_KEY 27

int getkey()
{
        // Make reading from stdin nonblocking.
        int flags = fcntl(STDIN_FILENO, F_GETFL);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

        int ch = getchar();

        // Turn off nonblocking I/O. On some systems, leaving stdin
        // nonblocking will also leave stdout nonblocking, which
        // can cause printing errors.
        fcntl(STDIN_FILENO, F_SETFL, flags);
        return ch;
}
int *fillBinaryBody(int messageLength)
{
        int numberOfElements = powf(2, messageLength);
        int *resp = calloc(numberOfElements, sizeof(int));
        for (int i = 0; i < numberOfElements; i++) {
                resp[i] = i;
        }
        return resp;
}
i32 onesWeight(i32 value)
{
        i32 numberOfOnes = 0;
        for (i32 i = 0; i < 32; i++) {
                if (value & (1 << i))
                        numberOfOnes++;
        }
        return numberOfOnes;
}
i32 *i32Selecti32(i32 *binaryNumbers, int length, i32 (*f)(i32))
{
        i32 *resp = malloc(sizeof(i32) * length);
        for (i32 i = 0; i < length; i++) {
                resp[i] = f(binaryNumbers[i]);
        }
        return resp;
}
i32 compareInts(const void *a, const void *b)
{
        i32 valA = onesWeight(*(i32 *)a);
        i32 valB = onesWeight(*(i32 *)b);
        return valB - valA;
}

int abs_mod(int n, int base) { return (n % base + base) % base; }
int binaryStuff()
{
        i32 noS = 7;

        i32 *binaryBody = fillBinaryBody(noS);
        i32 *binaryWeights = i32Selecti32(binaryBody, powf(2, noS), onesWeight);

        for (i32 i = 0; i < powf(2, noS); i++) {
                L("binaryBody[%.2d] = %.*b -- w(x)=%i", i, noS, binaryBody[i], binaryWeights[i]);
        }
        i32 *sortedBinaryWeights = calloc(powf(2, noS), sizeof(i32));
        for (i32 i = 0; i < powf(2, noS); i++) {
                sortedBinaryWeights[i] = binaryBody[i];
        }
        qsort(sortedBinaryWeights, powf(2, noS), sizeof(i32), compareInts);

        i32 *sortedBinaryByWeightWeights = i32Selecti32(sortedBinaryWeights, powf(2, noS), onesWeight);
        for (i32 i = 0; i < powf(2, noS); i++) {
                L("SortedArray[%.2d] = %.*b -- w(x)=%i", i, noS, sortedBinaryWeights[i], sortedBinaryByWeightWeights[i]);
        }
        return 0;
}
pathPoints *WayToCoordsPaths(struct way *w)
{
        pathPoints *resp = malloc(sizeof(pathPoints));
        v3 *n0 = malloc(sizeof(v3) * w->nodesCount);
        resp->vecs = n0;
        resp->nElements = w->nodesCount;
        for (int i = 0; i < w->nodesCount; i++) {
                n0[i] = (v3){w->nodes[i].lat, w->level, w->nodes[i].lon};
        }
        return resp;
}
/// Return overpass result as a string
__attr(malloc) OverpassQuery *retrieveRoadsForGame(__attribute__((unused)) void *ov)
{
        // Small place: 820 elements (700 nodes, 120 ways)
        double bottom_left_lat = 40.4293746, bottom_left_lon = -3.6857843; //, top_right_lat = 40.4363450, top_right_lon = -3.6735964;
        // Big place: 90k elements (72k nodes, 18k ways)
        // double bottom_left_lat = 40.3875813, bottom_left_lon = -3.7207142,
        //        top_right_lat = 40.4652025, top_right_lon = -3.5965000;
        tic = clock();
        L("Query: incoming");
        // char *query = get_bbox(bottom_left_lat, bottom_left_lon, top_right_lat, top_right_lon);
        char *query = get_area(getenv("DISTANCE_FROM_CENTER") ? atof(getenv("DISTANCE_FROM_CENTER")) : 1000, bottom_left_lat, bottom_left_lon);
        L("Query: %s", query);
        toc = clock();
        ISNULL(query, ("Error reading command output"));
        tic = clock();
        var overpassQuery = OverpassQuery_new("NormalQuery", query, NULL);
        overpassQuery->launchQuery(overpassQuery);
        toc = clock();
        struct elements *el = &overpassQuery->elements;
        if (el->nodes != NULL && el->ways != NULL) {
                i64i64 resp = printElements(*el);
                Ld("Nodes: %ld, Ways: %ld", resp.a, resp.b);
        }
        // We already have the nodes and ways
        //  We will return an array of ways (paths with N nodes or points)
        PathsList *resp = malloc(sizeof(PathsList) * (overpassQuery->elements.waysCount + 1));
        resp[overpassQuery->elements.waysCount] = NULL;
        for (u64 i = 0; i < overpassQuery->elements.waysCount; i++) {
                resp[i] = WayToCoordsPaths(&overpassQuery->elements.ways[i]);
        }
        var ppPathsList = (PathsList **)ov;
        *ppPathsList = resp;
        return overpassQuery;
}

// Returns the delta in meters for lat and long
int MapCoordsToGraphic(__attribute__((unused)) void *args)
{
        // List of roads
        // thrd_t overpassThread;
        PathsList *roadsAsPaths = NULL;
        var ovQ = retrieveRoadsForGame(&roadsAsPaths);
        free(ovQ->query);
        free(ovQ); // TODO: Free elements
        var deltas = LatLongToWorldDeltas(PointOfOrigin);
        for (u32 i = 0; roadsAsPaths[i] != NULL; i++) {
                var pathRoad = roadsAsPaths[i];
                var scale = getenv("SCALE") ? atof(getenv("SCALE")) : 1;
                for (u32 j = 0; j < pathRoad->nElements; j++) {
                        pathRoad->vecs[j].x = (pathRoad->vecs[j].x - PointOfOrigin.x) * deltas.x;
                        pathRoad->vecs[j].y = scale * pathRoad->vecs[j].y;
                        pathRoad->vecs[j].z = (pathRoad->vecs[j].z - PointOfOrigin.z) * -deltas.y;
                }
        }
        // Save to data_meters.json

        var castedArgs = (PathsList **)args;
        *castedArgs = roadsAsPaths;
        return 0;
}
int main(int argc, char *argv[])
{
#include <stddef.h>
        // Start log
#ifdef DEBUG
        L("Debug mode");
        log_set_level(LOG_DEBUG);
#endif
        parse_options(argc, argv);
        L("The C version is %s %ld\n", __STDC__ == 0 ? "Cstd" : "GNUC", __STDC_VERSION__);
        L("Max number of textures %d", GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
        // RayThread
        thrd_t testThread;
        PathsList *reg = NULL;
        thrd_create(&testThread, MapCoordsToGraphic, &reg);

        thrd_t raythread;
        thrd_create(&raythread, gameStuff, &reg);

        // thrd_join(raythread, NULL);
        thrd_join(raythread, NULL);
        return 0;
}
struct LoadModelReturn {
        Model model;
        Texture2D texture;
};
struct LoadModelReturn loadModel()
{
        Model model = LoadModel("resources/guy.iqm");            // Load the animated model mesh
        Texture2D texture = LoadTexture("resources/guytex.png"); // Load model texture and set material
        SetMaterialTexture(&model.materials[0], MATERIAL_MAP_DIFFUSE,
                           texture); // Set model material map texture
        struct LoadModelReturn resp = {.model = model, .texture = texture};
        return resp;
}
#define BACKGROUND_COLOR BLACK
// Invert color function

v3 renderRayAndFloorIntersection(v3 camTarget, v3 frontVec, v3 FloorPoint, v3 FloorNormal)
{
        // Draw a ray from the camera to the floor

        v3v3 viewRay = intersecRayPlane(camTarget, frontVec, FloorPoint, FloorNormal);
        if (viewRay.lineStart.x <= 0.01 && viewRay.lineStart.y <= 0.01 && viewRay.lineStart.z <= 0.01)
                return (v3){0, 0, 0};
        // DrawLine3D(Vector3Add(viewRay.lineStart, (v3){0, -0.1, 0}), viewRay.lineEnd, RED);
        //  Draw a circle in the floor
        var scale = getenv("SCALE") ? atof(getenv("SCALE")) : 1;
        DrawSphere(viewRay.lineEnd, scale * .1, RED);
        return viewRay.lineEnd;
}
typedef struct {
        v3 topRight;
        v3 topLeft;
} Rectangle3D;

void setTexCoordScale(float *vertexcoords, float *texcoords, float scale)
{
        v3 squaredRoad[4] = {{0, 0, 0}, {scale, 0, 0}, {scale, 0, 1}, {0, 0, 1}};
        for (int i = 0; i < 4; i++) {
                texcoords[i * 2] = squaredRoad[i].x;
                texcoords[i * 2 + 1] = squaredRoad[i].z;
        }
        for (int i = 0; i < 4; i++) {
                vertexcoords[i * 3] = squaredRoad[i].x;
                vertexcoords[i * 3 + 1] = squaredRoad[i].y;
                vertexcoords[i * 3 + 2] = squaredRoad[i].z;
        }
}
/// Generates a unique mesh from a list of paths

/// Generate a road from a path
/// If mesh is not NULL, it will be used to store the mesh, vtxCoords should already be allocated as a List(the last available position will be zero
///
/// @returns a pointer to the road
LiteMesh GenRoadFromPath(u32 path_size, v3 path[path_size], f32 roadWidth, LiteMesh *mesh)
{ // Will refer as vertex to a point of the road and point to an element of the path
        LiteMesh m = {0};
        assert(path_size > 1);
        v3v2 *vtxCoords = NULL;
        u32_3 *indices = NULL;
        var skippedMeshVTXCoods = 0;
        var skippedMeshIndices = 0;
        if (mesh != NULL) {
                skippedMeshVTXCoods = mesh->vertexCount;
                skippedMeshIndices = mesh->indexCount;
                vtxCoords = mesh->ver_texcoords + skippedMeshVTXCoods;
                indices = mesh->indices + skippedMeshIndices;
                // Ld("Skipped %d vtxCoords and %d indices", skippedMeshVTXCoods, skippedMeshIndices);
        } else {
                vtxCoords = calloc(sizeof(v3v2), 2 * path_size);
                // Indices, for 2p -> 2 triangles, 3p->4 triangles, 4p->6 triangles, 5p->8p
                indices = calloc(sizeof(u32_3), (path_size - 1) * 2);
        }
        v2 pointA = (v2){path[0].x, path[0].z};
        v2 pointB = (v2){path[1].x, path[1].z};

        // Obtaining the right vector (cross product)
        v2 dn = Vector2Normalize(Vector2Subtract(pointB, pointA));
        v2 dp = {-dn.y, dn.x}; //* roadWidth / 2
        v2 scaledDp = Vector2Scale(dp, roadWidth / 2);
        var lastDiff = Vector2Length(Vector2Subtract(pointB, pointA));
        v3 botomLeft = (v3){pointA.x + scaledDp.x, path[0].y, pointA.y + scaledDp.y};
        v3 botomRight = (v3){pointA.x - scaledDp.x, path[0].y, pointA.y - scaledDp.y};
        vtxCoords[0].v3 = botomLeft;
        vtxCoords[0].v2 = (v2){0, 0};
        vtxCoords[1].v3 = botomRight;
        vtxCoords[1].v2 = (v2){0, 1};
        // Now we iterate through the rest
        if (path_size == 2)
                goto lastPoint;
        for (u32 i = 1; i < path_size - 1; i++) {
                pointA = (v2){path[i].x, path[i].z};
                pointB = (v2){path[i + 1].x, path[i + 1].z};
                var curdn = Vector2Subtract(pointB, pointA);
                var dpCurrent = Vector2Normalize((v2){-curdn.y, curdn.x});
                // For curves
                //
                v2 dpsAdded = Vector2Normalize(Vector2Add(dpCurrent, dp));
                v2 dpsAddedScaled = Vector2Scale(dpsAdded, roadWidth / 2);

                v3 currentBottomLeft = (v3){pointA.x + dpsAddedScaled.x, path[i].y, pointA.y + dpsAddedScaled.y};
                var diff = Vector2Length(curdn);
                vtxCoords[i * 2].v3 = currentBottomLeft;
                vtxCoords[i * 2].v2 = (v2){lastDiff, 0};

                var currentBottomRight = (v3){pointA.x - dpsAddedScaled.x, path[i].y, pointA.y - dpsAddedScaled.y};
                vtxCoords[i * 2 + 1].v3 = currentBottomRight;
                vtxCoords[i * 2 + 1].v2 = (v2){lastDiff, 1};
                lastDiff += diff;
                dp = dpCurrent; // Last perpendicular vector, this could be dpForCurves, TODO:TEST
        }
lastPoint:
        // For the last  iteration (dp is the last one)
        pointA = (v2){path[path_size - 2].x, path[path_size - 2].z};
        pointB = (v2){path[path_size - 1].x, path[path_size - 1].z};
        var pA_pB = Vector2Subtract(pointB, pointA);
        // lastDiff += Vector2Length(pA_pB);
        dn = Vector2Normalize(pA_pB);
        dp = (v2){-dn.y, dn.x};
        v2 dpsAddedScaled = Vector2Scale(dp, roadWidth / 2);
        v3 currentBottomLeft = (v3){pointB.x + dpsAddedScaled.x, path[path_size - 1].y, pointB.y + dpsAddedScaled.y};
        v3 currentBottomRight = (v3){pointB.x - dpsAddedScaled.x, path[path_size - 1].y, pointB.y - dpsAddedScaled.y};
        vtxCoords[(path_size - 1) * 2].v3 = currentBottomLeft;
        vtxCoords[(path_size - 1) * 2].v2 = (v2){lastDiff, 0};
        vtxCoords[(path_size - 1) * 2 + 1].v3 = currentBottomRight;
        vtxCoords[(path_size - 1) * 2 + 1].v2 = (v2){lastDiff, 1};

        // And the last two Indices
        for (u32 i = 0; i < 2 * (path_size - 1); i += 2) {
                indices[i] = (u32_3){i + skippedMeshVTXCoods, i + 1 + skippedMeshVTXCoods, (i + 2 + skippedMeshVTXCoods)};
                indices[i + 1] = (u32_3){i + skippedMeshVTXCoods + 1, i + 3 + skippedMeshVTXCoods, i + 2 + skippedMeshVTXCoods};
        }
        m.ver_texcoords = vtxCoords;
        m.indices = indices;
        m.indexCount = 2 * (path_size - 1);
        m.vertexCount = 2 * path_size;
        return m;
}
LiteMesh *GenRoadFromPathList(PathsList *pathL, f32 roadWith)
{
        // First we will count the number of elements
        u32 numberOfWays = 0, numberOfNodes = 0;
        u32 nIndices = 0;
        for (u32 i = 0; pathL[i] != NULL; i++) {
                numberOfWays++;
                numberOfNodes += pathL[i]->nElements;
                nIndices += 2 * (pathL[i]->nElements - 1);
        }
        assert(nIndices == 2 * (numberOfNodes - numberOfWays));
        LiteMesh *mp = malloc(sizeof(LiteMesh));
        mp->ver_texcoords = calloc(sizeof(v3v2), numberOfNodes * 2);
        mp->indices = calloc(sizeof(u32_3), (numberOfNodes - numberOfWays) * 2);
        mp->vertexCount = 0;
        mp->indexCount = 0;
        // Ld("From %d paths with around %d nodes, we will need %d vertices and %d indices, %d indexRaw", numberOfWays, numberOfNodes,
        // mp->vertexCount,
        //    mp->indexCount, nIndices);
        var timeStart = clock();
        for (u32 i = 0; i < numberOfWays; i++) {

                var path = pathL[i];
                var recvMesh = GenRoadFromPath(path->nElements, path->vecs, roadWith, mp);
                // Ld("Path to skip %d vtxCoords and %d indices", recvMesh.vertexCount, recvMesh.indexCount);
                mp->vertexCount += recvMesh.vertexCount;
                mp->indexCount += recvMesh.indexCount;
        }
        assert(mp->vertexCount == numberOfNodes * 2);
        assert(mp->indexCount == (numberOfNodes - numberOfWays) * 2);
        L("Time to generate the road: %fms ( per road %fms)", (float)(clock() - timeStart) / CLOCKS_PER_SEC * 1000,
          (float)(clock() - timeStart) / CLOCKS_PER_SEC * 1000 / numberOfWays);
        return mp;
}
void setTexCoordFromRectangle(v2v2 line, float *vertexcoords, float *texcoords)
{
        v2 pointA = line.lineStart;
        v2 pointB = line.lineEnd;
        // For the first point
        v2 vector = {pointB.x - pointA.x, pointB.y - pointA.y};
        v2 rightVec = Vector2Normalize((v2){-vector.y, vector.x});

        float roadWidth = 1;
        v3 rightVecV3 = {rightVec.x * roadWidth / 2, 0, rightVec.y * roadWidth / 2};

        v3 botomLeft = Vector3Add((v3){pointA.x, 0, pointA.y}, rightVecV3);
        v3 botomRight = Vector3Subtract((v3){pointA.x, 0, pointA.y}, rightVecV3);

        v3 topLeft = Vector3Add((v3){pointB.x, 0, pointB.y}, rightVecV3);
        v3 topRight = Vector3Subtract((v3){pointB.x, 0, pointB.y}, rightVecV3);

        v3 squaredRoad[4] = {botomLeft, botomRight, topRight, topLeft};
        float scale = Vector2Distance(pointA, pointB);
        v3 texScale[4] = {{0, 0, 0}, {0, 0, 1}, {scale, 0, 1}, {scale, 0, 0}};
        for (int i = 0; i < 4; i++) {
                texcoords[i * 2] = texScale[i].x;
                texcoords[i * 2 + 1] = texScale[i].z;
        }

        for (int i = 0; i < 4; i++) {
                vertexcoords[i * 3] = squaredRoad[i].x;
                vertexcoords[i * 3 + 1] = squaredRoad[i].y;
                vertexcoords[i * 3 + 2] = squaredRoad[i].z;
        }
        // clang-format off
        // Line in to a rectangle
         // (p0x,p0z) -----                         (p0x,0,p0z)-rightVecV3*roadWidth/2 -------------------------  (p0x,0,p1z)-rightVecV3*roadWidth/2
         //                ----                ===>     
         //                    --->(p1x,p1z)        (p0x,0,p1z)-rightVecV3*roadWidth/2 ------------------------- (p1x,0,p1z)-rightVecV3*roadWidth/2
        // We will map the line to a XyZ rectangle
        // clang-format on
}

int roadVertexGen(void *args)
{
        L("-----Road vertex gen");
        struct {
                PathsList *pathList;
                LiteMesh **resultp;
        } *roadArgs = args;
        var numberOfWays = 0;
        var numberOfNodes = 0;
        for (u32 i = 0; roadArgs->pathList[i] != NULL; i++) {
                numberOfWays++;
                numberOfNodes += roadArgs->pathList[i]->nElements;
        }
        L("Found %d ways and %d nodes", numberOfWays, numberOfNodes);

        LiteMesh *meshesA = GenRoadFromPathList(roadArgs->pathList, getenv("ROADWIDTH") != null ? atof(getenv("ROADWIDTH")) : 1.0);
        *roadArgs->resultp = meshesA;
        return 0;
}
FILE *fG = NULL;
void sigsevHandler(int sig)
{
        if (fG != NULL) {
                fprintf(fG, "-----------SIGSEV: %d", sig);
                fclose(fG);
        }
        printf("-----------SIGSEV: %d", sig);
        exit(1);
}
int gameStuff(__attribute__((unused)) void *args)
{
        // signal(SIGSEGV, sigsevHandler);
        _raylibInit();
        PathsList **roadsAsPaths = args;
#ifdef DEBUG
        log_set_level(LOGC_TRACE);
#else
        log_set_level(LOGC_DEBUG);
#endif
        SetTraceLogLevel(LOG_INFO);
        Camera3D cam = {0};
        cam.position = (Vector3){0, 2, 0};
        cam.target = (Vector3){.4f, 1.0f, .4f};
        cam.up = (Vector3){0.0f, .2f, 0.0f};
        cam.fovy = 80.0f;
        cam.projection = CAMERA_PERSPECTIVE;
        DisableCursor();

        // Girl model and animations
        Model girlModel = LoadModel("resources/bettergirl.glb");

        girlModel.transform = MatrixScale(.05, .05, .05);
        girlModel.transform = MatrixMultiply(girlModel.transform, MatrixRotateX(90 * DEG2RAD));
        int animsCount = 0;
        ModelAnimation *animGirl = LoadModelAnimations("resources/bettergirl.glb", &animsCount);
        for (int i = 0; i < animsCount; i++) {
                L("Animation %d: %s", i, animGirl[i].name);
        }

        // Load texture
        Image roadImg = LoadImage("resources/road.png"); // Load model texture and set material
        Texture2D roadTexture = LoadTextureFromImage(roadImg);

        ISNULL(roadTexture.id, ("Error loading texture"));
        //  be unloaded from RAM
        Vector3 modelPosition = {0, 0, 0};

        // Road shaders and mesh
        char *vxStr = LoadFileText("resources/mainvs.glsl");
        ISNULL(vxStr, ("Error loading vertex shader"));
        char *fgStr = LoadFileText("resources/mainfs.glsl");
        ISNULL(fgStr, ("Error loading fragment shader"));
        Shader roadShader = LoadShaderFromMemory(vxStr, fgStr);
        int i = GetShaderLocationAttrib(roadShader, "vertexPosition");
        L("vertexPosition: %d", i);
        i = rlGetLocationUniform(roadShader.id, "mvp");
        free(vxStr);
        free(fgStr);
        // L("mvp: %d", i);
        // L("Program shader id: %d", roadShader.id);

        // Road model
        Model floorModel = LoadModel("resources/floor.glb");
        // floorModel.materialCount++;
        floorModel.materials = realloc(floorModel.materials, sizeof(Material) * 2);
        floorModel.materials[1] = floorModel.materials[0];

        roadShader.locs[SHADER_LOC_VERTEX_POSITION] = GetShaderLocationAttrib(roadShader, "vertexPosition");
        roadShader.locs[SHADER_LOC_MATRIX_MVP] = rlGetLocationUniform(roadShader.id, "mvp");
        roadShader.locs[SHADER_LOC_VERTEX_TEXCOORD01] = GetShaderLocationAttrib(roadShader, "vertexTexCoord");
        roadShader.locs[SHADER_LOC_VERTEX_COLOR] = GetShaderLocationAttrib(roadShader, "vertexColor");
        // Every vertex loc
        L("SHADER_LOC_VERTEX_POSITION: %d", roadShader.locs[SHADER_LOC_VERTEX_POSITION]);
        L("SHADER_LOC_MATRIX_MVP: %d", roadShader.locs[SHADER_LOC_MATRIX_MVP]);
        L("SHADER_LOC_VERTEX_TEXCOORD01: %d", roadShader.locs[SHADER_LOC_VERTEX_TEXCOORD01]);
        L("SHADER_LOC_VERTEX_COLOR: %d", roadShader.locs[SHADER_LOC_VERTEX_COLOR]);

        // Matrix roadTransform = MatrixIdentity();
        Material roadMaterial = LoadMaterialDefault();
        // SetMaterialTexture(&roadMaterial, MATERIAL_MAP_DIFFUSE, roadTexture);
        floorModel.materials = &roadMaterial;
        roadMaterial.shader = roadShader;

        // Just a triangle
        uint vao = 0, vbo = 0;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        window = GetWindowHandle();

        // Road path
        v3 path[] = {{0, 0, 0}, {4, 0, 0}}; //,{4,0,4}};
        var results = GenRoadFromPath(sizeof(path) / sizeof(v3), path, getenv("ROADWIDTH") != null ? atoi(getenv("ROADWIDTH")) : 1.0, NULL);
        // Ld("results: %p", &results);
        // Ld("vertexCount: %d", results.vertexCount);
        // Ld("indexCount: %d", results.indexCount);
#define forE(index, refvar) for (typeof(refvar) index = 0; index < refvar; index++)

        forE(i, results.vertexCount)
        {
                L("results[%d]: verCoords %f, %f, %f", i, results.ver_texcoords[i].v3.x, results.ver_texcoords[i].v3.y,
                  results.ver_texcoords[i].v3.z);
                L("results[%d]: texCoords %f, %f", i, results.ver_texcoords[i].v2.x, results.ver_texcoords[i].v2.y);
        }

        for (u32 i = 0; i < results.indexCount; i++) {
                L("results[%d]: indices %d, %d, %d", i, results.indices[i].a, results.indices[i].b, results.indices[i].c);
        }

        GLuint ebo;
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(v3v2) * results.vertexCount, results.ver_texcoords, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32_3) * results.indexCount, results.indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(v3v2), (void *)0);
        glEnableVertexAttribArray(0);
        // Find the index of textcoords
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(v3v2), (void *)sizeof(v3));
        glEnableVertexAttribArray(3);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Don't unbind EBO, just leave it bound to this VAO
        //
        // Texture
        GLuint textureRoadId;
        glGenTextures(1, &textureRoadId);
        glBindTexture(GL_TEXTURE_2D, textureRoadId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, roadImg.width, roadImg.height, 0, GL_RGB, GL_UNSIGNED_BYTE, roadImg.data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindVertexArray(0);
        UnloadImage(roadImg); // Once image has been converted to texture and uploaded to VRAM, it can

        bool RoadsInMeters = false;
        LiteMesh *loadedRoadsL = NULL;
        struct renderRoadsArgs renderRoadsArgs = {
            .roadsAsPaths = roadsAsPaths, .RoadsInMeters = &RoadsInMeters, .loadedRoadsLP = &loadedRoadsL, .vao = vao, .vbo = vbo, .ebo = ebo};

        struct minimapPNGsManager *minimapPNGsManager = calloc(sizeof(struct minimapPNGsManager) + 9 * sizeof(struct texZoomTile), 1);
        minimapPNGsManager->nITT = 9;

        struct minimapCTX minimapState = {
            .state = kmmInit, .zoom = 16, .scale = {.45, .45, 0}, .translation = {.75, .75, 0}, .pngManager = minimapPNGsManager};

        while (!WindowShouldClose()) {
                if (IsKeyDown(KEY_P)) {
                        sleep(1);
                        BeginDrawing();
                        EndDrawing();
                        continue;
                }
                renderRoads(&renderRoadsArgs);
                //
                // UpdateCamera(&cam, CAMERA_THIRD_PERSON);
                struct inputArgs iargs = {.cam = &cam};
                modelPosition = (Vector3){cam.target.x, cam.target.y - 1, cam.target.z};
                v3 frontVec = {cam.target.x - cam.position.x, 0, cam.target.z - cam.position.z};
                v3 frontVecV3 = {cam.target.x - cam.position.x, cam.target.y - cam.position.y, cam.target.z - cam.position.z};
                float frontAngle = atan2(frontVec.z, frontVec.x);

                i64i64v3 resp = runningGuyNF(animGirl, &girlModel, &frontVec, &modelPosition);
                cam.target = resp.nextPost;
                cam.target.y = .75;
                // Better first person like camera :)
                cam.target.x += .1 * cos(frontAngle);
                cam.target.z += .1 * sin(frontAngle);
                processInput(iargs);

                i64 currentAnimation = resp.i64i64.a;
                i64 animFrameCounter = resp.i64i64.b;
                v3 nextPos = resp.nextPost;
                var camRadius = processCam(CAMERA_THIRD_PERSON, &cam);
                if (camRadius) { // Skip if camRadius is Zero
                        minimapState.zoom = 18 - camRadius / 200;
                }

                BeginDrawing();
                BeginMode3D(cam);
                // DrawGrid(5000, 1.0f);

                ClearBackground(BACKGROUND_COLOR);

                rlEnableBackfaceCulling();
                rlDisableBackfaceCulling();

                // DrawModel(floorModel, (Vector3){0, 0, 0}, 1, WHITE);
                // glUseProgram(0);
                v3 ballPoint = renderRayAndFloorIntersection(cam.position, frontVecV3, (v3){0}, (v3){0, 1, 0});
                // Draw green sphere in the closest road point to the ballPoint
                v3 closerPoint = (v3){0, 0, 0};
                v3 secondCloserPoint = (v3){0, 0, 0};

                // Check roadsAsPaths is loaded
                if (*roadsAsPaths != NULL) {
                        PathsList minPath = NULL;
                        f32 oldDistance = FLT_MAX;
                        for (u32 i = 0; (*roadsAsPaths)[i] != NULL; i++) {
                                var pathRoad = (*roadsAsPaths)[i];
                                for (u32 j = 0; j < pathRoad->nElements; j++) {
                                        var point = pathRoad->vecs[j];
                                        f32 newDistance = Vector2Distance((v2){point.x, point.z}, (v2){ballPoint.x, ballPoint.z});
                                        if (closerPoint.x <= .001 && closerPoint.y <= .001 && closerPoint.z <= .001) {
                                                secondCloserPoint = point;
                                                closerPoint = point;
                                                minPath = pathRoad;
                                        } else if (newDistance <= oldDistance) {
                                                secondCloserPoint = closerPoint;
                                                closerPoint = point;
                                                minPath = pathRoad;
                                                oldDistance = newDistance;
                                        }
                                }
                        }
                        // Draw the path with the closest point
                        var scale = getenv("SCALE") != null ? atof(getenv("SCALE")) : 1.0;
                        for (u32 i = 0; i < minPath->nElements - 1; i++) {
                                DrawLine3D((Vector3){minPath->vecs[i].x, minPath->vecs[i].y + scale / 10, minPath->vecs[i].z},
                                           (Vector3){minPath->vecs[i + 1].x, minPath->vecs[i + 1].y + scale / 10, minPath->vecs[i + 1].z}, RED);
                        }
                        // closerPoint.x - secondCloserPoint.x < .001 && closerPoint.y == secondCloserPoint.y && closerPoint.z == secondCloserPoint.z
                        if (Vector3Length(Vector3Subtract(closerPoint, secondCloserPoint)) < .001) {
                                DrawSphere((Vector3){closerPoint.x, closerPoint.y, closerPoint.z}, scale / 10, YELLOW);
                                DrawSphere((Vector3){secondCloserPoint.x, secondCloserPoint.y, secondCloserPoint.z}, scale / 10, YELLOW);
                        } else {
                                DrawSphere((Vector3){closerPoint.x, closerPoint.y, closerPoint.z}, scale / 10, GREEN);
                                DrawSphere((Vector3){secondCloserPoint.x, secondCloserPoint.y, secondCloserPoint.z}, scale / 10, BLUE);
                        }
                }

                DrawModel(girlModel, nextPos, .1, WHITE);

                drawProyectiles(GetFrameTime());
                // Shadow
                DrawCylinder((Vector3){modelPosition.x + 1, 0, modelPosition.z}, .25, 0, .01, 90, Fade(RED, .3f));
                DrawCylinder((Vector3){modelPosition.x, 0, modelPosition.z + 1}, .25, 0, .01, 90, Fade(GREEN, .3f));

                // Draw line:
                for (u32 i = 0; i < sizeof(path) / sizeof(v3) - 1 && IsKeyDown(KEY_O); i++) {
                        DrawLine3D((Vector3){path[i].x, 0, path[i].z}, (Vector3){path[i + 1].x, 0, path[i + 1].z}, RED);
                }

                var modelMatrix = MatrixIdentity();
                var viewMatrix = MatrixLookAt(cam.position, cam.target, cam.up);
                var projectionMatrix = MatrixPerspective(cam.fovy * DEG2RAD, (float)GetScreenWidth() / GetScreenHeight(), 0.01f, 5000.0f);
                var mvp = MatrixMultiply(MatrixMultiply(viewMatrix, modelMatrix), projectionMatrix);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, textureRoadId);
                glUseProgram(roadShader.id);
                rlSetUniformMatrix(1, mvp);
                glBindVertexArray(vao);
                glDrawElements(!IsKeyDown(KEY_I) ? GL_TRIANGLES : (glPointSize(10), GL_POINTS),
                               loadedRoadsL == NULL ? results.indexCount * 3 : loadedRoadsL->indexCount * 3, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
                EndMode3D();
                char *uiMessage[40] = {0};
                v2 latLong = WorldCoordsToLatLong(modelPosition.x, modelPosition.z, (v2){PointOfOrigin.x, PointOfOrigin.z});
                asprintf(&uiMessage[0], " Current position in Lat: %f, Long: %f ", latLong.x, latLong.y);
                asprintf(&uiMessage[1], " Current position in opengl units: %f, Y: %f, Z: %f ", cam.target.x, cam.target.y, cam.target.z);
                asprintf(&uiMessage[2], " Model position: %f, Y: %f, Z: %f ", modelPosition.x, modelPosition.y, modelPosition.z);
                asprintf(&uiMessage[3], " Camera position: %f, Y: %f, Z: %f ", cam.position.x, cam.position.y, cam.position.z);
                asprintf(&uiMessage[4], " Camera target: %f, Y: %f, Z: %f ", cam.target.x, cam.target.y, cam.target.z);
                for (int i = 0; i < 6; i++) {
                        DrawText(uiMessage[i], 10, 20 + 20 * i, 20, (Color){255, 255, 255, 120});
                        free(uiMessage[i]);
                }

                if (*roadsAsPaths == null) {
                        DrawText(TextFormat("Loading Roads %*.s (%.2f ms elapsed)", ((int)GetTime()) % 4 + 1, "....", GetTime()),
                                 GetScreenWidth() * 2 / 3, 10, 20, (Color){255, 255, 255, 120});
                } else if (!RoadsInMeters) {
                        DrawText("Press L to load the roads", GetScreenWidth() * 2 / 3, 10, 20, (Color){255, 255, 255, 120});
                } else {
                        DrawText("Press P to pause", GetScreenWidth() * 2 / 3, 10, 20, (Color){255, 255, 255, 120});
                        // Bottom of screen "Press C to copy the current position to the clipboard"
                        DrawText("Press C to copy the current position to the clipboard", GetScreenWidth() * 2 / 3, GetScreenHeight() - 20, 20,
                                 (Color){255, 255, 255, 120});
                }
                if (IsKeyDown(KEY_C)) {
                        char *clipBoard = NULL;
                        asprintf(&clipBoard, "%f , %f", latLong.x, latLong.y);
                        SetClipboardText(clipBoard);
                        free(clipBoard);
                }
                DrawText(TextFormat("\nCloser point: %f, %f, %f", closerPoint.x, closerPoint.y, closerPoint.z), GetScreenWidth() * 2 / 3, 10, 20,
                         (Color){255, 255, 255, 120});
                renderMinimap(&minimapState, &modelPosition, &frontVec, &PointOfOrigin);
                EndDrawing();

                thrd_yield();
        }
        // fclose(f);
        CloseWindow();
        return 1;
}
void parse_options(int argc, char *argv[])
{
        int opt;
        while ((opt = getopt(argc, argv, "hl")) != -1) {
                switch (opt) {
                case 'h':
                        printf("Usage: %s [OPTION]...\n", argv[0]);
                        printf("  -h\t\tShow this help\n");
                        exit(0);
                        break;

                case 'l': // log level
                        SetTraceLogLevel(LOG_DEBUG);
                        break;

                default: /* '?' */
                        fprintf(stderr, "Usage: %s [OPTION]...\n", argv[0]);
                        fprintf(stderr, "Try '%s -h' for more information.\n", argv[0]);
                        exit(EXIT_FAILURE);
                }
        }
}

// read command output
//  TODO: This is for not malicius commands, if you want to use this
//  for something else, you should check the command first
char *read_command(char *const fmt)
{
        FILE *fp;
        fp = popen(fmt, "r");
        if (fp == NULL) {
                Le("Pipe failed");
                return NULL;
        }
        L("Reading pipe");
        // Read pipe in chunks of 1024 bytes
        i64 length = 0;
        char c[1024] = {0};
        char *buffer = malloc(0);
        while (fgets(c, sizeof(c), fp) != NULL) {
                length += strlen(c);
                buffer = realloc(buffer, length + 1);
                if (buffer == NULL) {
                        Le("realloc failed");
                        return NULL;
                }
                if (length == (int)strlen(c))
                        strcpy(buffer, c);
                else
                        strcat(buffer, c);
        }
        pclose(fp);

        return buffer;
}

bool areNodesSorted(struct node *nodes)
{
        for (int i = 1; nodes[i].id != 0; i++) {
                if (nodes[i].id < nodes[i - 1].id)
                        return false;
        }
        return true;
}
i64i64 printElements(struct elements el)
{

        L("MAIN:Nodes: %lu", el.nodesCount);
        L("MAIN:Ways: %lu", el.waysCount);
        L("MAIN:are nodes sorted? %s", areNodesSorted(el.nodes) ? "yes" : "no");
        i64i64 resp = {.a = el.nodesCount, .b = el.waysCount};
        return resp;
}

int compareNodes(const void *a, const void *b, __attribute((unused)) void *arg)
{
        struct node *nodeA = (struct node *)a;
        struct node *nodeB = (struct node *)b;
        if (nodeA->id < nodeB->id)
                return -1;
        else if (nodeA->id > nodeB->id)
                return 1;
        else
                return 0;
}
int compareWayNodes(const void *a, void *b)
{
        struct node *nodeA = (struct node *)a;
        struct node *nodeB = (struct node *)b;
        if (nodeA->id < nodeB->id)
                return -1;
        else if (nodeA->id > nodeB->id)
                return 1;
        else
                return 0;
}
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
        assert(window != NULL);
        // make sure the viewport matches the new window dimensions; note that width and
        // height will be significantly larger than specified on retina displays.
        glViewport(0, 0, width, height);
}
void _raylibInit()
{
#ifndef DEBUG
        SetTraceLogLevel(LOG_NONE);
#endif
        prev();
        SetTargetFPS(30);
        SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
        InitWindow(width, height, "gamev");
        glEnable(GL_MULTISAMPLE);
}

i64i64v3 runningGuyNF(struct ModelAnimation *anims, Model *model, v3 *frontVec, v3 *currentPos)
{
        enum Animations { crouch = 0, idle = 1, run = 2, Tpose = 3, walk = 4 };
        static int animFrameCounter = 0;
        static bool __attribute__((unused)) pauseAnimation = false;
        static enum Animations currentAnimation = idle;
        // First frame animation
        static bool firstFrame = true;
        static Matrix transform = {0};
        static v3 modelPosition = {0, 0, 0};
        if (firstFrame) {
                transform = MatrixMultiply(model->transform, MatrixRotateY(90 * DEG2RAD));
                modelPosition = *currentPos;
                firstFrame = false;
        }
        static float speed = 1.0f;
        float rotationAngle = atan2(frontVec->z, frontVec->x) * RAD2DEG;
        int orientation = 1;
        bool isMoving = IsKeyDown(KEY_W) || IsKeyDown(KEY_A) || IsKeyDown(KEY_D) || IsKeyDown(KEY_S);

        rotationAngle += (IsKeyDown(KEY_D) - IsKeyDown(KEY_A)) * (45.0f * (!IsKeyDown(KEY_W) + 1) + 240 * IsKeyDown(KEY_S));

        v3 cummulativeMovement = {0};
        switch (currentAnimation) {
        case idle:
                speed = 0.0f;
                if (isMoving) {
                        currentAnimation = walk;
                        animFrameCounter = 0;
                        break;
                }
                break;
        case walk:
                speed = 1.0f;
                if (IsKeyDown(KEY_S)) {
                        orientation = -1;
                } else
                        orientation = 1;

                cummulativeMovement.x = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
                cummulativeMovement.z = IsKeyDown(KEY_W) - IsKeyDown(KEY_S);

                if (!isMoving) {
                        currentAnimation = idle;
                        orientation = 1;
                        animFrameCounter = 0;
                        break;
                } else if (orientation == 1 && IsKeyDown(KEY_LEFT_SHIFT)) {
                        currentAnimation = run;
                        animFrameCounter = 0;
                        break;
                }

                break;
        case run:
                cummulativeMovement.x = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
                cummulativeMovement.z = IsKeyDown(KEY_W) - IsKeyDown(KEY_S);
                speed = 200.0f;

                if (IsKeyDown(KEY_S)) {
                        currentAnimation = walk;
                        animFrameCounter = 0;
                        break;
                }
                if (IsKeyUp(KEY_LEFT_SHIFT)) {
                        currentAnimation = walk;
                        animFrameCounter = 0;
                }

                if (!isMoving) {
                        currentAnimation = idle;
                        animFrameCounter = 0;
                        break;
                }
                break;
        default:
                currentAnimation = Tpose;
                break;
        }
        // Next Frame animation
        UpdateModelAnimation(*model, anims[currentAnimation], animFrameCounter);
        animFrameCounter = abs_mod(animFrameCounter + orientation, anims[currentAnimation].frameCount);

        // Model rotation
        // Formward movemest is frontVec
        // we will use the cross product to get the right vector
        v3 rightVec = Vector3CrossProduct(*frontVec, (v3){0, 1, 0});
        v3 totalMovement =
            Vector3Normalize(Vector3Add(Vector3Scale(*frontVec, cummulativeMovement.z), Vector3Scale(rightVec, cummulativeMovement.x)));
        v3 modelNextPos = Vector3Scale(totalMovement, speed * GetFrameTime());
        modelPosition = Vector3Add(modelPosition, modelNextPos);

        model->transform = MatrixMultiply(transform, MatrixRotateY(-rotationAngle * DEG2RAD));

        return (i64i64v3){.i64i64.a = currentAnimation, .i64i64.b = animFrameCounter, .v3 = *frontVec, .nextPost = modelPosition};
}

v3 sphericalToCartesian(float radius, Radian pitchAngle, Radian yawAngle, v3 *center)
{
        v3 resp = {0};
        resp.x = center->x + radius * cos(pitchAngle) * cos(yawAngle);
        resp.y = center->y + radius * sin(pitchAngle);
        resp.z = center->z + radius * cos(pitchAngle) * sin(yawAngle);
        return resp;
}
// Process camera to rotate aroung the cam->target (distance 10 units)
// Returns the distance from the camera to the target
f32 processCam(CameraMode cm, Camera *cam)
{
        if (cm != CAMERA_THIRD_PERSON)
                return 0;
        // Factors
        const float pitchFactor = 2;
        const float yawFactor = 2;
        const float maxRadius = 1000;
        const float minRadius = .01;
        static float radius = 3;
        radius -= GetMouseWheelMove() * (IsKeyDown(KEY_LEFT_ALT) ? .1 : IsKeyDown(KEY_LEFT_CONTROL) * (maxRadius - radius) * 2 / 10 + 0.5);
        radius = fminf(fmaxf(radius, minRadius), maxRadius);
        const float deltaTime = GetFrameTime();
        // Globals
        static bool firstFrame = true;
        static float pitchAngle = 0;
        static float yawAngle = 0;
        if (firstFrame) {
                firstFrame = false;
        }
        // Get Input
        v2 MousePitchYaw = GetMouseDelta();
        MousePitchYaw = (MousePitchYaw.y == .0f && MousePitchYaw.x == .0f)
                            ? (v2){(-IsKeyDown(KEY_LEFT) + IsKeyDown(KEY_RIGHT)) * 30, (-IsKeyDown(KEY_UP) + IsKeyDown(KEY_DOWN)) * 30}
                            : MousePitchYaw;

        pitchAngle += MousePitchYaw.y * pitchFactor * deltaTime;
        yawAngle += MousePitchYaw.x * yawFactor * deltaTime;
        v3 *center = &cam->target;
        pitchAngle = fmax(fmin(fmod(pitchAngle, 180), 80), -80);
        // Spherical to cartesian
        v3 camPosition = sphericalToCartesian(radius, pitchAngle * DEG2RAD, yawAngle * DEG2RAD, center);

        // Crop camera position
        camPosition.y = fmax(camPosition.y, 0.1);
        cam->position = camPosition;
        return radius;
}

v3v3 intersecRayPlane(v3 rayOrigin, v3 rayDirection, v3 planeOrigin, v3 planeNormal)
{
        float denom = Vector3DotProduct(planeNormal, rayDirection);
        if (denom < -1e-6) {
                v3 p0l0 = Vector3Subtract(planeOrigin, rayOrigin);
                float t = Vector3DotProduct(p0l0, planeNormal) / denom;
                return (v3v3){.lineStart = rayOrigin, .lineEnd = Vector3Add(rayOrigin, Vector3Scale(rayDirection, t))};
        }
        return (v3v3){.lineStart = (v3){0}, .lineEnd = (v3){0}};
}

int renderRoads(struct renderRoadsArgs *args)
{
        var roadsAsPaths = args->roadsAsPaths;
        var RoadsInMetersP = args->RoadsInMeters;
        var loadedRoadsLP = args->loadedRoadsLP;
        var vao = args->vao;
        var vbo = args->vbo;
        var ebo = args->ebo;

        if (*roadsAsPaths != NULL && !(*RoadsInMetersP)) {
                L("------------- Now We will generate the vertexes");
                thrd_t roadVertexGenThread;
                struct {
                        PathsList *pathList;
                        LiteMesh **meshesAP;
                } roadVertexGenArgs = {.pathList = *roadsAsPaths, .meshesAP = loadedRoadsLP};
                /// THe roads in meters :
                // for (u32 i = 0; (*roadsAsPaths)[i] != NULL; i++) {
                //         var pathRoad = (*roadsAsPaths)[i];
                //         Ld("Path %d: %p\n", i, pathRoad);
                //         for (u32 j = 0; j < pathRoad->nElements; j++) {
                //                 Ld("\tPoint %d: %f, %f, %f\n", j, pathRoad->vecs[j].x, pathRoad->vecs[j].y, pathRoad->vecs[j].z);
                //         }
                // }

                var timer = clock();
                thrd_create(&roadVertexGenThread, roadVertexGen, &roadVertexGenArgs);
                *RoadsInMetersP = true;
                thrd_join(roadVertexGenThread, NULL);
                L("----- Time road to vertexes: %fms", (float)(clock() - timer) / CLOCKS_PER_SEC * 1000);
                timer = clock();
                // Now LiteMesh is loaded
                //  we upload it to the GPU
                // Save to file the mesh
                Ld("LoadedRoadsL.indiceCount: %d\n", (*loadedRoadsLP)->indexCount);
                Ld("LoadedRoadsL.vertexCount: %d\n", (*loadedRoadsLP)->vertexCount);
                timer = clock();
                glBindVertexArray(vao);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, sizeof(v3v2) * (*loadedRoadsLP)->vertexCount, (*loadedRoadsLP)->ver_texcoords, GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32_3) * (*loadedRoadsLP)->indexCount, (*loadedRoadsLP)->indices, GL_STATIC_DRAW);
                glBindVertexArray(0);
                L("-----Time to upload to GPU: %fms ", (float)(clock() - timer) / CLOCKS_PER_SEC * 1000);
        }
        return 0;
}
