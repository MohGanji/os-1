#ifndef __DS_STRUCT__
#define __DS_STRUCT__

#include "list.h"

typedef struct DS_Struct{
    char ip[30];
    char port[6];
    char path[1024];
    list *files;
    struct DS_Struct* next;
} DS_Struct;


DS_Struct* DS_create(char *ip, char *port, DS_Struct* next);
DS_Struct* DS_prepend(DS_Struct* head, char *ip, char *port);
int DS_count(DS_Struct *head);
DS_Struct* search_port(DS_Struct* head,char *port);
DS_Struct* add_path(DS_Struct* head, char* port, char* path);
DS_Struct* add_to_files(DS_Struct* head, char* port, char* filename);
DS_Struct* find_file(DS_Struct *ds, char* filename);

#endif