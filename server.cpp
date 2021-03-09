#include <iostream>      /* for printf() and fprintf() */
#include <cstdlib>     /* for atoi() and exit() */
#include <cstring>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <vector>

#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */

struct message {
    int lastMsg = 0;
    std::string addr;
    std::string msg;
};

// print error and exits
void DieWithError(const char* errorMessage) {
    perror(errorMessage);
    exit(1);
}

// acknowledges client
void acknowledge(int sock, std::string* message, int msgNum, struct sockaddr_in* clientAddr) {
    // build acknowledgement message
    char* ackMsg;
    sprintf(ackMsg, "ACK%d:%s", msgNum, message);

    // send acknowledgement
    if(sendto(sock, ackMsg, strlen(ackMsg), 0, (struct sockaddr*) clientAddr, sizeof(*clientAddr)) != strlen(ackMsg)) {
        DieWithError("sendto() sent a different number of bytes than expected");
    }
}

// finds message being built in msgs
struct message* getMessage(std::vector<message*>* msgs, std::string* ip) {
    for(int i = 0; i < msgs->size(); i++) {
        if((*msgs)[i]->addr == *ip) {
            return (*msgs)[i];
        }
    }
    return NULL;
}

// handles receiving messages
void handleMessage(int sock, std::vector<message*>* msgs, struct sockaddr_in* clientAddr, int msgNum, std::string* content) {
    std::string addr = std::string(inet_ntoa((*clientAddr).sin_addr));
    struct message* msg = getMessage(msgs, &addr);
    if(msg == NULL) {
        // create message
        message* newMsg = new message;
        newMsg->addr = std::string(inet_ntoa((*clientAddr).sin_addr));
        newMsg->lastMsg = msgNum;
        newMsg->msg = *content;
        msgs->push_back(newMsg);

        acknowledge(sock, content, msgNum, clientAddr);
    } else if(msgNum = msg->lastMsg + 1) {
        // append message
        msg->msg.append(*content);
        msg->lastMsg = msgNum;

        acknowledge(sock, content, msgNum, clientAddr);
    } else if(msgNum <= msg->lastMsg) {
        // re-acknowledge
        acknowledge(sock, content, msgNum, clientAddr);
    }
}

int main(int argc, char* argv[]) {
    const int RECVMAX = 1400;       // longest message to receive
    int sock;                       // socket
    unsigned short csPort;          // client-server port
    struct sockaddr_in serverAddr;  // local address

    // check parameters
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <SERVER PORT> <SAVE FILE>\n", argv[0]);
        exit(1);
    }

    csPort = atoi(argv[1]); // receive port

    // create socket
    if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        DieWithError("socket() failed");
    }

    // construct local address structure
    memset(&serverAddr, 0, sizeof(serverAddr));   // zero out structure
    serverAddr.sin_family = AF_INET;                 // address family
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);  // get local address
    serverAddr.sin_port = htons(csPort);             // port

    // bind port to local address
    if(bind(sock, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0) {
        DieWithError("bind() failed");
    }

    printf("Port server is listening to is: %d\n", csPort);

    struct sockaddr_in clientAddr;                          // client address
    unsigned int clientAddrLen = sizeof(clientAddr);        // client address length

    int rcvMsgSize;                                         // received message size
    char buffer[RECVMAX];                                   // message buffer

    std::vector<message*> msgs;                             // collect messages
    std::string buffStr;                                    // buffer as a string
    std::string prefix;                                     // prefix command of message
    int msgNum;                                             // identifier for message
    std::string content;                                    // message content

    // always on server loop
    while(prefix != "EXT") {
        // receive message
        if((rcvMsgSize = recvfrom(sock, buffer, RECVMAX, 0, (struct sockaddr*) &clientAddr, &clientAddrLen)) < 0) {
            continue;
        }

        // check buffer
        buffer[rcvMsgSize] = '\0';
        buffStr = std::string(buffer);
        prefix = buffStr.substr(0, 3);
        msgNum = stoi(buffStr.substr(3, buffStr.find(':', 3) - 3));
        content = buffStr.substr(buffStr.find(':', 3) + 1);

        // check message type
        if(prefix == "MSG") {   // construct message
            handleMessage(sock, &msgs, &clientAddr, msgNum, &content);
        } else if(prefix == "ACK") { // receive acknowledgement

        } else {
            printf("Received invalid message");
        }
    }

    // close server
    close(sock);
    exit(0);
}