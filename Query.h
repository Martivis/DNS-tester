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
	char qName[256];
	uint16_t qType;
	uint16_t qClass;
};

struct DNSQuery
{
	struct DNSHeader header;
	struct DNSQuestion question;
};

char* form_DNS_query(struct DNSQuery* query)
{ // Запись DNS запроса в буфер
	size_t headerSize = sizeof(query->header);
	size_t questionSize = sizeof(query->question);
	char* result = (char*)malloc(headerSize + questionSize);

	memcpy(result, &(query->header), headerSize);					// Запись шапки запроса в буфер
	memcpy(result + headerSize, &(query->question), questionSize);	// Запись секции запроса в буфер

	return result;
}

void read_DNS_query(char* source, struct DNSQuery* query)
{ // Чтение DNS запроса из буфера
	size_t headerSize = sizeof(query->header);
	memcpy(&(query->header), source, headerSize);								// Чтение шапки из буфера
	memcpy(&(query->question), source + headerSize, sizeof(query->question));	// Чтений секции запроса из буфера
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