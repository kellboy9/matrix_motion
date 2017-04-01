#ifndef LIST_H
#define LIST_H
#include <stdlib.h>
#include <stdio.h>

//a generic list of pointers that can be iterated and extended.

typedef struct list List;
typedef struct node Node;

List *new_list();
size_t size(List *list);
void add_item(List *list, void *item);
void delete_item(List *list, void *item);
//don't ask why I wrote it this way
Node *next(List *list, Node *i);
void *item(Node *i);
void free_list(List *list); //frees list and all its elements

#endif
