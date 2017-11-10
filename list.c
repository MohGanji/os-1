#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

list *create(char *data,list *next)
{
    list* new_list = (list*)malloc(sizeof(list));
    if(new_list == NULL)
    {
        print("Error creating a new list.\n");
        exit(1);
    }
    strcpy(new_list->data, data);
    new_list->next = next;
    return new_list;
}

list* prepend(list* head,char *data)
{
    list *new_list = create(data,head);
    head = new_list;
    return head;
}

void traverse(list* head,callback f)
{
    list* cursor = head;
    while(cursor != NULL)
    {
        f(cursor);
        cursor = cursor->next;
    }
}
    

int count(list *head)
{
    list *cursor = head;
    int c = 0;
    while(cursor != NULL)
    {
        c++;
        cursor = cursor->next;
    }
    return c;
}
