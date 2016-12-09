// Helper functions for client server
// Functions have been tested in sandbox 
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structdefs.h"
struct Parsed_config parse_config(){
    /*
        Read config file to create initial routing table
    */
    // printf("\nTesting making init routing table from config file");
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    // struct My_node my_node;
    // struct Neighbor n1;
    struct Parsed_config parsed_config;

    fp = fopen("./config", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    int line_num = 1;
    while ((read = getline(&line, &len, fp)) != -1) {
        // printf("Retrieved line of length %zu :\n", read);
        // printf("%s", line);

        char str[256];
        strncpy(str,line,sizeof str-1);
        str[255] = '\0';
        char *token = NULL;
        int n_tokens = 0;

       // Split the string",  into tokens delimited by spaces and commas
       token = strtok (str," ,");
       while (token != NULL)
       {
            // printf("%s\n", token);
            if(line_num==1){
                strncpy(parsed_config.node,token,sizeof parsed_config.node-1);
                parsed_config.node[255] = '\0';
            }
            else if(line_num==2){
                strncpy(parsed_config.port,token,sizeof parsed_config.port-1);
                parsed_config.port[255] = '\0';
            }
            else{
                if(n_tokens==0){
                    strncpy(parsed_config.element[line_num-3].node,token,sizeof parsed_config.element[line_num-3].node-1);
                    // n1.name[255] = '\0';
                }
                if(n_tokens==1){
                    strncpy(parsed_config.element[line_num-3].dist,token,sizeof parsed_config.element[line_num-3].dist-1);
                    parsed_config.element[line_num-3].dist[255] = '\0';
                }
                if(n_tokens==2){
                    strncpy(parsed_config.element[line_num-3].address,token,sizeof parsed_config.element[line_num-3].address-1);
                    parsed_config.element[line_num-3].address[255] = '\0';
                }
            }
           // Different call
           token = strtok (NULL, " ,");
           n_tokens++;
       }
       line_num++;
    }
   parsed_config.num_rows = line_num-3;

   printf("\nLines:%d",parsed_config.num_rows);
    
    

    fclose(fp);
    if (line)
        free(line);
    printf("\nEND");
    return parsed_config;
}

void disp_routing_table (struct Routing_table rt) {
    printf("\n--------------------");
    printf("\nRouting table of %s",rt.node);
    printf("\nNode\tDist\tNextHop");
    for(int i = 0; i < rt.num_rows;i++){
        printf("\n%s\t%d\t%s",rt.element[i].node, rt.element[i].dist, rt.element[i].next_hop);
    }
    printf("\n--------------------");
}

void disp_distance_vector (struct Distance_vector dv) {
    printf("\n--------------------");
    printf("\nDistance vector of %s",dv.sender);
    printf("\nNode\tDist");
    for(int i = 0; i < dv.num_of_dests;i++){
        printf("\n%s\t%d",dv.element[i].dest, dv.element[i].dist);
    }
    printf("\n--------------------");
}


struct Routing_table create_rt_from_parsed(struct Parsed_config parsed_config){
    /*
        Parse config file into parsed_config struct
        Use that struct to create rt table
    */
    // struct Parsed_config parsed_config;
    // parsed_config = test_parse_config_to_struct();
    struct Routing_table rt;
    strcpy(rt.node,parsed_config.node);
    rt.num_rows = parsed_config.num_rows;
    for(int i = 0; i < rt.num_rows;i++){
        strcpy(rt.element[i].node,parsed_config.element[i].node);
        int num;
        sscanf(parsed_config.element[i].dist,"%d",&num);
        rt.element[i].dist = num;
        strcpy(rt.element[i].next_hop, parsed_config.element[i].node); 
    }
    // rt.num_rows++;
    // printf("%d",rt.num_rows:)
    // strcpy(rt.element[rt.num_rows-1].node,rt.node);
    // rt.element[rt.num_rows-1].dist = 0;
    // strcpy(rt.element[rt.num_rows-1].next_hop,rt.node); 
    // disp_routing_table(rt);
    return rt;
}

struct Distance_vector create_dv_from_rt(struct Routing_table rt){
    struct Distance_vector dv;
    // struct Routing_table rt;
    // rt = test_create_rt_from_parsed();
    // disp_routing_table(rt);
    strcpy(dv.sender,rt.node);
    for(int i = 0; i < rt.num_rows; i++){
        dv.num_of_dests = rt.num_rows;
        strcpy(dv.element[i].dest,rt.element[i].node);
        dv.element[i].dist = rt.element[i].dist;
    }
    return dv;
}

struct Distance_vector convert_str_to_dv(char msg[]){
    // Convert string to struct dv
    // printf("\nmsg:%s",msg);
   // Split the string into tokens delimited by spaces and commas
    // printf("\nTokens:\n");
    int n_tokens = 0;
    int line_num = 1;
    char *token = NULL;
    token = strtok (msg," \r\n");   
    int neighbors = 0;
    struct Distance_vector dv;
    int curr_neighbor = -1;
    while (token != NULL)
    {
        // printf("\nn_tokens:%d,token:%s.", n_tokens,token);
        // printf("\nsize:%zu",sizeof(*token));
        if(n_tokens == 0){
            strcpy(dv.sender,token);
            dv.sender[5] = '\0';
            // printf("\nAdding sender:%s",token);
            // printf("\ndv sender:%s.",dv.sender);

        }
        else if(n_tokens % 2 == 1 || n_tokens == 1){
            curr_neighbor++;
            // printf("\n%d",curr_neighbor);
            strcpy(dv.element[curr_neighbor].dest,token);
            dv.sender[5] = '\0';
            // printf("\nAdding neighbor:%s",token);
            // printf("\ndv neighbor:%s.",dv.element[curr_neighbor].dest);
        }
        else if(n_tokens % 2 == 0){
            int int_dist;
            sscanf(token,"%d",&int_dist);
            dv.element[curr_neighbor].dist = int_dist;
            // printf("\nAdding dist:%d",int_dist);
            // printf("\ndv dist:%d.",dv.element[curr_neighbor].dist);
        }

        // printf("\ncurr_n:%d",curr_neighbor);

        // Different call
        token = strtok (NULL, " \r\n");
        n_tokens++;
        if (n_tokens % 2 == 0){
            neighbors++;
        }
        // printf("\n");
    }
    dv.num_of_dests = neighbors;

    // printf("\nnum_of_dests:%d",dv.num_of_dests);
    // printf("\nsender:%s",dv.sender);
    // printf("\n--------------------");
    // printf("\nDistance vector of %s",dv.sender);
    // printf("\nNode\tDist");

    return dv;

}
struct Routing_table update_routing(struct Distance_vector dv, struct Routing_table rt){
    // printf("\nUpdating routing");
    int add_to;
    for(int i = 0; i < rt.num_rows; i++){
        printf("\nrt element node:%s",rt.element[i].node);
        if(strcmp(dv.sender,rt.element[i].node)==0){
            add_to = rt.element[i].dist;
        }
    }
    for(int j = 0; j < dv.num_of_dests; j++){
        // printf("\n%s\t%d\t%s",rt.element[i].node, rt.element[i].dist, rt.element[i].next_hop);
        int match = 0;
        for(int i = 0; i < rt.num_rows;i++){
            // printf("\nChecking if rt %s matches dv%s ",rt.element[i].node,dv.element[j].dest);
            // if(rt.element[i].node==dv.element[j].dest){ // Broken due to data change
            if(strcmp(rt.element[i].node,dv.element[j].dest)==0){
                // printf("\tMatch");
                match = 1;
                // printf("\n\trt dist:%d\tdv dist:%d",rt.element[i].dist , dv.element[j].dist);
                if(dv.element[j].dist + add_to < rt.element[i].dist){
                    // printf("\nDecreasing distance");
                    rt.element[i].dist = dv.element[j].dist;
                    strcpy(rt.element[i].next_hop,dv.sender);
                }
                else{
                    // printf("\nNot decreasing distance");
                }
                break;
            }
        }
        if(match == 0){
            rt.num_rows++;
            strcpy(rt.element[rt.num_rows-1].node,dv.element[j].dest);
            rt.element[rt.num_rows-1].dist = dv.element[j].dist;
            strcpy(rt.element[rt.num_rows-1].next_hop, dv.sender);
        }
    }
    // printf("\nAfter updating:");
    // disp_routing_table(rt);
    return rt;
}