#ifndef __UTILS__
#define __UTILS__


#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h> 

#include "list.h"

// buffer[i] = '\0'; ==> changes buffer size, now we can only read i bytes.


#include <stdio.h> // should be removed

// file descriptors for stdin, stdout and stderr
#define STDIN 0
#define STDOUT 1
#define STDERR 2

// enum of states for client and data_server
enum STATE {Idle, /*Data:*/ Data, Port, Dir, Files, /*Client:*/ Client};


// structures.
struct Packet {
    char *data;
    int len;
};

// utility functions

void print(char* string);
void printl(char* string, int size);
void printInt(int num);
struct Packet myread(int fd, char *buffer, int len);
int mystoi(char *str);
void *get_in_addr(struct sockaddr *sa);
int mysend(int sock, char *buf, int len);    

#endif