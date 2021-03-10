#include <iostream>      /* for printf() and fprintf() */
#include <cstdlib>     /* for atoi() and exit() */
#include <cstring>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <vector>
#include <string>

#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */

struct user {
    std::string name;
    std::string ip;
    int port;
};

struct imessage {
    std::string msg;
    std::string author;
};

struct contact_list {
    std::string name;
    std::vector<imessage*> imsgs;
    std::vector<user*> contacts;
};

// print error and exits
void DieWithError(const char* errorMessage) {
    perror(errorMessage);
    exit(1);
}

char* registerUser(char* msg, std::vector<user*>* database) {
    return "register";
}

char* createList(char* msg, std::vector<contact_list*>* contact_lists) {
    return "create list";
}

char* queryLists(std::vector<contact_list*>* contact_lists) {
    return "query lists";
}

char* joinList(char* msg, std::vector<contact_list*>* contact_lists) {
    return "join list";
}

char* leaveList(char* msg, std::vector<contact_list*>* contact_lists) {
    return "leave list";
}

char* exitServer(char* msg, std::vector<contact_list*>* contact_lists) {
    return "exit server";
}

char* imStart(char* msg, std::vector<user*>* database, std::vector<contact_list*>* contact_lists) {
    return "im start";
}

char* imComplete(char* msg, std::vector<contact_list*>* contact_lists) {
    return "im complete";
}

char* save(char* msg, std::vector<user*>* database, std::vector<contact_list*>* contact_lists) {
    return "save";
}

char* execute(char* msg, struct sockaddr* clientAddr, std::vector<user*>* database, std::vector<contact_list*>* contact_lists) {
    // receive command
    std::string msgStr = std::string(msg);
    int firstSpc = msgStr.find(' ', 0);
    std::string command;
    if(firstSpc == -1) {
        command = std::string(msgStr);
    } else {
        command = msgStr.substr(0,firstSpc);
    }

    // execute proper command
    if(command == "register") {
        return registerUser(msg, database);
    } else if(command == "create") {
        return createList(msg, contact_lists);
    } else if(command == "query-lists") {
        return queryLists(contact_lists);
    } else if(command == "join") {
        return joinList(msg, contact_lists);
    } else if(command == "leave") {
        return leaveList(msg, contact_lists);
    } else if(command == "exit") {
        return exitServer(msg, contact_lists);
    } else if(command == "im-start") {
        return imStart(msg, database, contact_lists);
    } else if(command == "im-complete") {
        return imComplete(msg, contact_lists);
    } else if(command == "save") {
        return save(msg, database, contact_lists);
    } else {
        return "Invalid Command";
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
        DieWithError("socket() failed\n");
    }

    // construct local address structure
    memset(&serverAddr, 0, sizeof(serverAddr));   // zero out structure
    serverAddr.sin_family = AF_INET;                 // address family
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);  // get local address
    serverAddr.sin_port = htons(csPort);             // port

    // bind port to local address
    if(bind(sock, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0) {
        DieWithError("bind() failed\n");
    }

    printf("Port server is listening to is: %d\n", csPort);

    struct sockaddr_in clientAddr;                          // client address
    unsigned int clientAddrLen = sizeof(clientAddr);        // client address length

    int rcvMsgSize;                                         // received message size
    char buffer[RECVMAX];                                   // message buffer
    char* ackBuff;                                          // ack buffer
    std::vector<user*> database;                            // list of users registered
    std::vector<contact_list*> contact_lists;               // all contact lists created
    // always on server loop
    while(true) {
        // receive message
        if ((rcvMsgSize = recvfrom(sock, buffer, RECVMAX, 0, (struct sockaddr *) &clientAddr, &clientAddrLen)) < 0) {
            DieWithError("rcvfrom() failed\n");
        }

        // check buffer
        buffer[rcvMsgSize] = '\0';
        char* ackBuff = execute(buffer, (struct sockaddr*) &clientAddr, &database, &contact_lists);

        // send ack
        if (sendto(sock, ackBuff, strlen(ackBuff), 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr)) != strlen(ackBuff)) {
            DieWithError("sendto() sent a different number of bytes than expected\n");
        }
    }

    // close server
    close(sock);
    exit(0);
}