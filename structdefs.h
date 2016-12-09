// struct My_node{
//     char name[6];  
//     char port[6];
// };
// struct Neighbor{
//     char name[6];
//     char cost[6];
//     char address[6];
// };
struct DV_Element {
    char dest[6];
    int dist;
};
struct Distance_vector {
    char sender[6];
    int num_of_dests;
    struct DV_Element element[8];
};
struct RT_element {
    char node[6];
    int dist;
    char next_hop[6];
};
struct Routing_table {
    char node[6];
    int num_rows;
    struct RT_element element[8];
};
struct Config_element {
    char node[6];
    int dist;
    char address[16];
};
struct Parsed_config {
    char node[6];
    char port[6];
    int num_rows;
    struct Config_element element[8];
};