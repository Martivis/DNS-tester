#include "List.h"

struct Node* add_to_head(char* data, size_t dataSize, struct Node* next)
{
	struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
	newNode->data = data;
	newNode->next = next;
	newNode->dataSize = dataSize;
	return newNode;
}

void add_after(char* data, size_t dataSize, struct Node* prev)
{
	struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
	newNode->next = prev->next;
	newNode->data = data;
	newNode->dataSize = dataSize;
	prev->next = newNode;
}

void clear_list(struct Node* list)
{
	struct Node* ptr = list;
	while (ptr != NULL)
	{
		struct Node* toDel = ptr;
		ptr = ptr->next;
		free(toDel->data);
		free(toDel);
	}
}

struct Node* list_begin(struct Node* list)
{
	return list;
}

struct Iterator* iterator_create(struct Node* list)
{
	struct Iterator* result = (struct Iterator*)malloc(sizeof(struct Iterator));
	result->begin = list;
	result->ptr = list;
	return result;
}

struct Node* iterator_value(struct Iterator* it)
{
	return it->ptr;
}

void iterator_rewind(struct Iterator* it)
{
	it->ptr = it->begin;
}

void iterator_advance(struct Iterator* it)
{
	if (it->ptr->next)
		it->ptr = it->ptr->next;
	else
		it->ptr = it->begin;
}
