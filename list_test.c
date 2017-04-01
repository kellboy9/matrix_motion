#include "list.h"
#include <string.h>

typedef struct { int i; } IntCarrier;

int main(int argc, char **argv) {
	List *mylist = new_list();

	printf("populating list...\n");
	for(int i = 0; i < 10; ++i) {
		IntCarrier *car = malloc(sizeof(IntCarrier));
		car->i = i;
		add_item(mylist, car);

	}
	printf("size: %zu\n", size(mylist));
	
	printf("traversing list...\n");
	Node *ptr = NULL;
	while(ptr = next(mylist, ptr)) {
		printf("checking item...\n");
		IntCarrier *cur = item(ptr);
		printf("accessing item...\n");
		printf("%d\n", cur->i);
	}

	printf("freeing list...\n");
	ptr = NULL;
	while(ptr = next(mylist, ptr)) {
		IntCarrier *cur = item(ptr);	
		free(cur);
	}
	free_list(mylist);

	return 0;
}
