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

    fp = fopen("./neighbor_config", "r");
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