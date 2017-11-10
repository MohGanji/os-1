#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "DS_Struct.h"

DS_Struct* DS_create(char *ip, char *port, DS_Struct* next)
{
    DS_Struct* new_DS_Struct = (DS_Struct*)malloc(sizeof(DS_Struct));
    if(new_DS_Struct == NULL)
    {
        print("Error creating a new DS_Struct.\n");
        exit(1);
    }
    strcpy(new_DS_Struct->ip, ip);
    strcpy(new_DS_Struct->port, port);    
    new_DS_Struct->files = NULL;
    new_DS_Struct->next = next;
    return new_DS_Struct;
}

DS_Struct* DS_prepend(DS_Struct* head, char *ip, char *port)
{
    DS_Struct *new_list = DS_create(ip, port, head);
    head = new_list;
    return head;
}
    

int DS_count(DS_Struct *head)
{
    DS_Struct *cursor = head;
    int c = 0;
    while(cursor != NULL)
    {
        c++;
        cursor = cursor->next;
    }
    return c;
}

DS_Struct* search_port(DS_Struct* head,char *port)
{
    DS_Struct *cursor = head;
    while(cursor!=NULL)
    {
        if(strcmp(cursor->port, port) == 0 )
            return cursor;
        cursor = cursor->next;
    }
    return NULL;
}

DS_Struct* add_path(DS_Struct* head, char *port, char *path)
{
    DS_Struct *found = search_port(head, port);
    if(found)
    {
        strcpy(found->path, path);
        return found;
    } 
    else
        return NULL;
}

DS_Struct* add_to_files(DS_Struct* head, char* port, char* filename)
{
    DS_Struct *found = search_port(head, port);
    if(found)
    {
        if(found->files == NULL)
            found->files = create(filename, found->files);
        else
            found->files = prepend(found->files, filename);
        return found->files;
    }
    else
        return NULL;
}

DS_Struct* find_file(DS_Struct *ds, char* filename){
    DS_Struct * found = ds;
    list *found_file = NULL;
    while(found != NULL){
        found_file = found->files;
        while(found_file != NULL){
            if(strcmp(filename, found_file->data) == 0)
                return found;
            found_file = found_file->next;
        }
        found = found->next;
    }
    return NULL;
}