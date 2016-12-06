
struct My_node{
    // char* name;  // My node name
    // char* port;
    char name[256];  // My node name
    char port[256];
};
struct Neighbor{
    char name[256];
    char cost[256];
    char address[256];
};
struct DV_Element {
    char dest[256];
    int dist;
};
struct Distance_vector {
    char sender[256];
    int num_of_dests;
    struct DV_Element element[256];
};
struct RT_element {
    char node[256];
    int dist;
    char next_hop[256];
};
struct Routing_table {
    char node[256];
    int num_rows;
    struct RT_element element[256];
};
struct Config_element {
    char node[256];
    char dist[256];
    char address[256];
};
struct Parsed_config {
    char node[256];
    char port[256];
    int num_rows;
    struct Config_element element[256];
};