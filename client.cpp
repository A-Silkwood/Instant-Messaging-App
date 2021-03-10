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
    const int RCVMAX = 10;

    int sock;                       // socket
    struct sockaddr_in serverAddr;  // server address
    unsigned short csPort;          // client-server port
    char *serverIP;                 // server IP address

    struct sockaddr_in clientAddr;  // client address
    unsigned int clientAddrLen;     // client address length
    std::string message;
    char buffer[RCVMAX+1];          // message buffer
    int respStringLen;

    // check format
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <SERVER IP ADDRESS> <SERVER PORT>\n", argv[0]);
        exit(1);
    }

    serverIP = argv[1]; // receive server IP
    csPort = atoi(argv[2]);   // receive client-server port

    printf("Connecting to %s, port %d\n", serverIP, csPort);

    // create socket
    if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        DieWithError("socket() failed\n");
    }

    // construct server address structure
    memset(&serverAddr, 0, sizeof(serverAddr));      // zero out structure
    serverAddr.sin_family = AF_INET;                    // address family
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);   // server IP address
    serverAddr.sin_port = htons(csPort);                // port

    // client side loop
    while(buffer != "EXT") {
        printf("Enter message: ");
        std::cin >> buffer;

        if(sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) != strlen(buffer)) {
            DieWithError("sendto() sent a different number of bytes than expected\n");
        }

        clientAddrLen = sizeof(clientAddr);

        if((respStringLen = recvfrom(sock, buffer, RCVMAX, 0, (struct sockaddr*) &clientAddr, &clientAddrLen)) > RCVMAX) {
            DieWithError("recvfrom() failed\n");
        }
        buffer[respStringLen] = '\0';
        printf("Server-Sent: %s\n", buffer);
    }

    // close client
    close(sock);
    exit(0);
}