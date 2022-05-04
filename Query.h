#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "DNSConstants.h"

#define DEBUG

struct DNSHeader
{
	/* ====== HEADER ====== */
	uint16_t id;			  // identifyer

	/* flags */
	uint8_t qr : 1;			  // 0 - query; 1 - reply
	uint8_t opcode : 4;		  // Type of query: 0 - standart; 1 - inverse query; 2 - status
	uint8_t aa : 1;			  // Authoritative answer
	uint8_t tc : 1;			  // Truncation (message is too long and truncated)
	uint8_t rd : 1;			  // Recursion desired (client)
	uint8_t ra : 1;			  // Recursion available (server)
	uint8_t z : 3;			  // --reserved--
	uint8_t rcode : 4;		  // Resporse code: 0 - NOERROR; 1 - FORMERR; 2 - SERVFAIL; 3 - NXDOMAIN...
	/* ----- */

	uint16_t questionsCount;  // The number of questions int the Question section of the message
	uint16_t answersCount;	  // The number of resource records in the Answer section of the message
	uint16_t authorityCount;  // Authority records count
	uint16_t additionalCount; // Additional record count

};

struct DNSQuestion
{
	/* ===== QUESTION ===== */
	char* qName;
	uint16_t qType;
	uint16_t qClass;
};

struct DNSQuery
{
	struct DNSHeader header;
	struct DNSQuestion question;
};

size_t name_to_buff(char* buff, char* name)
{	// Converts domain from www.example.com to 3www7example3com cstring (numbers is codes of symbols or just byte content)
	// qName convert
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

size_t buff_to_name(char** name, char* buff)
{	// Converts buffered domain form 3www7example3com to www.example.com cstring
	char tmpStr[256];
	memset(tmpStr, '\0', 256);
	size_t i = 0;
	while (buff[i + 1] != '\0')
	{
		size_t toRead = buff[i];
		if (i > 0)
			tmpStr[i - 1] = '.';
		for (size_t j = 0; j < toRead; j++, i++)
		{
			tmpStr[i] = buff[i + 1];
		}
		i++;
	}
	*name = (char*)malloc(i);
	strcpy(*name, tmpStr);
	return i;
}

char* form_DNS_query(struct DNSQuery* query)
{ // Write DNS query to buffer
	size_t headerSize = sizeof(query->header);
	size_t questionSize = sizeof(query->question);
	char* result = (char*)malloc(headerSize + questionSize + 1);

	memcpy(result, &(query->header), headerSize);												// Write the header to buffer
	size_t nameSize = name_to_buff(result + headerSize, query->question.qName);					// Write domain name to buffer
	memcpy(result + headerSize + nameSize, &(query->question.qType), questionSize - nameSize);	// Write the rest of the question section to buffer

	return result;
}

struct DNSQuery* read_DNS_query(char* source)
{ // Read DNS query from buffer
	struct DNSQuery* result = (struct DNSQuery*)malloc(sizeof(struct DNSQuery*));
	size_t headerSize = sizeof(result->header);
	memcpy(&(result->header), source, headerSize);								// Read the header from buffer
	size_t nameSize = buff_to_name(&result->question.qName, source + headerSize);
	memcpy(&(result->question.qType), source + headerSize + nameSize + 1, sizeof(result->question) - nameSize - 1);	// Read the question section from buffer
	return result;
}

#ifdef DEBUG

void print_query(const struct DNSQuery* q)
{
	printf("\nid: %d\nqr: %d\nopcode: %d\naa: %d\ntc: %d\nrd: %d\nra: %d\nrcode: %d\nz: %d\nqCount: %d\naCount: %d\nauCount: %d\naddCount: %d\n\n",
		q->header.id, q->header.qr, q->header.opcode, q->header.aa, q->header.tc, q->header.rd, q->header.ra, q->header.rcode,
		q->header.z, q->header.questionsCount, q->header.answersCount, q->header.authorityCount, q->header.additionalCount);
	printf("name: %s\ntype: %d\nclass: %d", q->question.qName, q->question.qType, q->question.qClass);
}


#endif // DEBUG