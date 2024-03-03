#include "overpassQueries.h"
#include "fileOperations.h"
#include <log.h>
#include <curl/curl.h>
#include <errno.h>
#include <json-c/json.h>

struct elements json_parse(su64 data);
su64 query_data(const char *msg);
f32 way_getLevel(json_object *element);
f32 way_getWidth(json_object *element);
u64 launchQuery(OverpassQuery *);
static int compareNodes(const void *a, const void *b);

// Main stuff
OverpassQuery *OverpassQuery_new(char *name, char *query, char *url)
{
        OverpassQuery *this = malloc(sizeof(OverpassQuery));
        this->name = name;
        this->query = query;
        this->url = url == NULL ? "https://overpass-api.de/api/interpreter" : url;
        this->output = (su64){0};
        this->elements = (struct elements){0};
        // TODO: use url in launchQuery
        this->launchQuery = launchQuery;
        return this;
}

// Curl request to OSM overpass, using json-c library to then parse the response
// Query data from overpass api,
// msg is the query string if online mode (OFFLINE environment variable not set)
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
su64 query_data(const char *msg)
{
        // Offline mode
        if (getenv("OFFLINE")) {
                L("Querying data, offline mode, from <%s>", getenv("OFFLINE"));

                FILE *f1 = fopen(getenv("OFFLINE"), "r");
                if (!f1) {
                        L("%s", strerror(errno));
                        exit(1);
                }
                char *data = NULL;
                size_t len;
                ssize_t bytes_read = getdelim(&data, &len, '\0', f1);
                fclose(f1);
                bool gt1000 = bytes_read > 1000L;
                bool gt1000_000 = bytes_read > 1000000L;
                f32 humanSize = gt1000_000 ? bytes_read / 1000000.0 : gt1000 ? bytes_read / 1000.0 : bytes_read;
                char *humanUnit = gt1000_000 ? "MB" : gt1000 ? "kB" : "B";
                L("The file is  %.2f %s", humanSize, humanUnit);
                return (su64){.data = (u8*)data, .size = len};
        }
        // Online mode
        CURL *hnd = curl_easy_init();
        curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
        curl_easy_setopt(hnd, CURLOPT_URL, "https://lz4.overpass-api.de/api/interpreter");
        curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, msg);
        curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE,
                         (curl_off_t)strlen(msg)); // Security?
        curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/8.0.1");
        curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
        curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(hnd, CURLOPT_TIMEOUT, 300L);

        su64 dmem = {.data = malloc(0), .size = 0};
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &dmem);

        // get time of the curl request
        float perf_0 = clock();
        CURLcode ret = curl_easy_perform(hnd);
        float perf_1 = clock();
        if (ret) {
                Le("Query to overpass returned: %i, msg: <%s>", ret, dmem.size > 1000 ? "<DATA2BIG>" : (char*)dmem.data);
                free(dmem.data);
                dmem = (su64){0};
                goto end;
        }
        if (dmem.data[dmem.size]) {
                Le("These guys tried to overflow me, but I'm too smart for them #cringe");
                free(dmem.data);
                dmem = (su64){0};
                goto end;
        }
        L("Received %ld kB in %fms", dmem.size / 1000, (perf_1 - perf_0) * 1000 / CLOCKS_PER_SEC);
        // Start thread to save the data to a file
        struct saveToFileArgs *args = malloc(sizeof(struct saveToFileArgs));
        *args = (struct saveToFileArgs){.filename = "data.json", .content = dmem};
        saveToFileAsync(args);
end:
        curl_easy_cleanup(hnd);
        return dmem;
}

struct node *node_by_id(u64 id, struct node *nodes, int nodesCount);
struct elements json_parse(su64 data)
{
        json_object *jobj = json_tokener_parse(data.datav);
#define _initialNodesSize 2048
        struct node *all_nodes = calloc(_initialNodesSize, sizeof(struct node));
#define _initialWaysSize 1024
        struct way *ways = calloc(_initialWaysSize, sizeof(struct way));

        // Iterate over the json fields, the data is in the "elements" field
        // which is an array of objects, where the nodes are first and
        // the ways are after the last node
        json_object *elements;
        json_object_object_get_ex(jobj, "elements", &elements);
        if (!elements) {
                Le("No elements in json");
                goto errorbye;
        }
        int len = json_object_array_length(elements);
        L("\tElements: %d", len);
        int max_nodes = _initialNodesSize;
        float nodes_perf_0 = clock();
        for (int i = 0; i < len; i++) {
                json_object *element = json_object_array_get_idx(elements, i);
                json_object *type;
                json_object_object_get_ex(element, "type", &type);
                if (json_object_get_string(type)[0] == 'n') {
                        // Node
                        json_object *lat, *lon;
                        json_object_object_get_ex(element, "lat", &lat);
                        json_object_object_get_ex(element, "lon", &lon);
                        // Ld("Node %lu: (%s, %s)",
                        //    json_object_get_int64(json_object_object_get(element, "id")),
                        //    json_object_get_string(lat), json_object_get_string(lon));
                        // Add node to nodes
                        if (i >= max_nodes) { // one less for search purposes
                                max_nodes *= 1.5;
                                Ld("reallocating nodes %d", i);
                                all_nodes = realloc(all_nodes, max_nodes * sizeof(struct node));
                        }
                        json_object *tjo;
                        if (!json_object_object_get_ex(element, "id", &tjo)) {
                                goto errorbye;
                        }

                        all_nodes[i] =
                            (struct node){.id = json_object_get_int64(tjo), .lat = json_object_get_double(lat), .lon = json_object_get_double(lon)};
                }
                if (json_object_get_string(type)[0] == 'w') {
                        all_nodes[i] = (struct node){0};
                        max_nodes = i;
                        break;
                }
        }
        float nodes_perf_1 = clock();
        L("\t %i Nodes perf: %fms [%f us per node]", max_nodes, (nodes_perf_1 - nodes_perf_0) * 1000 / CLOCKS_PER_SEC,
          (nodes_perf_1 - nodes_perf_0) * 1000000 / CLOCKS_PER_SEC / max_nodes);
        // sort nodes
        //  qsort(all_nodes, max_nodes, sizeof(struct node), compareNodes);
        //  assert(

        nodes_perf_0 = clock();
        if (len - max_nodes > _initialWaysSize) {
                // Ld("reallocating ways %d", len - max_nodes);
                ways = realloc(ways, (len - max_nodes + 1) * sizeof(struct way));
                ways[len - max_nodes] = (struct way){0};
        }
        for (int i = 0; i < len - max_nodes; i++) {
                json_object *element = json_object_array_get_idx(elements, i + max_nodes);
                // json_object *type;
                // json_object_object_get_ex(element, "type", &type);
                json_object *way_nodes;
                json_object_object_get_ex(element, "nodes", &way_nodes);
                int nodes_len = json_object_array_length(way_nodes);
                // Ld("Way in string: %s with %u nodes",
                // json_object_to_json_string(element),
                //    nodes_len);
                // Add way to ways
                json_object *tjo;
                if (!json_object_object_get_ex(element, "id", &tjo)) {
                        goto errorbye;
                }
                // Tags
                // f32 layer = way_getLevel(element);
                f32 layer = way_getLevel(element);
                // __attribute__((unused))
                // f32 width = way_getWidth(element);

                ways[i] = (struct way){
                    .id = json_object_get_int64(tjo), .nodes = calloc(nodes_len + 1, sizeof(struct node)), .nodesCount = nodes_len, .level = layer};

                // Ld("Way: '%s' - %lu, nodes:", json_object_get_string(tjo), ways[i].id);
                for (int j = 0; j < nodes_len; j++) {
                        json_object *node = json_object_array_get_idx(way_nodes, j);
                        u64 id = json_object_get_int64(node);
                        struct node *foundNode = node_by_id(id, all_nodes, max_nodes);
                        if (!foundNode) {
                                Le("Node %lu not found", id);
                                exit(1);
                        }
                        ways[i].nodes[j] = *foundNode;
                }
        }
        nodes_perf_1 = clock();
        L("\t %i Ways perf: %fms [%f us per way]", len - max_nodes, (nodes_perf_1 - nodes_perf_0) * 1000 / CLOCKS_PER_SEC,
          (nodes_perf_1 - nodes_perf_0) * 1000000 / CLOCKS_PER_SEC / (len - max_nodes));
        // Clean json object
        json_object_put(jobj);
        return (struct elements){.nodes = all_nodes, .ways = ways, .nodesCount = max_nodes, .waysCount = len - max_nodes};
errorbye:
        json_object_put(jobj);
        free(all_nodes);
        // free ways
        for (int i = 0; ways[i].id != 0; i++) {
                free(ways[i].nodes);
        }
        free(ways);
        return (struct elements){.nodes = NULL, .ways = NULL};
}
static int compareNodes(const void *a, const void *b)
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
//
// @brief Get the distance between two points in meters
// @param nodes Array of nodes, should end with an empty node(id = 0)
//
struct node *node_by_id(u64 id, struct node *nodes, int nodesCount)
{
        struct node key = {.id = id};
        struct node *res = bsearch(&key, nodes, nodesCount, sizeof(struct node), compareNodes);
        // for (int i = 0; nodes[i].id != 0; i++) {
        //         if (nodes[i].id == id)
        //                 return &nodes[i];
        // }
        return res;
}
// Returns the size of the data in bytes
u64 launchQuery(OverpassQuery *this)
{
        char *query = this->query;
        su64 dmem = query_data(query);
        if (dmem.size == 0) {
                Le("No data received");
                return 0;
        }
        struct elements el = json_parse(dmem);
        this->elements = el;
        this->output = dmem;
        return dmem.size;
}
#include <signal.h>
#include <stdarg.h>
f32 way_getLevel(json_object *element)
{
        //(bridge!=no && layer<0 && location!=underground && indoor!=yes)
        json_object *tags;
        json_object_object_get_ex(element, "tags", &tags);
        if (!tags) {
                return 0;
        }

        bool isBridge = false; // tag.bridge
        json_object *bridgeTag;
        if (json_object_object_get_ex(tags, "bridge", &bridgeTag)) {
                isBridge = json_object_get_boolean(bridgeTag);
        }
        //
        i64 layer = 0; // tag.layer
        json_object *layerTag;
        if (json_object_object_get_ex(tags, "layer", &layerTag)) {
                layer = json_object_get_int(layerTag);
        }

        i64 level = 0; // tag.level
        json_object *levelTag;
        if (json_object_object_get_ex(tags, "level", &levelTag)) {
                level = json_object_get_int(levelTag);
        }
        bool isUnderground = false; // tag.location="underground"
        json_object *locationTag;
        if (json_object_object_get_ex(tags, "location", &locationTag)) {
                isUnderground = strncmp(json_object_get_string(locationTag), "underground", sizeof("underground")) == 0;
        }
        bool isTunnel = false; // tag.tunnel
        const char *tunnelDesc;
        json_object *tunnelTag;
        if (json_object_object_get_ex(tags, "tunnel", &tunnelTag)) {
                isTunnel = json_object_get_boolean(tunnelTag);
                tunnelDesc = json_object_get_string(tunnelTag);
        }
        bool isCovered = false;
        json_object *coveredTag;
        if (json_object_object_get_ex(tags, "covered", &coveredTag)) {
                isCovered = json_object_get_boolean(coveredTag);
        }

        // We shouldn't care about covered ways
        // bool isCovered = false; // tag.Covered
        // json_object *coveredTag;
        // if (json_object_object_get_ex(tags, "covered", &coveredTag)) {
        //         isCovered = json_object_get_boolean(coveredTag);
        // }

        // logic
        if (isBridge) {
                return 2;
        }
        // If it's not a tunnel:building_passage and isCovered, let's put it at -.5
        if (!isTunnel) {
                if (isCovered && strncmp(tunnelDesc, "building_passage", sizeof("building_passage")) == 0)
                        return -.7;
                if (-.1 < layer && layer <= .1) {
                        return .5;
                }
                return layer;
        } else if (isTunnel) {
                if (-.1 < layer && layer <= .1) {
                        return level;
                }
                return (-.1 < level && level < .1) ? layer : level;
        } else if (level < 0 && !isUnderground) {
                return -1;
        }
        return 0;
}
/// @brief Get the width of a way from the kind of highway and lanes
f32 way_getWidth(__attribute__((unused)) json_object *element)
{
        // tags.width is very rare
        return .0f;
}
