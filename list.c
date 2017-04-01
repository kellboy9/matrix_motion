#include "list.h"

struct list {
	size_t size;
	struct node {
		void *item;
		Node *next;
	} *head;
};

List *new_list() {
	List *new = malloc(sizeof(List));
	new->size = 0;
	new->head = NULL;
	return new;
}

size_t size(List *list) {
	return list->size;
}

void add_item(List *list, void *item) {
	Node *new = malloc(sizeof(Node));
	new->item = item;
	new->next = NULL;
	if(!list->head)	{ 
		list->head = new;
		list->size++;
		return;
	} 
	
	Node *ptr = list->head;
	while(ptr->next) {
		ptr = ptr->next;
	}

	ptr->next = new;
	list->size++;
}

void delete_item(List *list, void *item) {
	if(!list->head)	{ 
		return;
	} 
	
	Node *ptr = list->head;
	while(ptr->next) {
		if(ptr->next->item == item) {
			free(ptr->next); //free dangling reference
			ptr->next = ptr->next->next; //'skip' the deleted item
			return;
		}
		ptr = ptr->next;
	}
}

/*
 * Node *i = NULL;
 * List list = some list;
 * while(i = next(list, i)) {
 *  	//do something with item(i)
 * }
 */

//fun fact originally called "each" because it behaves similarly
Node *next(List *list, Node *i) {
	if(!i) {
		i = list->head;
	} else {
		i = i->next;
	}
	return i; //will break when i->next is NULL
}

void *item(Node *i) {
	return i->item;
}

void free_list(List *list) {
	Node *ptr = list->head;
	if(ptr){ 
		Node *next;
		while(next = ptr->next){
			free(ptr);
			ptr = next;
		}
	}
	free(list);
}
