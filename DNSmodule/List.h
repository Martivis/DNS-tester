#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdio.h>
struct Node
{
	char* data;
	size_t dataSize;
	struct Node* next;
};

struct Iterator
{
	struct Node* begin;
	struct Node* ptr;
};

struct Node* add_to_head(char* data, size_t dataSize, struct Node* next);

void add_after(char* data, size_t dataSize, struct Node* prev);

void clear_list(struct Node* list);

struct Node* list_begin(struct Node* list);

struct Iterator* iterator_create(struct Node* list);

struct Node* iterator_value(struct Iterator* it);

void iterator_rewind(struct Iterator* it);

void iterator_advance(struct Iterator* it);

#endif
