// client.c with the variables changed to c_<name>

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

    if ((argc < 3) || (argc > 4))    /* Test for correct number of arguments */
    {
        fprintf(stderr,"Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
        exit(1);
    }

    c_servIP = argv[1];           /* First arg:  server IP address (dotted quad) */
    c_echoString = argv[2];       /* Second arg: string to echo */

    if ((c_echoStringLen = strlen(c_echoString)) > ECHOMAX)
        DieWithError("Echo word too long");

    if (argc == 4)
        c_echoServPort = atoi(argv[3]);  /* Use given port, if any */
    else
        c_echoServPort = 7;  /* 7 is well-known port for echo service */

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

    /* Send the string to the server */
    if (sendto(c_sock, c_echoString, c_echoStringLen, 0, (struct sockaddr *)
               &c_echoServAddr, sizeof(c_echoServAddr)) != c_echoStringLen)
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
