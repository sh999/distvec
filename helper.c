// Helper functions for distance vector routing protocol program
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structdefs.h"
struct Parsed_config parse_config(char file[]){
    /*
        Read config file to create initial routing table
    */
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    struct Parsed_config parsed_config;
    fp = fopen(file, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    int line_num = 1;
    while ((read = getline(&line, &len, fp)) != -1) {
        char str[256];
        strncpy(str,line,sizeof str-1);
        str[255] = '\0';
        char *token = NULL;
        int n_tokens = 0;
       token = strtok (str," ,");
       while (token != NULL)
       {
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
                }
                if(n_tokens==1){
                    int int_dist;
                    sscanf(token,"%d",&int_dist);
                    parsed_config.element[line_num-3].dist = int_dist;
                }
                if(n_tokens==2){
                    strncpy(parsed_config.element[line_num-3].address,token,sizeof parsed_config.element[line_num-3].address-1);
                    parsed_config.element[line_num-3].address[255] = '\0';
                }
            }
           token = strtok (NULL, " ,");
           n_tokens++;
       }
       line_num++;
    }
   parsed_config.num_rows = line_num-3;
    fclose(fp);
    if (line)
        free(line);
    return parsed_config;
}

void disp_routing_table (struct Routing_table rt) {
    /*
        Print routing table
    */
    printf("\n-----------------------");
    printf("\nRouting table of %s",rt.node);
    printf("\nNode\tDist\tNextHop");
    for(int i = 0; i < rt.num_rows;i++){
        printf("\n%s\t%d\t%s",rt.element[i].node, rt.element[i].dist, rt.element[i].next_hop);
    }
    printf("\n-----------------------");
}

void disp_distance_vector (struct Distance_vector dv) {
    /*
        Print distance vector
    */
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
    struct Routing_table rt;
    strcpy(rt.node,parsed_config.node);
    rt.num_rows = parsed_config.num_rows;
    for(int i = 0; i < rt.num_rows;i++){
        strcpy(rt.element[i].node,parsed_config.element[i].node);
        rt.element[i].dist = parsed_config.element[i].dist;
        strcpy(rt.element[i].next_hop, parsed_config.element[i].node); 
    }
    return rt;
}

struct Distance_vector create_dv_from_rt(struct Routing_table rt){
    /*
        Create distance vector from routing table
    */
    struct Distance_vector dv;
    strcpy(dv.sender,rt.node);
    for(int i = 0; i < rt.num_rows; i++){
        dv.num_of_dests = rt.num_rows;
        strcpy(dv.element[i].dest,rt.element[i].node);
        dv.element[i].dist = rt.element[i].dist;
    }
    return dv;
}

struct Distance_vector convert_str_to_dv(char msg[]){
    /*
        Convert string to distance vector
        String is message received through socket
    */
    int n_tokens = 0;
    int line_num = 1;
    char *token = NULL;
    token = strtok (msg," \r\n");   
    int neighbors = 0;
    struct Distance_vector dv;
    int curr_neighbor = -1;
    while (token != NULL)
    {
        if(n_tokens == 0){
            strcpy(dv.sender,token);
            dv.sender[5] = '\0';
        }
        else if(n_tokens % 2 == 1 || n_tokens == 1){
            curr_neighbor++;
            strcpy(dv.element[curr_neighbor].dest,token);
            dv.sender[5] = '\0';
        }
        else if(n_tokens % 2 == 0){
            int int_dist;
            sscanf(token,"%d",&int_dist);
            dv.element[curr_neighbor].dist = int_dist;
        }
        token = strtok (NULL, " \r\n");
        n_tokens++;
        if (n_tokens % 2 == 0){
            neighbors++;
        }
    }
    dv.num_of_dests = neighbors;
    return dv;
}
struct Routing_table update_routing(struct Distance_vector dv, struct Routing_table rt, struct Parsed_config pc){
    /*
        Update routing table based on distance vector and neighbors
    */
    int add_to;
    for(int i = 0; i < pc.num_rows; i++){
        if(strcmp(dv.sender,pc.element[i].node)==0){
            add_to = pc.element[i].dist;
        }
    }
    for(int j = 0; j < dv.num_of_dests; j++){
        int match = 0;
        for(int i = 0; i < rt.num_rows;i++){
            if(strcmp(rt.element[i].node,dv.element[j].dest)==0){
                match = 1;
                if(dv.element[j].dist + add_to < rt.element[i].dist){
                    rt.element[i].dist = dv.element[j].dist + add_to;
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
            rt.element[rt.num_rows-1].dist = dv.element[j].dist + add_to;
            strcpy(rt.element[rt.num_rows-1].next_hop, dv.sender);
        }
    }
    return rt;
}