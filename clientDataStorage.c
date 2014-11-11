#include "utils.h"
#include "clientDataStorage.h"

void** clients = NULL;
int maxSocket = 0;

typedef struct{
    char* buffer;
    int size;
} Client;

void initClient(int socket){
    Client* client = malloc(sizeof(Client));
    if (clients == NULL){
        fprintf(stderr, "malloc for client buffer init failed\n");
        exit(EXIT_FAILURE);
    }
    client->buffer = NULL;
    client->size = 0;
    clients[socket] = client;
}

void storeClientData(int socket, char* data, int dataSize){
    if (maxSocket < socket){
        maxSocket = socket;
        clients = realloc(clients, (maxSocket+1)*sizeof(Client*));
        if (clients == NULL){
            fprintf(stderr, "realloc for new client failed\n");
            exit(EXIT_FAILURE);
        }
        initClient(socket);
    }
#ifdef DEBUG
    printf(DEBUG_PREFIX);
    printf("Client %d recieved:", socket);
    fwrite(data, dataSize, 1, stdout);
    printf("[%d]\n", dataSize);
#endif
    Client* client = clients[socket];
    //Extend client buffer
    client->buffer = realloc(client->buffer, client->size + dataSize);
    if (client->buffer == NULL){
        fprintf(stderr, "realloc for client buffer failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(client->buffer + client->size, data, dataSize);
    client->size += dataSize;
#ifdef DEBUG
    printf(DEBUG_PREFIX);
    printf("Client %d buffered:", socket);
    fwrite(client->buffer, client->size, 1, stdout);
    printf("[%d]\n", client->size);
#endif
}

char* getClientData(int socket, int* dataSize){
    Client* client = clients[socket];
    if (dataSize != NULL)
        *dataSize = client->size;
    return client->buffer;
}

void freeClientData(int socket){
    if (clients != NULL){
        Client* client = clients[socket];
        if (client->buffer != NULL)
            free(client->buffer);
        initClient(socket);
    }
}
