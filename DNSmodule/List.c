#include "List.h"

struct Node* init()
{
	struct Node* result = (struct Node*)malloc(sizeof(struct Node));
	result->data = NULL;
	result->next = NULL;
	return result;
}

void add_after(char* data, struct Node* prev)
{
	struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
	newNode->next = prev->next;
	newNode->data = data;
	prev->next = newNode;
}

void clear_list(struct Node** list)
{
	struct Node* ptr = (*list)->next;
	while (ptr != NULL)
	{
		struct Node* toDel = ptr;
		ptr = ptr->next;
		free(toDel->data);
		free(toDel);
	}
	free(*list);
	*list = NULL;
}

struct Node* list_begin(struct Node* list)
{
	return list->next;
}

struct Iterator* iterator_create(struct Node* list)
{
	struct Iterator* result = (struct Iterator*)malloc(sizeof(struct Iterator));
	result->begin = list;
	result->ptr = list->next;
	return result;
}

struct Node* iterator_value(struct Iterator* it)
{
	return it->ptr;
}

void iterator_rewind(struct Iterator* it)
{
	it->ptr = it->begin->next;
}

void iterator_advance(struct Iterator* it)
{
	if (it->ptr->next)
		it->ptr = it->ptr->next;
	else
		it->ptr = it->begin->next;
}
