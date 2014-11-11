#include "utils.h"
#include "packetManager.h"
#include "clientDataStorage.h"
#include "taskManager.h"

void startServer(void);
void processClient(int socket, char* data, int dataSize);
void async_processClient(int listener, int socket, char* data, int dataSize);

int main(int argc, char* argv[]){
    startServer();
    return 0;
}

void async_processClient(int listener, int socket, char* data, int dataSize)
{
    pid_t pid;
    switch (pid = fork()) {
        case 0: // child 
            close(listener); // child doesn't need the listener
            processClient(socket, data, dataSize);
            close(socket);
            exit(0);
        case -1:
            perror("Failed to fork");
            exit(EXIT_FAILURE);
    }
}

void processClient(int socket, char* data, int dataSize){
    char* text = deserializeTextPacket(data);
#ifdef DEBUG
    printf("client %d request: %s\n", socket, text);
#endif
    char* answer = processCommand(text);
    sendText(socket, answer);
}

int setupServer(){
    struct addrinfo hints, *ai, *p;
    int yes=1;
    int rv;
    int listener;
    
    // get a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        // reuse socket
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    if (listen(listener, BACKLOG) == -1) {
        perror("listen");
        exit(3);
    }
    return listener;
}

void startServer()
{
    fd_set main_fds; 
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number
    int listener;     // listening socket descriptor
    int newfd;        // new accepted socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    char remoteIP[INET6_ADDRSTRLEN];
    signal(SIGCHLD, SIG_IGN);
    
    FD_ZERO(&main_fds);
    FD_ZERO(&read_fds);

    listener = setupServer();
    
    FD_SET(listener, &main_fds);
    fdmax = listener;

    int i;
    for(;;) {
        read_fds = main_fds;
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &main_fds);
                        if (newfd > fdmax) {
                            fdmax = newfd;
                        }
                        printf("%s connected on socket %d\n",
                            inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN),
                            newfd);
                    }
                } else {
                    // handle data from a client
                    char buf[MAXDATASIZE];
                    int recieveResult;
                    if ((recieveResult = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                        if (recieveResult == 0) {
                            printf("Client %d disconnected\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i);
                        FD_CLR(i, &main_fds);
                        freeClientData(i);
                    } else {  // we got some data from a client
                        storeClientData(i, buf, recieveResult);
                        int clientDataSize;
                        char* clientData = getClientData(i, &clientDataSize);
                        int validationResult = validatePacket(clientData, clientDataSize);
                        if (validationResult == 1){ // we got valid packet
                            async_processClient(listener, i, clientData, clientDataSize);
                            //processClient(i, request);
                            freeClientData(i);
                        }
                        else if(validationResult == -1){
                            fprintf(stderr, "client %d sent wrong packet. Disconnecting...\n", i);
                            close(i);
                            FD_CLR(i, &main_fds);
                            freeClientData(i);
                        }
                    }
                }
            }
        }
    }
}
