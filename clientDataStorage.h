#ifndef CLIENTDATA_H
#define CLIENTDATA_H

void storeClientData(int socket, char* data, int dataSize);
char* getClientData(int socket, int* dataSize);
void freeClientData(int socket);

#endif
