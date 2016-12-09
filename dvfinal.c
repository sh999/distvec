/* 
    Distance vector routing protocol-Final version
*/
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "structdefs.h" // routing table structs

//*****************client**********************
#include <errno.h>      /* for errno and EINTR */
#include <signal.h>     /* for sigaction() */

#define ECHOMAX         255     /* Longest string to echo */
#define TIMEOUT_SECS    2       /* Seconds between retransmits */
#define MAXTRIES        5       /* Tries before giving up */

int tries=0;   /* Count of times sent - GLOBAL for signal-handler access */
void CatchAlarm(int ignored);            /* Handler for SIGALRM */


//*****************client**********************
void DieWithError(char *errorMessage);  /* External error handling function */
struct Parsed_config parse_config(char file[]);
void disp_routing_table (struct Routing_table rt);
void disp_distance_vector (struct Distance_vector dv);
struct Distance_vector create_dv_from_rt(struct Routing_table rt);
struct Routing_table create_rt_from_parsed(struct Parsed_config parsed_config);
struct Distance_vector convert_str_to_dv(char msg[]);
struct Routing_table update_routing(struct Distance_vector dv, struct Routing_table rt, struct Parsed_config parsed_config);

int main(int argc, char *argv[])
{
    struct Parsed_config parsed_config;
    parsed_config = parse_config(argv[1]);
    parsed_config.node[1] = '\0';
    printf("\nInitial properties");
    printf("\n--------------------------------");
    printf("\nConfig of %s",parsed_config.node);
    // printf("\nPort:%s",parsed_config.port);
    for(int i = 0; i < parsed_config.num_rows; i++){
        printf("\nNeighbor \tDist \tAddress");
        printf("\n%s\t%d\t%s",parsed_config.element[i].node,parsed_config.element[i].dist,parsed_config.element[i].address);
    }
    printf("\n--------------------------------");

    struct Routing_table rt;            // Create initial routing table from parsedconfig file
    rt = create_rt_from_parsed(parsed_config);
    disp_routing_table(rt);
    struct Distance_vector dv;          // Create initial distance vector from routing table
    dv = create_dv_from_rt(rt);
    disp_distance_vector(dv);

    rt.num_rows++;                      // Add self to routing table with distance 0
    strcpy(rt.element[rt.num_rows-1].node,parsed_config.node);
    rt.element[rt.num_rows-1].dist = 0;
    strcpy(rt.element[rt.num_rows-1].next_hop,parsed_config.node); 

    // Setup listening (prefix s_, for 'server') and sending sockets (prefix c_, for 'client')
    int s_sock;                        /* Listening ocket */
    struct sockaddr_in s_echoServAddr; /* Local address */
    struct sockaddr_in s_echoClntAddr; /* Client address */
    unsigned int s_cliAddrLen;         /* Length of incoming message */
    char s_echoBuffer[ECHOMAX];        /* Buffer for echo string */
    unsigned short s_echoServPort;     /* Server port */
    int s_recvMsgSize;                 /* Size of received message */
    int c_sock;                        /* Sender socket */
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
    s_echoServPort = 5000;              // Hard code what port I will get from
    c_echoServPort = 5000;
    c_servIP = parsed_config.element[0].address;  // Set client IP based on parsed config neighbor value
    c_echoString = parsed_config.node;
    // printf("\nMsg to send:%s",c_echoString);
    if ((c_echoStringLen = strlen(c_echoString)) > ECHOMAX)
        DieWithError("Echo word too long");
    // Create array of addresses based on those found in config file
    struct sockaddr_in all_addresses[parsed_config.num_rows];  //SH: addresses to create sockets from
    char *c_servIP2;
    for(int i = 0; i < parsed_config.num_rows; i++){
        memset(&all_addresses[i], 0, sizeof(all_addresses[i]));    /* Zero out structure */
        all_addresses[i].sin_family = AF_INET;
        all_addresses[i].sin_addr.s_addr = inet_addr(parsed_config.element[i].address);  /* Server IP address */
        all_addresses[i].sin_port = htons(c_echoServPort);       /* Server port */
    }
    /* Create a best-effort datagram socket using UDP */
    if ((c_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
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
    if ((s_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    // /* Construct local address structure */
    memset(&s_echoServAddr, 0, sizeof(s_echoServAddr));   /* Zero out structure */
    s_echoServAddr.sin_family = AF_INET;                /* Internet address family */
    s_echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    s_echoServAddr.sin_port = htons(s_echoServPort);      /* Local port */

     // Bind to the local address 
    if (bind(s_sock, (struct sockaddr *) &s_echoServAddr, sizeof(s_echoServAddr)) < 0)
        DieWithError("bind() failed");
    int counter = 0;
    char msg[255];
    strcpy(msg,dv.sender);
    for(int i = 0; i < dv.num_of_dests; i++){       // Convert distance vector to string to be passed through socket
        strcat(msg,"\n");
        strcat(msg,dv.element[i].dest);
        strcat(msg," ");
        char str_dist[6];
        sprintf(str_dist,"%d",dv.element[i].dist);
        strcat(msg,str_dist);
    }
    // printf("\nstring_msg:\n%s",msg);
    // printf("\nsize:%zu",sizeof(msg));
    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        s_cliAddrLen = sizeof(s_echoClntAddr);
        alarm(TIMEOUT_SECS);
        /* Block until receive message from a client */
        printf("\n\nWaiting for other messages...");
        printf("\n\n*******************************************\nStep:%d",counter);
        if(counter % 5 != 0){                   // Ensures that if client gets constant message, a message send will be forced
            // Keep waiting to recieve message under TIMEOUT time
            while ((s_recvMsgSize = recvfrom(s_sock, s_echoBuffer, ECHOMAX, 0,
                (struct sockaddr *) &s_echoClntAddr, &s_cliAddrLen)) < 0)
                if (errno == EINTR){            // Send at every time out time, then keep listening
                    printf("\nAttempting to send my message...");
                    for(int i = 0; i < parsed_config.num_rows; i++){    // Send to all neighbors
                        if (sendto(c_sock, msg, sizeof(msg), 0, (struct sockaddr *)
                            &all_addresses[i], sizeof(all_addresses[i])) != sizeof(msg))
                            DieWithError("sendto() sent a different number of bytes than expected");
                    }
                    alarm(TIMEOUT_SECS);        // Set timeout again
                }
                printf("\nGot message from %s:", inet_ntoa(s_echoClntAddr.sin_addr));   // Successfully received message
                struct Distance_vector rec_dv;                      // Convert received string to received distance vector
                rec_dv = convert_str_to_dv(s_echoBuffer);           
                disp_distance_vector(rec_dv);                       // Show incoming distance vector
                rt = update_routing(rec_dv,rt,parsed_config);       // Update my routing table based on received distance vector
                printf("\nUpdated Routing table:");
                disp_routing_table(rt);
                dv = create_dv_from_rt(rt);                         // Update my distance vector based on current routing table status
                // printf("\nUpdated dv:");
                // disp_distance_vector(dv);
                msg[0] = '\0';                                      // Setup next message
                strcpy(msg,dv.sender);
                for(int i = 0; i < dv.num_of_dests; i++){           // Convert updated distance vector to string to be passed 
                    strcat(msg,"\n");
                    strcat(msg,dv.element[i].dest);
                    strcat(msg," ");
                    char str_dist[6];
                    sprintf(str_dist,"%d",dv.element[i].dist);
                    strcat(msg,str_dist);
                }
        }
        else{
            printf("\nAttempting to send my message...");           // Ensures message is passed if client receives a lot of messages
            for(int i = 0; i < parsed_config.num_rows; i++){        // If this is not included, client will get messages and never receive
                if (sendto(c_sock, msg, sizeof(msg), 0, (struct sockaddr *)
                    &all_addresses[i], sizeof(all_addresses[i])) != sizeof(msg))
                    DieWithError("sendto() sent a different number of bytes than expected");
                }
        }
        counter++;
    }
    /* NOT REACHED */
}

void CatchAlarm(int ignored)     /* Handler for SIGALRM */
{
    tries += 1;
}
