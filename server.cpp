#include <iostream>      /* for printf() and fprintf() */
#include <cstdlib>     /* for atoi() and exit() */
#include <cstring>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <vector>
#include <string>

#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */

// represents a registered user
struct user {
    std::string name;
    std::string ip;
    int port;
};

// instant message to send out
struct imessage {
    std::string msg;
    std::string author;
};

// stores a list of users in a im group
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

// copies string in char arr    // redundant but i am not fixing it
void setString(char* rtmsg, std::string* msg) {
    strcpy(rtmsg, msg->c_str());
}

// counts the amount of parameters in a command
// assumes command name is not a parameter
int paramCount(char* str) {
    int count = 0;
    for(int i = 0; i < strlen(str); i++) {
        if(str[i] == ' ') {
            count++;
        }
    }
    return count;
}

void param(std::string* str, std::string* cmd, int param) {
    int ix = -1;
    for(int i = 0; i < param; i++) {
        ix = cmd->find(' ', ix + 1);
    }
    ix++;
    int ixNext = cmd->find(' ', ix);
    if(ixNext == -1) {
        *str = cmd->substr(ix);
    } else {
        *str = cmd->substr(ix, ixNext - ix);
    }
}

// registers a user in the server
void registerUser(char* rtmsg, char* msg, std::vector<user*>* database) {
    std::string rt;

    // check parameter count
    if(paramCount(msg) != 3) {
        rt = "INVALID PARAMETERS";
        setString(rtmsg, &rt);
        return;
    }

    // receive parameters
    std::string msgStr = std::string(msg);
    std::string cname, ip, port;
    param(&cname, &msgStr, 1);
    param(&ip, &msgStr, 2);
    param(&port, &msgStr, 3);

    // check if name is registered already
    for(int i = 0; i < database->size(); i++) {
        if((*database)[i]->name == cname) {
            rt = "FAILURE";
            setString(rtmsg, &rt);
            return;
        }
    }

    // register user
    user* usr = new user;
    usr->name = cname;
    usr->ip = ip;
    usr->port = std::stoi(port);
    // add to database
    database->push_back(usr);

    // success
    rt = "SUCCESS";
    setString(rtmsg, &rt);
}

void createList(char* rtmsg, char* msg, std::vector<contact_list*>* contact_lists) {
    std::string rt;

    // check parameter count
    if(paramCount(msg) != 1) {
        rt = "INVALID PARAMETERS";
        setString(rtmsg, &rt);
        return;
    }

    // receive parameter
    std::string msgStr = std::string(msg);
    std::string clName;
    param(&clName, &msgStr, 1);
    std::cout << clName << std::endl;

    // check if name is already in use
    for(int i = 0; i < contact_lists->size(); i++) {
        if((*contact_lists)[i]->name == clName) {
            rt = "FAILURE";
            setString(rtmsg, &rt);
            return;
        }
    }

    // create contact list
    contact_list* cl = new contact_list;
    cl->name = clName;
    // add to contact lists
    contact_lists->push_back(cl);

    // success
    rt = "SUCCESS";
    setString(rtmsg, &rt);
}

void queryLists(char* rtmsg, char* msg, std::vector<contact_list*>* contact_lists) {
    std::string rt;

    // check parameter count
    if(paramCount(msg) != 0) {
        rt = "INVALID PARAMETERS";
        setString(rtmsg, &rt);
        return;
    }

    // construct return string
    rt.append(std::to_string(contact_lists->size()));
    for(int i = 0; i < contact_lists->size(); i++) {
        rt.append(" ");
        rt.append((*contact_lists)[i]->name);
    }

    setString(rtmsg, &rt);
}

void joinList(char* rtmsg, char* msg, std::vector<contact_list*>* contact_lists) {
    std::string rt = "join list";
    setString(rtmsg, &rt);
}

void leaveList(char* rtmsg, char* msg, std::vector<contact_list*>* contact_lists) {
    std::string rt = "leave list";
    setString(rtmsg, &rt);
}

void exitServer(char* rtmsg, char* msg, std::vector<contact_list*>* contact_lists) {
    std::string rt = "exit server";
    setString(rtmsg, &rt);
}

void imStart(char* rtmsg, char* msg, std::vector<user*>* database, std::vector<contact_list*>* contact_lists) {
    std::string rt = "im start";
    setString(rtmsg, &rt);
}

void imComplete(char* rtmsg, char* msg, std::vector<contact_list*>* contact_lists) {
    std::string rt = "im complete";
    setString(rtmsg, &rt);
}

void save(char* rtmsg, char* msg, std::vector<user*>* database, std::vector<contact_list*>* contact_lists) {
    std::string rt = "save";
    setString(rtmsg, &rt);
}

// possible server commands that can be received
void execute(char* rtmsg, char* msg, struct sockaddr* clientAddr, std::vector<user*>* database, std::vector<contact_list*>* contact_lists) {
    // receive command
    std::string msgStr = std::string(msg);
    int firstSpc = msgStr.find(' ', 0);
    std::string command;
    if(firstSpc == -1) {
        command = msgStr.substr(0, msgStr.size()-1);
    } else {
        command = msgStr.substr(0,firstSpc);
    }

    // execute proper command
    if(command == "register") {
        registerUser(rtmsg, msg, database);
    } else if(command == "create") {
        createList(rtmsg, msg, contact_lists);
    } else if(command == "query-lists") {
        queryLists(rtmsg, msg, contact_lists);
    } else if(command == "join") {
        joinList(rtmsg, msg, contact_lists);
    } else if(command == "leave") {
        leaveList(rtmsg, msg, contact_lists);
    } else if(command == "exit") {
        exitServer(rtmsg, msg, contact_lists);
    } else if(command == "im-start") {
        imStart(rtmsg, msg, database, contact_lists);
    } else if(command == "im-complete") {
        imComplete(rtmsg, msg, contact_lists);
    } else if(command == "save") {
        save(rtmsg, msg, database, contact_lists);
    } else {
        std::string rt = "Invalid Command";
        setString(rtmsg, &rt);
    }
}

int main(int argc, char* argv[]) {
    const int RECVMAX = 1000;       // longest message to receive
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

    struct sockaddr_in clientAddr;              // client address
    unsigned int clientAddrLen;                 // client address length

    int rcvMsgSize;                             // received message size
    char buffer[RECVMAX];                       // message buffer
    char ackBuff[RECVMAX];                      // ack buffer
    std::vector<user*> database;                // list of users registered
    std::vector<contact_list*> contact_lists;   // all contact lists created
    // always on server loop
    for(;;) {
        clientAddrLen = sizeof(clientAddr);
        // receive message
        if ((rcvMsgSize = recvfrom(sock, buffer, RECVMAX, 0, (struct sockaddr *) &clientAddr, &clientAddrLen)) < 0) {
            DieWithError("rcvfrom() failed\n");
        }

        // check buffer
        buffer[rcvMsgSize] = '\0';
        std::cout << "Client-Sent: " << buffer;
        // execute server command
        execute(ackBuff, buffer, (struct sockaddr*) &clientAddr, &database, &contact_lists);

        // send ack
        if(sendto(sock, ackBuff, strlen(ackBuff), 0, (struct sockaddr*) &clientAddr, sizeof(clientAddr)) != strlen(ackBuff)) {
            DieWithError("sendto() sent a different number of bytes than expected\n");
        }
    }

    // close server
    close(sock);
    exit(0);
}