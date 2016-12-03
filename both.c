// Merge server and client
// First, change server variables to s_<name>
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define ECHOMAX 255     /* Longest string to echo */

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

    if (argc != 2)         /* Test for correct number of parameters */
    {
        fprintf(stderr,"Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }

    s_echoServPort = atoi(argv[1]);  /* First arg:  local port */

    /* Create socket for sending/receiving datagrams */
    if ((s_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure */
    memset(&s_echoServAddr, 0, sizeof(s_echoServAddr));   /* Zero out structure */
    s_echoServAddr.sin_family = AF_INET;                /* Internet address family */
    s_echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    s_echoServAddr.sin_port = htons(s_echoServPort);      /* Local port */

    /* Bind to the local address */
    if (bind(s_sock, (struct sockaddr *) &s_echoServAddr, sizeof(s_echoServAddr)) < 0)
        DieWithError("bind() failed");
  
    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        s_cliAddrLen = sizeof(s_echoClntAddr);

        /* Block until receive message from a client */
        if ((s_recvMsgSize = recvfrom(s_sock, s_echoBuffer, ECHOMAX, 0,
            (struct sockaddr *) &s_echoClntAddr, &s_cliAddrLen)) < 0)
            DieWithError("recvfrom() failed");

        printf("Handling client %s\n", inet_ntoa(s_echoClntAddr.sin_addr));

        /* Send received datagram back to the client */
        if (sendto(s_sock, s_echoBuffer, s_recvMsgSize, 0, 
             (struct sockaddr *) &s_echoClntAddr, sizeof(s_echoClntAddr)) != s_recvMsgSize)
            DieWithError("sendto() sent a different number of bytes than expected");
    }
    /* NOT REACHED */
}
