#include <iostream>      /* for printf() and fprintf() */
#include <cstdlib>     /* for atoi() and exit() */
#include <cstring>     /* for memset() */

#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */

#include <unistd.h>     /* for close() */

// print error and exits
void DieWithError(const char *errorMessage) {
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sock;                       // socket
    unsigned short csPort;          // client-server port
    struct sockaddr_in localAddr;   // local address

    // check format
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <UDP SERVER PORT> <SAVE FILE LOCATION>\n", argv[0]);
        exit(1);
    }

    // receive port
    csPort = atoi(argv[1]);

    // create socket
    if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        DieWithError("socket() failed");
    }

    // construct local address structure
    memset(&localAddr, 0, sizeof(localAddr));    // zero out structure
    localAddr.sin_family = AF_INET;                 // address family
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);  // get local address
    printf( "Before bind, IP address on the server is: %s\n", inet_ntoa(localAddr.sin_addr.s_addr) );
    localAddr.sin_port = htons(csPort);             // port

    // bind port to local address
    if(bind(sock, (struct sockaddr *) &localAddr, sizeof(localAddr)) < 0) {
        DieWithError("bind() failed");
    }

    printf( "After bind, IP address on the server is: %s\n", inet_ntoa(localAddr.sin_addr ) );

    close(sock);
    exit(0);
}