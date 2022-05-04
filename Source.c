#include <stdio.h>
#include "List.h"

uint16_t read_type(FILE* file);
struct Node* create_DNS_list(const char* fileName);

int main()
{	

#ifdef DEBUG
//	print_query(query);
	printf("=============\n");
#endif // DEBUG

	struct Node* list = create_DNS_list("/home/martivis/projects/DNS/dns.txt");

#ifdef DEBUG

	if (list->next != NULL)
		print_list(list);

	// test how buffer works
	char* buff = form_DNS_query(list->next->data);
	printf("\n=======\n");
	struct DNSQuery* buffedQ = read_DNS_query(buff);
	print_query(buffedQ);
	// ---------------------

	getchar();
#endif // DEBUG

	clear_list(&list);
	return 0;
}

uint16_t read_type(FILE* file)
{
	char* str = (char*)malloc(8);
	memset(str, '\0', 8);

	str[0] = fgetc(file);
	uint8_t i = 0;
	while (str[i] != ' ')
		str[++i] = fgetc(file);
	str[i] = '\0';

	uint16_t result = 0;
	if (strcmp("A", str) == 0) // It's a plug, 'cause nothing is used except A type
		result = 0;

	return result;
}

struct Node* create_DNS_list(const char* fileName)
{ // Creates DNS queries list in RAM using adresses from the file
	static uint16_t id = 0;

	struct Node* result = init(); // Create list
	struct Node* ptr = result;	  // Working pointer (list tail)

	FILE* file = fopen(fileName, "r");
	if (file != NULL)
	{
		while (!feof(file))
		{
			struct DNSQuery* q = (struct DNSQuery*)malloc(sizeof(struct DNSQuery));
			// Form query
			// Header init
			q->header.id = id++;
			q->header.qr = QUERY;
			q->header.opcode = QUERY;
			q->header.aa = 0;
			q->header.tc = 0;
			q->header.rd = 0;
			q->header.ra = 0;
			q->header.z = 0;
			q->header.rcode = NOERROR;
			q->header.questionsCount = 1;
			q->header.answersCount = 0;
			q->header.authorityCount = 0;
			q->header.additionalCount = 0;
			// ------
			// Question init
			q->question.qType = read_type(file);
			q->question.qClass = IN;

			char tmpStr[256];
			fscanf(file, "%s", tmpStr);
			q->question.qName = (char*)malloc(strlen(tmpStr));
			strcpy(q->question.qName, tmpStr);
			// ------
			//print_query(q);
			add_after(q, ptr); // Add formed query to list tail
			ptr = ptr->next;
		}
		fclose(file);
	}
	else
		printf("File error\n");

	return result;
}