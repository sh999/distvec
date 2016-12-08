// Read a config file and filter into struct
// From http://stackoverflow.com/questions/3501338/c-read-file-line-by-line
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct My_node{
    // char* name;  // My node name
    // char* port;
    char name[6];  // My node name
    char port[6];
};
struct Neighbor{
    char name[6];
    char cost[6];
    char address[6];
};
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
    char dist[6];
    char address[16];
};
struct Parsed_config {
    char node[6];
    char port[6];
    int num_rows;
    struct Config_element element[8];
};
void disp_distance_vector (struct Distance_vector dv) {
    printf("\n--------------------");
    printf("\nDistance vector of %s",dv.sender);
    printf("\nNode\tDist");
    for(int i = 0; i < dv.num_of_dests;i++){
        printf("\n%s\t%d",dv.element[i].dest, dv.element[i].dist);
    }
    printf("\n--------------------");
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
void update_routing(struct Distance_vector dv, struct Routing_table rt){
    printf("\nUpdating routing");
    for(int i = 0; i < rt.num_rows;i++){
        // printf("\n%s\t%d\t%s",rt.element[i].node, rt.element[i].dist, rt.element[i].next_hop);
        for(int j = 0; j < dv.num_of_dests; j++){
            printf("\nChecking if %s matches %s ",rt.element[i].node,dv.element[j].dest);
            // if(rt.element[i].node==dv.element[j].dest){ // Broken due to data change
            if(strcmp(rt.element[i].node,dv.element[j].dest)==0){
                printf("\tMatch");
                printf("\n\trt dist:%d\tdv dist:%d",rt.element[i].dist , dv.element[j].dist);
                if(dv.element[j].dist<rt.element[i].dist){
                    printf("\nDecreasing distance");
                    rt.element[i].dist = dv.element[j].dist;
                }
                else{
                    printf("\nNot decreasing distance");
                }
                break;
            }
            else{
                printf("\tNo match");
            }
        }
    }
    printf("\nAfter updating:");
    disp_routing_table(rt);
}


int test_config()
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    struct My_node my_node;
    struct Neighbor neighbor1;
    struct Neighbor neighbor2;

    // struct Neighbor neighbors[];

    fp = fopen("./neighbor_config", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    int line_num = 1;
    while ((read = getline(&line, &len, fp)) != -1) {
        printf("\nRetrieved line of length %zu :\n", read);
        printf("\n%s", line);
        printf("\nline_num:%d",line_num);
        if(line_num == 1){
            printf("\nSetting name to line:%s",line);

            // memcpy(my_node.name,line,sizeof(line));
        }
        // else if(line_num == 2){
        //     printf("\nSetting port to line:%s",line);
        //     memcpy(my_node.port,line,sizeof(line));
        // }
        // else if(line_num == 3){
        //     printf("\nSetting neighbor1 to line:%s",line);
        //     memcpy(neighbor1.name,line,sizeof(line));
        // }
        // else if(line_num == 4){
        //     printf("\nSetting port to line:%s",line);
        //     memcpy(neighbor1.cost,line,sizeof(line));
        // }
        // else if(line_num == 5){
        //     printf("\nSetting port to line:%s",line);
        //     memcpy(neighbor1.address,line,sizeof(line));
        // }
        // else if(line_num == 6){
        //     printf("\nSetting neighbor1 to line:%s",line);
        //     memcpy(neighbor2.name,line,sizeof(line));
        // }
        // else if(line_num == 7){
        //     printf("\nSetting port to line:%s",line);
        //     memcpy(neighbor2.cost,line,sizeof(line));
        // }
        // else if(line_num == 8){
        //     printf("\nSetting port to line:%s",line);
        //     memcpy(neighbor2.address,line,sizeof(line));
        // }

        line_num++;

    }
    printf("\nDone. line_num:%d",line_num);
    printf("\nParsed file:");
    printf("\nMy node name:%s",my_node.name);
    printf("\nMy node port:%s",my_node.port);
    printf("\nNeighbor1 name:%s",neighbor1.name);
    printf("\nNeighbor1 cost:%s",neighbor1.cost);
    printf("\nNeighbor1 address:%s",neighbor1.address);
    printf("\nNeighbor2 name:%s",neighbor2.name);
    printf("\nNeighbor2 cost:%s",neighbor2.cost);
    printf("\nNeighbor2 address:%s",neighbor2.address);
    fclose(fp);
    if (line)
        free(line);
    exit(EXIT_SUCCESS);
}

int test_token(){
    char str[] ="Elizarraras, Salvador, UG";
    char *token = NULL;
    int n_tokens = 0;

    printf ("Get tokens from string \"%s\"\n",str);
    printf("%s\n", "*********************************************");

   // Split the string into tokens delimited by spaces and commas
   token = strtok (str," ,");
   while (token != NULL)
   {
       printf("%s\n", token);
       // Different call
       token = strtok (NULL, " ,");
       n_tokens++;
   }

  return 0;
}

int test_tokenize_file()
{
    /*
    http://stackoverflow.com/questions/12956477/reading-a-text-file-into-a-data-structure-in-c
    http://stackoverflow.com/questions/1508838/how-to-copy-char-str-to-char-c-in-c
    http://www.cs.bu.edu/teaching/cpp/string/array-vs-ptr/
    */
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("./neighbor_config", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        printf("Retrieved line of length %zu :\n", read);
        printf("%s", line);

        char str[256];
        strncpy(str,line,sizeof str-1);
        str[255] = '\0';
        char *token = NULL;
        int n_tokens = 0;

        // printf ("Get tokens from string \"%s\"\n",str);
        // printf("%s\n", "*********************************************");

       // Split the string into tokens delimited by spaces and commas
       token = strtok (str," ,");
       printf("\nTokens:\n");
       while (token != NULL)
       {
           printf("%s\n", token);
           // Different call
           token = strtok (NULL, " ,");
           n_tokens++;
       }
    }
    fclose(fp);
    if (line)
        free(line);
    exit(EXIT_SUCCESS);
}

int test_token_struct()
{
    /*
    http://stackoverflow.com/questions/12956477/reading-a-text-file-into-a-data-structure-in-c
    http://stackoverflow.com/questions/1508838/how-to-copy-char-str-to-char-c-in-c
    http://www.cs.bu.edu/teaching/cpp/string/array-vs-ptr/
    */
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    struct My_node my_node;
    struct Neighbor n1;

    fp = fopen("./neighbor_config", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    int line_num = 1;
    while ((read = getline(&line, &len, fp)) != -1) {
        printf("Retrieved line of length %zu :\n", read);
        printf("%s", line);

        char str[256];
        strncpy(str,line,sizeof str-1);
        str[255] = '\0';
        char *token = NULL;
        int n_tokens = 0;

       // Split the string",  into tokens delimited by spaces and commas
       token = strtok (str," ,");
       while (token != NULL)
       {
            printf("%s\n", token);
            if(line_num==1){
                strncpy(my_node.name,token,sizeof my_node.name-1);
                my_node.name[255] = '\0';
            }
            else if(line_num==2){
                strncpy(my_node.port,token,sizeof my_node.port-1);
                my_node.port[255] = '\0';
            }
            else if(line_num==3){
                if(n_tokens==0){
                    strncpy(n1.name,token,sizeof n1.name-1);
                    n1.name[255] = '\0';
                }
                if(n_tokens==1){
                    strncpy(n1.cost,token,sizeof n1.cost-1);
                    n1.cost[255] = '\0';
                }
                if(n_tokens==2){
                    strncpy(n1.address,token,sizeof n1.address-1);
                    n1.address[255] = '\0';
                }
            }
           // Different call
           token = strtok (NULL, " ,");
           n_tokens++;
       }
       line_num++;
    }
    printf("\nMy node name:%s",my_node.name);
    printf("\nMy port name:%s",my_node.port);
    printf("\nNeighbor1 name:%s",n1.name);
    printf("\nNeighbor1 cost:%s",n1.cost);
    printf("\nNeighbor1 address:%s",n1.address);
    fclose(fp);
    if (line)
        free(line);
    exit(EXIT_SUCCESS);
}
struct Distance_vector test_create_dv(){
    /*
        Test creation of distance vector
    */
        printf("\nin test create dv");
    struct Distance_vector dv_incoming;
    strcpy(dv_incoming.sender,"A");
    dv_incoming.sender[5] = '\0';
    dv_incoming.num_of_dests = 2;
    strcpy(dv_incoming.element[0].dest,"E");
    dv_incoming.element[0].dest[5] = '\0';
    dv_incoming.element[0].dist = 3;
    strcpy(dv_incoming.element[1].dest,"F");
    dv_incoming.element[1].dest[5] = '\0';
    dv_incoming.element[1].dist = 2;
    disp_distance_vector(dv_incoming);
    return dv_incoming;
}
struct Routing_table test_create_rt(){
    /*
        Test creation of routing table

    */
    struct Distance_vector dv_incoming;
    struct Routing_table rt;
    strcpy(rt.node,"B");
    rt.num_rows = 3;
    strcpy(rt.element[0].node,"A");
    rt.element[0].dist = 9;
    strcpy(rt.element[0].next_hop,"B");
    strcpy(rt.element[1].node,"F");
    rt.element[1].dist = 9;
    strcpy(rt.element[1].next_hop,"B");
    strcpy(rt.element[2].node,"C");
    rt.element[2].dist = 2;
    strcpy(rt.element[2].next_hop,"B");
    disp_routing_table(rt);
    return rt;
}



void test_update_routing(){
    /*
        Create a distance vector and routing table
        Update routing table by distance vector values
        In here, the creation of the 2 objects are coded and not from a config file
        In real program, dv comes from another node and rt comes from init config file
    */
        printf("\nIn test update_routing");
    struct Distance_vector dv;
    struct Routing_table rt;
    dv = test_create_dv();
    rt = test_create_rt();
    disp_distance_vector(dv);
    disp_routing_table(rt);
    update_routing(dv,rt);

}

struct Parsed_config test_parse_config_to_struct(){
    /*
        Read config file to create initial routing table
    */
    printf("\nTesting making init routing table from config file");
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
        printf("Retrieved line of length %zu :\n", read);
        printf("%s", line);

        char str[256];
        strncpy(str,line,sizeof str-1);
        str[255] = '\0';
        char *token = NULL;
        int n_tokens = 0;

       // Split the string",  into tokens delimited by spaces and commas
       token = strtok (str," ,");
       while (token != NULL)
       {
            printf("%s\n", token);
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
       parsed_config.num_rows = line_num-3;
       printf("\nLines:%d",parsed_config.num_rows);
    }
    printf("\nMy node name:%s",parsed_config.node);
    printf("\nMy port name:%s",parsed_config.port);
    for(int i = 0; i < parsed_config.num_rows; i++){
        printf("Neighbor #%d :%s\tDist:%s\tAddress:%s",i,parsed_config.element[i].node,parsed_config.element[i].dist,parsed_config.element[i].address);
    }
    // printf("\nNeighbor1 name:%s",n1.name);
    // printf("\nNeighbor1 cost:%s",n1.cost);
    // printf("\nNeighbor1 address:%s",n1.address);
    

    fclose(fp);
    if (line)
        free(line);
    printf("\nEND");
    return parsed_config;
}

struct Routing_table test_create_rt_from_parsed(){
    /*
        Parse config file into parsed_config struct
        Use that struct to create rt table
    */
    struct Parsed_config parsed_config;
    parsed_config = test_parse_config_to_struct();
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
    disp_routing_table(rt);
    return rt;
}

struct Distance_vector test_create_dv_from_rt(){
    struct Distance_vector dv;
    struct Routing_table rt;
    rt = test_create_rt_from_parsed();
    disp_routing_table(rt);
    strcpy(dv.sender,rt.node);
    for(int i = 0; i < rt.num_rows; i++){
        dv.num_of_dests = rt.num_rows;
        strcpy(dv.element[i].dest,rt.element[i].node);
        dv.element[i].dist = rt.element[i].dist;
    }
    return dv;
}

void setup_test_create_dv_from_rt(){
    struct Distance_vector dv;
    dv = test_create_dv_from_rt();
    disp_distance_vector(dv);
    
}
char * test_dv_to_msg(struct Distance_vector dv){
    // Turn dv into string
    disp_distance_vector(dv);
    char msg[255];
    for(int i = 0; i < dv.num_of_dests; i++){
        if(i == 0){
            
        }
    }
    return "hey";
}
void test_dv_to_msg_setup(){
    // Create dv, turn it into string
    struct Distance_vector dv;
    dv = test_create_dv_from_rt();
    disp_distance_vector(dv);
    char msg[255];
    strcpy(msg,test_dv_to_msg(dv));    
    printf("\nsizeof dv:%zu",sizeof(dv));
    printf("\nmsg:%s",msg);
    printf("\nsizeof msg:%zu",sizeof(msg));
}

void test_string(){
    // Testing strcat, strcpy
    char str1[] = "bob";
    char * str2 = "bo2";
    char * str3;
    char str4[30];
    // strcpy(str3,str1);
    strcpy(str4,str1);
    strcat(str4,str1);
    // str4[2] = '\0';
    // printf("%s",str1);
    printf("\n%zu",sizeof(str1));
    printf("\n%s",str2);
    printf("\n%zu",sizeof(str2));
    printf("\n%s",str3);
    printf("\n%zu",sizeof(str3));
    printf("\n%send",str4);
    printf("\n%zu",sizeof(str4));
    for(int i = 0; i < 30;i++){
        printf("\n%d%c",i,str4[i]);

    }
}

void test_convert(){
    char some[30];
    int i = 33;
    sprintf(some,"%d",i);
    printf("\n%s",some);
    char some2[30] = "123";
    sscanf(some2,"%d",&i);
    printf("\n%d",i);
    printf("\n%d",i+1);
}
struct Distance_vector convert_str_to_dv(char msg[]){
    // Convert string to struct dv
    printf("\nmsg:%s",msg);
    

    // printf ("Get tokens from string \"%s\"\n",str);
    // printf("%s\n", "*********************************************");

   // Split the string into tokens delimited by spaces and commas
   // token = strtok (str," ,");
    printf("\nTokens:\n");
    int n_tokens = 0;
    int line_num = 1;
    char *token = NULL;
    token = strtok (msg," \r\n");   
    int neighbors = 0;
    struct Distance_vector dv;
    int curr_neighbor = -1;
    while (token != NULL)
    {
        printf("\nn_tokens:%d,token:%s.", n_tokens,token);
        printf("\nsize:%zu",sizeof(*token));
        if(n_tokens == 0){
            strcpy(dv.sender,token);
            dv.sender[5] = '\0';
            printf("\nAdding sender:%s",token);
            printf("\ndv sender:%s.",dv.sender);

        }
        else if(n_tokens % 2 == 1 || n_tokens == 1){
            curr_neighbor++;
            printf("\n%d",curr_neighbor);
            strcpy(dv.element[curr_neighbor].dest,token);
            dv.sender[5] = '\0';
            printf("\nAdding neighbor:%s",token);
            printf("\ndv neighbor:%s.",dv.element[curr_neighbor].dest);
        }
        else if(n_tokens % 2 == 0){
            int int_dist;
            sscanf(token,"%d",&int_dist);
            dv.element[curr_neighbor].dist = int_dist;
            printf("\nAdding dist:%d",int_dist);
            printf("\ndv dist:%d.",dv.element[curr_neighbor].dist);
        }

        printf("\ncurr_n:%d",curr_neighbor);

        // Different call
        token = strtok (NULL, " \r\n");
        n_tokens++;
        if (n_tokens % 2 == 0){
            neighbors++;
        }
        printf("\n");
    }
    dv.num_of_dests = neighbors;

    printf("\nnum_of_dests:%d",dv.num_of_dests);
    printf("\nsender:%s",dv.sender);
    printf("\n--------------------");
    printf("\nDistance vector of %s",dv.sender);
    printf("\nNode\tDist");
    // for(int i = 0; i < dv.num_of_dests;i++){
    //     printf("\n%s\t%d",dv.element[i].dest, dv.element[i].dist);
    // }
    // printf("\n--------------------");

    return dv;

}
int main(void){
    // test_config();
    // test_token();
    // test_tokenize_file();
    // test_token_struct();
    // test_create_dv();
    // test_create_rt();
    // test_update_routing();
    // test_parse_config_to_struct();
    struct Parsed_config parsed_config;
    parsed_config = test_parse_config_to_struct();
    // test_create_rt_from_parsed();
    //setup_test_create_dv_from_rt()
    // test_dv_to_msg_setup();
    // test_string();
    // test_convert();
    { // test convert str to dv
        // char msg[] = "A  \n B  3 \nC     2";
        // struct Distance_vector dv;
        // dv = convert_str_to_dv(msg);
        // disp_distance_vector(dv);
    }
    { // test update rt from dv

    }
    

    printf("\nReturned with no errors");

}
