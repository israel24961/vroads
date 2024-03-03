#include <main.h>
#include <raylib.h>
/**
 * @brief I wish to turn a way (with width and level) into
 * the vertex and texture coordinates of the path
 *
 * Right now,
 * A way turns in to a list of nodes(1) and then into a list of
 * vertex and texture coordinates(2)
 * Step (1) destroys the way's width information,
 * hence (1) and (2) should be combined into a single step?
 * * @return int
 */
int main()
{
        struct way ws = {};
        struct node nodes[4] = {(struct node){.id = 1, .lat = 0, .lon = 0}, (struct node){.id = 2, .lat = 0, .lon = 5},
                                (struct node){.id = 3, .lat = 8, .lon = 5}, (struct node){.id = 4, .lat = 8, .lon = 10}};
        ws.id = 1;
        ws.level = 2;
        ws.nodes = nodes;
        ws.nodesCount = 4;
        var e = WayToCoordsPaths(&ws);
        var v0 = Vector3Length(Vector3Subtract(e->vecs[0], (Vector3){1, ws.level, 0}));
        if (v0 > 0.01)
                Le("v0!=%f", v0);
        var v1 = Vector3Length(Vector3Subtract(e->vecs[1], (Vector3){1, ws.level, 5}));
        if (v0 > 0.01)
                Le("v0!=%f", v0);
        var v2 = Vector3Length(Vector3Subtract(e->vecs[2], (Vector3){8, ws.level, 5}));
        if (v0 > 0.01)
                Le("v0!=%f", v0);
        var v3 = Vector3Length(Vector3Subtract(e->vecs[3], (Vector3){8, ws.level, 10}));
        if (v0 > 0.01)
                Le("v0!=%f", v0);
        if (e->nElements!=ws.nodesCount)
                Le("e->nElements!=ws.nodesCount");

        if (v0 > .01 || v1 > .01 || v2 > .01 || v3 > .01 || e->nElements!=ws.nodesCount)
                return 1;
        return 0;
}

