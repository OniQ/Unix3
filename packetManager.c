#include "utils.h"
#include "packetManager.h"

void sendText(int socket, char* packet){
    packet = serializeTextPacket(packet);
    int size = strlen(packet)+1;
    sendPacket(socket, packet, &size);
}

int sendPacket(int socket, const char *packet, int *size)
{
    int total = 0;       
    int bytesLeft = *size;
    int n;

    while(total < *size) {
        n = send(socket, packet+total, bytesLeft, 0);
        if (n == -1) { break; }
        total += n;
        bytesLeft -= n;
    }

    *size = total;

    return n==-1?-1:0;
}

int getPacketSize(const char* data, int* size){
    char packLenStr[MAX_LEN_BITS+1];
    int i;
    int valid = 0;
    for (i = 0; i < (*size) && i < MAX_LEN_BITS; i++){
        if (data[i] == ':'){
            valid = 1;
            break;
        }
        packLenStr[i] = data[i];
    }
    if(!valid)
        return 0;
    packLenStr[++i] = 0;
    *size -= i;
    int packetSize = atoi(packLenStr);
    return packetSize;
}

int validatePacket(const char* data, const int size){
    int bufDataSize = size;
    int packetSize = getPacketSize(data, &bufDataSize);
    if (packetSize <= 0)
        return -1;
#ifdef DEBUG
    printf(DEBUG_PREFIX);
    printf("pack size: %d, buf size %d\n", packetSize, bufDataSize);
    return (bufDataSize >= packetSize) ? 1 : 0;
#endif
}

char* serializePacket (const char *packet, int size)
{
    char packetSizeStr [MAX_LEN_BITS] = {0};
    char* serializedPacket = calloc(size, 1);
    if (serializedPacket == NULL){
        fprintf(stderr, "malloc for packet failed\n");
        exit(EXIT_FAILURE);
    }
    int i;

    sprintf(packetSizeStr, "%d", size);
    for ( i = 0; i < strlen(packetSizeStr); i++ )
        serializedPacket[i] = packetSizeStr[i];
    serializedPacket[strlen(packetSizeStr)] = ':';

    for ( i = 0; i < size; i++ )
        serializedPacket[i + (strlen(packetSizeStr) + 1)] = packet[i];
    return serializedPacket;
}

char* serializeTextPacket (const char *data){
    int size = strlen(data);
    char* text = serializePacket (data, size+1);
    return text;
}

char* deserializeTextPacket (const char *data){
    int size = strlen(data);
    char* text = deserializePacket (data, size, 1);
    return text;
}

char* deserializePacket (const char *data, const int size, int terminateStr)
{
    int tempSize = size;
    int packetSize = getPacketSize(data, &tempSize);
    int offset = size-tempSize;
    if (packetSize > 0){
        int t = terminateStr ? 1 : 0;
        char* packet = malloc(packetSize+t);
        if (packet == NULL){
            fprintf(stderr, "malloc for packet failed\n");
            exit(EXIT_FAILURE);
        }
        memcpy(packet, data+offset, packetSize);
        if (terminateStr)
            packet[packetSize] = '\0';
        return packet;
    }
    else{
        fprintf(stderr, "Failed to deserialize packet\n");
        exit(EXIT_FAILURE);
    }
}
