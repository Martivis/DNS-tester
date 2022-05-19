#include "Query.h"

size_t encode_name(char* buff, char* name)
{	// Converts domain from www.example.com to 3www7example3com cstring (numbers is codes of symbols or just byte content)
	size_t i = 0; // i flows through 'name', buff is shifted (1 add byte in [0])
	while (name[i] != '\0')
	{
		uint8_t sectionSize = 0;
		size_t border = i;
		while (name[i] != '.' && name[i] != '\0')
		{
			buff[i] = name[i++];
			sectionSize++;
		}
		buff[border] = (char)sectionSize;
		i++;
	}
	buff[++i] = '\0';
	return i;
}

size_t decode_name(char* name, char* buff)
{	// Converts buffered domain form 3www7example3com0 to www.example.com cstring
	memset(name, '\0', 256);
	size_t i = 0;
	while (buff[i] != '\0')
	{
		size_t toRead = buff[i];
		if (i > 0)
			name[i - 1] = '.';
		for (size_t j = 0; j < toRead; j++, i++)
		{
			name[i] = buff[i + 1];
		}
		i++;
	}
	return i;
}

void buff_DNS_header(struct DNSHeader* header, char* buff)
{
	memcpy(buff, header, sizeof(struct DNSHeader));
}

size_t buff_DNS_question(struct DNSQuestion* question, char* buff)
{
	size_t nameSize = encode_name(buff, question->qName);
	memcpy(buff + nameSize, &question->qType, 4);
	return nameSize + 4;
}

void debuff_DNS_header(struct DNSHeader* header, char* buff)
{
	memcpy(header, buff, sizeof(struct DNSHeader));
}

void debuff_DNS_question(struct DNSQuestion* question, char* buff)
{
	size_t nameSize = decode_name(question->qName, buff);
	memcpy(&question->qType, buff + nameSize + 1, 4);
}

struct Node* create_DNS_list(const char* fileName)
{ // Creates DNS queries list in RAM using adresses from the file
	static uint16_t id = 0;

	struct Node* result = NULL; // Create list

	FILE* file = fopen(fileName, "r");
	if (file)
	{
		while (!feof(file))
		{
			struct DNSQuery q;
			// Form query
			// Header init
			q.header.id = htons(id++);
			q.header.qr = QUERY;
			q.header.opcode = htons(SQUERY);
			q.header.aa = 0;
			q.header.tc = 0;
			q.header.rd = 0;
			q.header.ra = 0;
			q.header.z = 0;
			q.header.rcode = htons(NOERROR);
			q.header.questionsCount = htons(1);
			q.header.answersCount = 0;
			q.header.authorityCount = 0;
			q.header.additionalCount = 0;
			// ------
			// Question init
			q.question.qType = read_type(file);
			q.question.qClass = htons(IN);

			memset(q.question.qName, 0, sizeof(q.question.qName));
			fscanf(file, "%s", q.question.qName);
			// ------
			//print_query(q);
			char* buff = (char*)malloc(sizeof(struct DNSQuestion));
			buff_DNS_header(&q.header, buff);
			size_t questionSize = buff_DNS_question(&q.question, buff + sizeof(struct DNSHeader));
			result = add_to_head(buff, sizeof(struct DNSHeader) + questionSize, result); // Add formed and buffered query to list head
		}
		fclose(file);
	}
	else
		printf("File error\n");

	return result;
}

uint16_t read_type(FILE* file)
{
	char* str = (char*)malloc(8);
	memset(str, '\0', 8);

	str[0] = (char)fgetc(file);
	uint8_t i = 0;
	while (str[i] != ' ')
		str[++i] = (char)fgetc(file);
	str[i] = '\0';

	uint16_t result = 0;
	if (strcmp("A", str) == 0) // It's a plug, 'cause nothing is used except of A type
		result = 0;

	return result;
}

void print_query(const struct DNSQuery* q)
{
	printf("\nid: %d\nqr: %d\nopcode: %d\naa: %d\ntc: %d\nrd: %d\nra: %d\nrcode: %d\nz: %d\nqCount: %d\naCount: %d\nauCount: %d\naddCount: %d\n\n",
		ntohs(q->header.id), q->header.qr, ntohs(q->header.opcode), q->header.aa, q->header.tc, q->header.rd, q->header.ra, ntohs(q->header.rcode),
		q->header.z, ntohs(q->header.questionsCount), ntohs(q->header.answersCount), ntohs(q->header.authorityCount), ntohs(q->header.additionalCount));
	printf("name: %s\ntype: %d\nclass: %d", q->question.qName, ntohs(q->question.qType), ntohs(q->question.qClass));
}
void print_header(const struct DNSHeader* header)
{
	printf("\nid: %d\nqr: %d\nopcode: %d\naa: %d\ntc: %d\nrd: %d\nra: %d\nrcode: %d\nz: %d\nqCount: %d\naCount: %d\nauCount: %d\naddCount: %d\n\n",
		ntohs(header->id), header->qr, ntohs(header->opcode), header->aa, header->tc, header->rd, header->ra, ntohs(header->rcode),
		header->z, ntohs(header->questionsCount), ntohs(header->answersCount), ntohs(header->authorityCount), ntohs(header->additionalCount));
}
