#include "utils.h"
#include "packetManager.h"

int connectToServer(char* address){
    int sockfd;  
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(address, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("Connected successfully\n");

    freeaddrinfo(servinfo);
    return sockfd;
}

typedef struct{
    char* buffer;
    int size;
} ServerData;

int getAnswer(int i, char* buf){
    int recieveResult;
    if ((recieveResult = recv(i, buf, MAXDATASIZE, 0)) <= 0) {
        if (recieveResult == 0) {
            printf("Server closed connection\n");
        } else {
            perror("recv");
        }
        close(i);
        exit(0);
    }
    return recieveResult;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }
    
    int serverSocket = connectToServer(argv[1]);
    char buf[MAXDATASIZE];
    int nBytes, valid;
    ServerData serverData = {NULL, 0};
    char *line = NULL;
    size_t lineSize;
    int result;
    while((result = getline(&line, &lineSize, stdin)) != EOF){
        strtok(line, "\n");
        sendText(serverSocket, line);
        if (strcmp("exit", line) == 0)
            break;
        do {
            nBytes = getAnswer(serverSocket, buf);
            serverData.buffer = realloc(serverData.buffer, serverData.size+nBytes);
            memcpy(serverData.buffer + serverData.size, buf, nBytes);
            serverData.size += nBytes; 
            valid = validatePacket(serverData.buffer, serverData.size);
            if (valid == -1){
                fprintf(stderr, "server sent wrong packet\n");
                close(serverSocket);
                exit(1);
            }
        }while(valid != 1);      
        char* text = deserializeTextPacket(serverData.buffer);
        printf("server:%s\n", text);
        serverData.buffer = NULL;
        serverData.size = 0;
    }
    close(serverSocket);

    return 0;
}
