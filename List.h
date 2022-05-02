#pragma once
#include "Query.h"

typedef struct Query data_t;

struct Node
{
	data_t* data;
	struct Node* next;
};

struct Node* init()
{
	struct Node* result = (struct Node*)malloc(sizeof(struct Node));
	result->data = NULL;
	result->next = NULL;
	return result;
}

void add_after(const data_t* data, struct Node* prev)
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

#ifdef DEBUG
void print_list(const struct Node* list)
{
	struct Node* ptr = list->next;
	while (ptr != NULL)
	{
		print_query(ptr->data);
		ptr = ptr->next;
	}
}
#endif // DEBUG
