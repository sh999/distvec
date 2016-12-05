//
// Client that Reads config file, get 1 neighbor info, and send to that one neighbor

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() and alarm() */
#include <errno.h>      /* for errno and EINTR */
#include <signal.h>     /* for sigaction() */

#define ECHOMAX         255     /* Longest string to echo */
#define TIMEOUT_SECS    2       /* Seconds between retransmits */
#define MAXTRIES        5       /* Tries before giving up */

int tries=0;   /* Count of times sent - GLOBAL for signal-handler access */

void DieWithError(char *errorMessage);   /* Error handling function */
void CatchAlarm(int ignored);            /* Handler for SIGALRM */

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

void parse_config(struct My_node *my_node,struct Neighbor *n1,struct Neighbor *n2)
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

    // struct My_node my_node;
    // struct Neighbor n1;

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
                strncpy(my_node->name,token,sizeof my_node->name-1);
                my_node->name[255] = '\0';
            }
            else if(line_num==2){
                strncpy(my_node->port,token,sizeof my_node->port-1);
                my_node->port[255] = '\0';
            }
            else if(line_num==3){
                if(n_tokens==0){
                    strncpy(n1->name,token,sizeof n1->name-1);
                    n1->name[255] = '\0';
                }
                if(n_tokens==1){
                    strncpy(n1->cost,token,sizeof n1->cost-1);
                    n1->cost[255] = '\0';
                }
                if(n_tokens==2){
                    strncpy(n1->address,token,sizeof n1->address-1);
                    n1->address[255] = '\0';
                }
            }
            else if(line_num==4){
                if(n_tokens==0){
                    strncpy(n2->name,token,sizeof n2->name-1);
                    n2->name[255] = '\0';
                }
                if(n_tokens==1){
                    strncpy(n2->cost,token,sizeof n2->cost-1);
                    n2->cost[255] = '\0';
                }
                if(n_tokens==2){
                    strncpy(n2->address,token,sizeof n2->address-1);
                    n2->address[255] = '\0';
                }
            }
           // Different call
           token = strtok (NULL, " ,");
           n_tokens++;
       }
       line_num++;
    }
    printf("\nMy node name:%s",my_node->name);
    printf("\nMy port name:%s",my_node->port);
    printf("\nNeighbor1 name:%s",n1->name);
    printf("\nNeighbor1 cost:%s",n1->cost);
    printf("\nNeighbor1 address:%s",n1->address);
    fclose(fp);
    if (line)
        free(line);
}

struct test{
    int x;
};
void bla(int *x){
    *x = *x * 2;
}
void glu(struct test *s){
    s->x = 3;
}
int main(int argc, char *argv[])
{
    int c_sock;                        /* Socket descriptor */
    struct sockaddr_in c_echoServAddr; /* Echo server address */
    struct sockaddr_in c_fromAddr;     /* Source address of echo */
    unsigned short c_echoServPort;     /* Echo server port */
    unsigned int c_fromSize;           /* In-out of address size for recvfrom() */
    struct sigaction c_myAction;       /* For setting signal handler */
    char *c_servIP;                    /* IP address of server */
    char *c_echoString;                /* String to send to echo server */
    char c_echoBuffer[ECHOMAX+1];      /* Buffer for echo string */
    int c_echoStringLen;               /* Length of string to echo */
    int c_respStringLen;               /* Size of received datagram */

    int c_sock2;                        /* Socket descriptor */
    struct sockaddr_in c_echoServAddr2; /* Echo server address */
    struct sockaddr_in c_fromAddr2;     /* Source address of echo */
    unsigned short c_echoServPort2;     /* Echo server port */
    unsigned int c_fromSize2;           /* In-out of address size for recvfrom() */
    // struct sigaction c_myAction;       /* For setting signal handler */
    char *c_servIP2;                    /* IP address of server */
    // char *c_echoString;                /* String to send to echo server */
    // char c_echoBuffer[ECHOMAX+1];      /* Buffer for echo string */
    // int c_echoStringLen;               /* Length of string to echo */
    // int c_respStringLen;               /* Size of received datagram */


    // Get struct from neighbor config file
    struct My_node my_node;
    struct Neighbor n1;
    struct Neighbor n2;
    
    parse_config(&my_node,&n1,&n2);
    printf("\nMy node name:%s",my_node.name);
    printf("\nMy port name:%s",my_node.port);
    printf("\nNeighbor1 name:%s",n1.name);
    printf("\nNeighbor1 cost:%s",n1.cost);
    printf("\nNeighbor1 address:%s",n1.address);
    printf("\nNeighbor2 name:%s",n2.name);
    printf("\nNeighbor2 cost:%s",n2.cost);
    printf("\nNeighbor2 address:%s",n2.address);

    if ((argc < 3) || (argc > 4))    /* Test for correct number of arguments */
    {
        fprintf(stderr,"Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
        exit(1);
    }

    // c_servIP = argv[1];           /* First arg:  server IP address (dotted quad) */
    printf("\nusing addres:%s",n2.address);
    c_servIP = n1.address;
    c_servIP2 = n2.address;
    c_echoString = argv[2];       /* Second arg: string to echo */

    if ((c_echoStringLen = strlen(c_echoString)) > ECHOMAX)
        DieWithError("Echo word too long");

    if (argc == 4){
        c_echoServPort = atoi(argv[3]);  /* Use given port, if any */
        c_echoServPort2 = atoi(argv[3]);
    }
    else
        c_echoServPort = 7;  /* 7 is well-known port for echo service */

    /* Create a best-effort datagram socket using UDP */
    if ((c_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");
    if ((c_sock2 = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Set signal handler for alarm signal */
    c_myAction.sa_handler = CatchAlarm;
    if (sigfillset(&c_myAction.sa_mask) < 0) /* block everything in handler */
        DieWithError("sigfillset() failed");
    c_myAction.sa_flags = 0;

    if (sigaction(SIGALRM, &c_myAction, 0) < 0)
        DieWithError("sigaction() failed for SIGALRM");

    /* Construct the server address structure */
    memset(&c_echoServAddr, 0, sizeof(c_echoServAddr));    /* Zero out structure */
    c_echoServAddr.sin_family = AF_INET;
    c_echoServAddr.sin_addr.s_addr = inet_addr(c_servIP);  /* Server IP address */
    c_echoServAddr.sin_port = htons(c_echoServPort);       /* Server port */

    memset(&c_echoServAddr2, 0, sizeof(c_echoServAddr2));    /* Zero out structure */
    c_echoServAddr2.sin_family = AF_INET;
    c_echoServAddr2.sin_addr.s_addr = inet_addr(c_servIP2);  /* Server IP address */
    c_echoServAddr2.sin_port = htons(c_echoServPort);       /* Server port */

    /* Send the string to the server */
    if (sendto(c_sock, c_echoString, c_echoStringLen, 0, (struct sockaddr *)
               &c_echoServAddr, sizeof(c_echoServAddr)) != c_echoStringLen)
        DieWithError("sendto() sent a different number of bytes than expected");
    c_echoString = "SECOND";
    if (sendto(c_sock2, c_echoString, c_echoStringLen, 0, (struct sockaddr *)
               &c_echoServAddr2, sizeof(c_echoServAddr2)) != c_echoStringLen)
        DieWithError("sendto() sent a different number of bytes than expected");

    /* Get a response */
    
    c_fromSize = sizeof(c_fromAddr);
    alarm(TIMEOUT_SECS);        /* Set the timeout */
    while ((c_respStringLen = recvfrom(c_sock, c_echoBuffer, ECHOMAX, 0,
           (struct sockaddr *) &c_fromAddr, &c_fromSize)) < 0)
        if (errno == EINTR)     /* Alarm went off  */
        {
            if (tries < MAXTRIES)      /* incremented by signal handler */
            {
                printf("timed out, %d more tries...\n", MAXTRIES-tries);
                if (sendto(c_sock, c_echoString, c_echoStringLen, 0, (struct sockaddr *)
                            &c_echoServAddr, sizeof(c_echoServAddr)) != c_echoStringLen)
                    DieWithError("sendto() failed");
                alarm(TIMEOUT_SECS);
            } 
            else
                DieWithError("No Response");
        } 
        else
            DieWithError("recvfrom() failed");

    /* recvfrom() got something --  cancel the timeout */
    alarm(0);

    /* null-terminate the received data */
    c_echoBuffer[c_respStringLen] = '\0';
    printf("Received: %s\n", c_echoBuffer);    /* Print the received data */
    
    close(c_sock);
    exit(0);
}

void CatchAlarm(int ignored)     /* Handler for SIGALRM */
{
    tries += 1;
}
