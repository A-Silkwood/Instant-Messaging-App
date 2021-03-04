#include <iostream>      /* for printf() and fprintf() */
#include <cstdlib>     /* for atoi() and exit() */
#include <cstring>     /* for memset() */
#include <unistd.h>     /* for close() */

#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */

// print error and exits
void DieWithError(const char *errorMessage) {
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[]) {
    const int RECVMAX = 255;        // longest message to receive

    int sock;                       // socket
    unsigned short csPort;          // client-server port
    struct sockaddr_in localAddr;   // local address


    // check format
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <UDP SERVER PORT> <SAVE FILE LOCATION>\n", argv[0]);
        exit(1);
    }

    csPort = atoi(argv[1]); // receive port

    // create socket
    if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        DieWithError("socket() failed");
    }

    // construct local address structure
    memset(&localAddr, 0, sizeof(localAddr));    // zero out structure
    localAddr.sin_family = AF_INET;                 // address family
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);  // get local address
    localAddr.sin_port = htons(csPort);             // port

    // bind port to local address
    if(bind(sock, (struct sockaddr *) &localAddr, sizeof(localAddr)) < 0) {
        DieWithError("bind() failed");
    }

    printf("Port server is listening to is: %d\n", csPort);

    struct sockaddr_in clientAddr;
    unsigned int clientAddrLen;
    char buffer[RECVMAX];
    int rcvMsgSize;

    // always on server
    #pragma clang diagnostic push
    #pragma ide diagnostic ignored "EndlessLoop"
    while(true) {
        clientAddrLen = sizeof(clientAddr);

        if((rcvMsgSize = recvfrom(sock, buffer, RECVMAX, MSG_PEEK, (struct sockaddr*) &clientAddr, &clientAddrLen)) < 0) {
            DieWithError("recvfrom() failed");
        }

        buffer[rcvMsgSize] = '\0';

        printf("RCVFROM: %s - %s", inet_ntoa(clientAddr.sin_addr), buffer);
    }
    #pragma clang diagnostic pop

    // close server
    close(sock);
    exit(0);
}