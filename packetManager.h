#ifndef PM_H
#define PM_H

#define MAX_LEN_BITS 6

int validatePacket(const char* data, const int size);
int sendPacket(int socket, const char* packet, int* size);
void sendText(int socket, char* packet);


char* serializePacket (const char *packet, int size);
char* deserializePacket (const char *data, const int size, int terminateStr);
char* serializeTextPacket (const char *data);
char* deserializeTextPacket (const char *data);

#endif
