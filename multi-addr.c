// Merged server.c and client.c so this prog can listen and send
// Based on client-merged-periodic.c
// Based on multiclient.c
// See if can send to multiple ip addresses (same port)
// Cand send to multiple ip addresses as long as the node has received a msg
// Otherwise will be stuck in listening loop
// Working on if I can pass through structs finally
// Based on multistruct.c
// Nodes can use this program by reading config file and using addresses from it
/*
To test:
Run a server program listening on 90211
Run a client program sending to 90210
Run this both program, which listens on 90210 and will send to 90211
client will send msgX, and both3 will get msgX, then will forward msgX to server
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
struct Parsed_config parse_config();
void disp_routing_table (struct Routing_table rt);
struct Routing_table create_rt_from_parsed();
int main(int argc, char *argv[])
{
    struct Parsed_config parsed_config;
    parsed_config = parse_config();
    printf("\n--------Config Properties--------");
    printf("\nMy node name:%s",parsed_config.node);
    printf("\nMy port:%s",parsed_config.port);
    for(int i = 0; i < parsed_config.num_rows; i++){
        printf("Neighbor #%d :\t%s\tDist:%s\tAddress:%s\n",i,parsed_config.element[i].node,parsed_config.element[i].dist,parsed_config.element[i].address);
    }
    printf("\n---------------------------------");

    struct Routing_table rt;
    rt = create_rt_from_parsed(parsed_config);
    disp_routing_table(rt);

    int s_sock;                        /* Socket */
    struct sockaddr_in s_echoServAddr; /* Local address */
    struct sockaddr_in s_echoClntAddr; /* Client address */
    unsigned int s_cliAddrLen;         /* Length of incoming message */
    char s_echoBuffer[ECHOMAX];        /* Buffer for echo string */
    unsigned short s_echoServPort;     /* Server port */
    int s_recvMsgSize;                 /* Size of received message */

//v****************client**********************
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
//^****************client**********************
    // From the server
    // if (argc != 2)         /* Test for correct number of parameters */ 
    // {
    //     fprintf(stderr,"Usage:  %s <UDP SERVER PORT>\n", argv[0]);
    //     exit(1);
    // }
    // if ((argc < 3) || (argc > 4))    /* Test for correct number of arguments */
    // {
    //     fprintf(stderr,"Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
    //     exit(1);
    // }
    // From the server
    // s_echoServPort = atoi(argv[1]);  /* First arg:  local port */
    s_echoServPort = 90210;  // Hard code what port I will get from

//v****************client**********************
    // c_servIP = argv[1];           /* First arg:  server IP address (dotted quad) */
    // c_echoString = argv[2];       /* Second arg: string to echo */
    
    c_servIP = parsed_config.element[0].address;  //SH: set client IP based on parsed config neighbor value
    c_echoString = parsed_config.node;
    printf("\nMsg to send:%s",c_echoString);
    if ((c_echoStringLen = strlen(c_echoString)) > ECHOMAX)
        DieWithError("Echo word too long");
    if (argc == 4)
        c_echoServPort = atoi(argv[3]);   //Use given port, if any 
    else
        c_echoServPort = 7;  /* 7 is well-known port for echo service */
    printf("Setting up...");
    // SH: Create array of addresses
    struct sockaddr_in all_addresses[parsed_config.num_rows];  //SH: addresses to create sockets from
    // Trying to have client socket to two diff addresses
    // struct sockaddr_in c_echoServAddr2; /* Echo server address */
    // unsigned short c_echoServPort2;     /* Echo server port */
    char *c_servIP2;
    // memset(&c_echoServAddr2, 0, sizeof(c_echoServAddr2));    /* Zero out structure */
    // c_echoServAddr2.sin_family = AF_INET;
    // c_echoServAddr2.sin_addr.s_addr = inet_addr(c_servIP2);  /* Server IP address */
    // c_echoServAddr2.sin_port = htons(c_echoServPort);       /* Server port */
    // Mine
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
//^****************client**********************
    //v****************client**********************
        /* Send the string to the server */
        // if (sendto(c_sock, c_echoString, c_echoStringLen, 0, (struct sockaddr *)
        //            &c_echoServAddr, sizeof(c_echoServAddr)) != c_echoStringLen)
        //     DieWithError("2sendto() sent a different number of bytes than expected");
        //^****************client**********************
    // /* Create socket for sending/receiving datagrams */
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
    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        s_cliAddrLen = sizeof(s_echoClntAddr);
        alarm(TIMEOUT_SECS);
        /* Block until receive message from a client */
        printf("\nWaiting for other messages...");
        printf("\n\nCounter:%d",counter);
        if(counter % 5 != 0){
            while ((s_recvMsgSize = recvfrom(s_sock, s_echoBuffer, ECHOMAX, 0,
                (struct sockaddr *) &s_echoClntAddr, &s_cliAddrLen)) < 0)
                if (errno == EINTR){
                    printf("\nAttempting to send my message...");
                    // printf("\n#Number of neighbors:%d",parsed_config.num_rows);
                    for(int i = 0; i < parsed_config.num_rows; i++){
                        if (sendto(c_sock, c_echoString, c_echoStringLen, 0, (struct sockaddr *)
                            &all_addresses[i], sizeof(all_addresses[i])) != c_echoStringLen)
                            DieWithError("sendto() sent a different number of bytes than expected");
                    }
                    if (argc == 5){
                        printf("\nSending message to different port...");
                        // if (sendto(c_sock, c_echoString, c_echoStringLen, 0, (struct sockaddr *)
                        // &c_echoServAddr2, sizeof(c_echoServAddr2)) != c_echoStringLen)
                        DieWithError("sendto() sent a different number of bytes than expected");
                    }
                    // printf("\nSetting alarm within errno if");
                    alarm(TIMEOUT_SECS);
                }
            printf("\nGot message from %s", inet_ntoa(s_echoClntAddr.sin_addr));
            printf("\nMessage:%s",s_echoBuffer);
        }
        else{
            printf("\nForce sending message...");
            for(int i = 0; i < parsed_config.num_rows; i++){
                        if (sendto(c_sock, c_echoString, c_echoStringLen, 0, (struct sockaddr *)
                            &all_addresses[i], sizeof(all_addresses[i])) != c_echoStringLen)
                            DieWithError("sendto() sent a different number of bytes than expected");
                    }
            // if (sendto(c_sock, c_echoString, c_echoStringLen, 0, (struct sockaddr *)
            //     &all_addresses[0], sizeof(all_addresses[0])) != c_echoStringLen)
            //     DieWithError("sendto() sent a different number of bytes than expected");
            if (argc == 5){
                printf("\nForce sending message to different port...");
                // if (sendto(c_sock, c_echoString, c_echoStringLen, 0, (struct sockaddr *)
                // &c_echoServAddr2, sizeof(c_echoServAddr2)) != c_echoStringLen)
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
