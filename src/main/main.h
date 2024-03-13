#pragma once
#include <0_GlobalIncludes.h>
float tic;
float toc;
#include "overpassQueries.h"

#define ISNULL(X, FMT, ...)                                                                                                                          \
        if (X == 0) {                                                                                                                                \
                Le(FMT __VA_OPT__(, ) __VA_ARGS__);                                                                                                  \
                assert(0);                                                                                                                           \
        }

#define Z(X)                                                                                                                                         \
        (X) { 0 }
#define ZE(X)                                                                                                                                        \
        (struct X) {}

double deg2rad(double deg) { return (deg * M_PI / 180); }
double haversine_distance(double lat1, double lon1, double lat2, double lon2)
{
        lat1 = deg2rad(lat1);
        lon1 = deg2rad(lon1);
        lat2 = deg2rad(lat2);
        lon2 = deg2rad(lon2);

        double dlat = lat2 - lat1;
        double dlon = lon2 - lon1;

        double a = sin(dlat / 2) * sin(dlat / 2) + cos(lat1) * cos(lat2) * sin(dlon / 2) * sin(dlon / 2);
        double c = 2 * atan2(sqrt(a), sqrt(1 - a));

        return 6371 * 1000 * c;
}
struct g_C {
        Vector2 center;
        float radius;
};
struct g_L {
        Vector2 p1;
        Vector2 p2;
};
#include <math.h>
#include <raymath.h>
#define sign(x) ((x) >= 0 ? 1 : ((x) < 0 ? -1 : 0))
#define typef(X) ((typeof(__FUNCTION__))X)
struct as0 {
        v2 p1, p2;
        double deter;
};
struct as0 IslineInsideCeroOrigin(struct g_C circ, v2 pA, v2 pB)
{
        // typeof(IslineInsideCeroOrigin(ZE(g_C), Z(v2), Z(v2))) ret ;
        double D = pA.x * pB.y - pA.y * pB.x;
        double dx = pB.x - pA.x;
        double dy = pB.y - pA.y;
        double dr = sqrt(pow(dx, 2) + pow(dy, 2));
        L("dx=%f,dy=%f,dr=%f\n", dx, dy, dr);
        double r = circ.radius;
        double discriminant = pow(r, 2) * pow(dr, 2) - pow(D, 2);
        double x = D * dy + sign(dy) * dx * sqrt(discriminant);
        x = x / pow(dr, 2);
        double y = -D * dx + fabs(dy) * sqrt(pow(circ.radius, 2) * pow(dr, 2) - pow(D, 2));
        y = y / pow(dr, 2);

        double x0 = D * dy - sign(dy) * dx * sqrt(discriminant);
        x0 = x0 / pow(dr, 2);
        L("X=%lf Y=%lf %lf deter=%lf %lf\n", x, y, -circ.radius, discriminant, sqrt(pow(circ.radius, 2) * pow(dr, 2) - pow(D, 2)));
        double y0 = -D * dx - fabs(dy) * sqrt(pow(circ.radius, 2) * pow(dr, 2) - pow(D, 2));
        y0 = y0 / pow(dr, 2);
        return (struct as0){{x, y}, {x0, y0}, discriminant};
}

/// @brief Returns the intersection points of a line and a circle
/// @return struct a0 {v2 p1, p2, double deter}
/// where p1 and p2 are the intersection points and deter is the determinant
/// of the line and the circle.
/// If the determinant is negative, the line does not intersect the circle.
/// If the determinant is zero, the line is tangent to the circle.
/// If the determinant is positive, the line intersects the circle in two
/// points.
struct as0 IsLineInside(struct g_C circ, v2 pA, v2 pB)
{
        struct as0 res = IslineInsideCeroOrigin(circ, Vector2Subtract(pA, circ.center), Vector2Subtract(pB, circ.center));
        res.p1.x += circ.center.x;
        res.p1.y += circ.center.y;
        res.p2.x += circ.center.x;
        res.p2.y += circ.center.y;
        return res;
        // return (struct a0){0};
}
int width = 800, height = 450;
__always_inline float vw(float x) { return x * width; }
__always_inline float vh(float x) { return x * height; }

void DRAWC(struct g_C X, Color COL) { DrawCircle(vw(X.center.x), vh(X.center.y), vh(X.radius), COL); }
void DRAWCL(struct g_C X, Color COL) { DrawCircleLines(vw(X.center.x), vh(X.center.y), vh(X.radius), COL); }
void DRAWL(v2 p1, v2 p2, Color COL) { DrawLine(vw(p1.x), vh(p1.y), vw(p2.x), vh(p2.y), COL); }

void parse_options(int argc, char *argv[]);

char *read_command(char *const fmt);

bool areNodesSorted(struct node *nodes);
i64i64 printElements(struct elements el);
int compareNodes(const void *a, const void *b, void *arg);
int compareWays(const void *a, const void *b);

void _raylibInit();

typedef struct {
        v3 lineStart;
        v3 lineEnd;
} v3v3;
typedef struct v2v2 {
        v2 lineStart;
        v2 lineEnd;
} v2v2;
bool v2v2Equals(v2v2 *restrict a, v2v2 *restrict b);
v3v3 intersecRayPlane(v3 rayOrigin, v3 rayDirection, v3 planeOrigin, v3 planeNormal);
typedef struct {
        v3 *vecs;
        i64 nElements;
} pathPoints;

pathPoints *WayToCoordsPaths(struct way *w) __attribute__((malloc));
