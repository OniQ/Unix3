#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "1992"
#define BACKLOG 10
#define MAXDATASIZE 1024
#define DEBUG_PREFIX "------------->"

void *get_in_addr(struct sockaddr *sa);

#endif
