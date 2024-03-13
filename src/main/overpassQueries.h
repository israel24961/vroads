#pragma once
#include "0_GlobalIncludes.h"

struct node {
        u64 id;
        double lat;
        double lon;
};
struct way {
        u64 id;
        i64 nodesCount;
        struct node *nodes;
        i64 level;
};
struct elements {
        struct node *nodes;
        struct way *ways;
        u64 nodesCount;
        u64 waysCount;
};

// Main object
typedef struct OverpassQuery {
        char *name;                                     /*this is just a name for this object*/
        char *query;                                    /*query to the overpass server*/
        char *url;                                      /*url to the overpass server*/
        su64 output;                                    /* output format in json*/
        struct elements elements;                       /*elements returned by the query*/
        u64 (*launchQuery)(struct OverpassQuery *self); /* launch the query*/
} OverpassQuery;

/// @brief creates a new OverpassQuery object
/// @param name name of the object
/// @param query query to the overpass server
/// @param url url to the overpass server, if NULL, a default url is used
OverpassQuery *OverpassQuery_new(char *name, char *query, char *url);
v2 WorldCoordsToLatLong(float lat, float lon, v2 pointOfOrigin);
f64 LatLongToMeters(f32 lat0, f32 lon0, f32 lat1, f32 lon1);
v2 LatLongToWorldDeltas(v3 pointOfOrigin);
