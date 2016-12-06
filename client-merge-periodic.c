// An attempt at making periodic send while listening
// Doesn't work so far
/*
To test:
Run a server program listening on 90211
Run a client program sending to 90210
Run this both program, which listens on 90210 and will send to 90211
client will send msgX, and both3 will get msgX, then will forward msgX to server
*/
#include <sys/time.h>

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */


//*****************client**********************
#include <errno.h>      /* for errno and EINTR */
#include <signal.h>     /* for sigaction() */

#define ECHOMAX         255     /* Longest string to echo */
#define TIMEOUT_SECS    10       /* Seconds between retransmits */
#define MAXTRIES        99       /* Tries before giving up */

int tries=0;   /* Count of times sent - GLOBAL for signal-handler access */
void CatchAlarm(int ignored);            /* Handler for SIGALRM */

//*****************client**********************


void DieWithError(char *errorMessage);  /* External error handling function */

int main(int argc, char *argv[])
{
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
    if ((argc < 3) || (argc > 4))    /* Test for correct number of arguments */
    {
        fprintf(stderr,"Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
        exit(1);
    }

    // From the server
    // s_echoServPort = atoi(argv[1]);  /* First arg:  local port */
    s_echoServPort = 90210;  // Hard code what port I will get from

//v****************client**********************
    c_servIP = argv[1];           /* First arg:  server IP address (dotted quad) */
    c_echoString = argv[2];       /* Second arg: string to echo */

    if ((c_echoStringLen = strlen(c_echoString)) > ECHOMAX)
        DieWithError("Echo word too long");

    if (argc == 4)
        c_echoServPort = atoi(argv[3]);  /* Use given port, if any */
    else
        c_echoServPort = 7;  /* 7 is well-known port for echo service */

    // Mine
    c_echoServPort = 90211;  // Hard code what this program will send to

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
  
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    /* Set timeout recvm */
    printf("\nBefore setsockopt");
    if (setsockopt(s_sock, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        perror("Error");
    }
    else{
        printf("\nSuccess setsockopt");
    }
    for (;;) /* Run forever */
    {
        printf("\nIn for loop");
        /* Set the size of the in-out parameter */
        s_cliAddrLen = sizeof(s_echoClntAddr);

        /* Block until receive message from a client */
        // if ((s_recvMsgSize = recvfrom(s_sock, s_echoBuffer, ECHOMAX, 0,
        //     (struct sockaddr *) &s_echoClntAddr, &s_cliAddrLen)) < 0)
        //     DieWithError("recvfrom() failed");
        if (recvfrom(s_sock, s_echoBuffer, ECHOMAX, 0,
            (struct sockaddr *) &s_echoClntAddr, &s_cliAddrLen) < 0)
            DieWithError("2recvfrom() failed");


        // printf("Handling client %s\n", inet_ntoa(s_echoClntAddr.sin_addr));
        // Host commented out
        /* Send received datagram back to the client */
        // if (sendto(s_sock, s_echoBuffer, s_recvMsgSize, 0, 
             // (struct sockaddr *) &s_echoClntAddr, sizeof(s_echoClntAddr)) != s_recvMsgSize)
            // DieWithError("sendto() sent a different number of bytes than expected");

        //v****************client**********************
        /* Send the string to the server */
        // if (sendto(c_sock, c_echoString, c_echoStringLen, 0, (struct sockaddr *)
        //            &c_echoServAddr, sizeof(c_echoServAddr)) != c_echoStringLen)
        //     DieWithError("2sendto() sent a different number of bytes than expected");
        //^****************client**********************
        // Alternative, passing msg from server buffer to client buffer out
        // if (sendto(c_sock, s_echoBuffer, s_recvMsgSize, 0, 
        //      (struct sockaddr *) &c_echoServAddr, sizeof(c_echoServAddr)) != s_recvMsgSize)
        //     DieWithError("3sendto() sent a different number of bytes than expected");
        // else{
        //     printf("Sending message from client?:%s",s_echoBuffer);
        // }
        // while (1){      // Keep sending periodically indefinitely every 2 seconds
            // sleep(5);   // Wait 2 seconds before sending again
            // if (tries < MAXTRIES)      /* incremented by signal handler */
            // {
                // printf("\nSending periodic message...");
                // // printf("timed out, %d more tries...\n", MAXTRIES-tries);
                // if (sendto(c_sock, s_echoBuffer, s_recvMsgSize, 0, (struct sockaddr *)
                //             &c_echoServAddr, sizeof(c_echoServAddr)) != s_recvMsgSize)
                //     DieWithError("sendto() failed");
            // } 
            // else
                // DieWithError("No Response");
        // }
        printf("\nout of while");
    }
    /* NOT REACHED */
}

void CatchAlarm(int ignored)     /* Handler for SIGALRM */
{
    tries += 1;
}
