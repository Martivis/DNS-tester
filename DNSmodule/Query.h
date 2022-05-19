#ifndef QUERY_H
#define QUERY_H

#include <string.h>
#include <stdio.h>
#include <netinet/in.h>

#include "DNSConstants.h"
#include "List.h"

struct DNSHeader
{
	/* ====== HEADER ====== */
	uint16_t id;			  // identifyer

	/* flags */
	uint8_t rd : 1;			  // Recursion desired (client)
	uint8_t tc : 1;			  // Truncation (message is too long and truncated)
	uint8_t aa : 1;			  // Authoritative answer
	uint8_t opcode : 4;		  // Type of query: 0 - standart; 1 - inverse query; 2 - status
	uint8_t qr : 1;			  // 0 - query; 1 - reply

	uint8_t rcode : 4;		  // Resporse code: 0 - NOERROR; 1 - FORMERR; 2 - SERVFAIL; 3 - NXDOMAIN...
	uint8_t z : 3;			  // --reserved--
	uint8_t ra : 1;			  // Recursion available (server)
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

size_t encode_name(char* buff, char* name);
// Converts domain from www.example.com to 3www7example3com0 cstring (numbers is codes of symbols or just byte content)

size_t decode_name(char* name, char* buff);
// Converts buffered domain form 3www7example3com0 to www.example.com cstring

void buff_DNS_header(struct DNSHeader* header, char* buff);
// Write DNS header to bytes buffer

size_t buff_DNS_question(struct DNSQuestion* question, char* buff);
// Write DNS question to bytes buffer. Returns number of written bytes

void debuff_DNS_header(struct DNSHeader* header, char* buff);
// Read DNS header from bytes buffer

void debuff_DNS_question(struct DNSQuestion* question, char* buff);
// Read DNS question from bytes buffer

struct Node* create_DNS_list(const char* fileName);

uint16_t read_type(FILE* file);

void print_query(const struct DNSQuery* q);

void print_header(const struct DNSHeader* header);

#endif
