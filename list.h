#ifndef __LIST__
#define __LIST__


typedef struct List{
    char data[1024];
    struct List* next;
} list;

typedef void (*callback)(list* data);

list* create(char *data,list* next);
list* prepend(list* head,char *data);
void traverse(list* head,callback f);
int count(list *head);

#endif