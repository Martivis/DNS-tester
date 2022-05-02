#pragma once
#include <stdint.h>
// Constants for DNS

/* qr */
const uint16_t QUERY = 0;
const uint16_t REPLY = 1;

/* opcode */
//const QUERY = 0;
const uint16_t IQUERY = 1;
const uint16_t STATUS = 2;

/* rcode */
const uint16_t NOERROR = 0;			  // Compiled successfully
const uint16_t FORMERR = 1;			  // Format error
const uint16_t SERFFAIL = 2;		  // Server failed to compile DNS request
const uint16_t NXDOMAIN = 3;		  // Nonexistent domain
const uint16_t NOTIMP = 4;			  // Function not implemented
const uint16_t REFUSED = 5;			  // The server refused to answer for the query
const uint16_t YXDOMAIN = 6;		  // Name that should not exist, does exist
const uint16_t XRRSET = 7;			  // RRset that should not exist, does exist
const uint16_t NOTAUTH = 8;			  // Server not authoritable for the zone
const uint16_t NOTZONE = 9;			  // Name not in zone

/* Query type */
const uint16_t A = 0;				  // IP-address
const uint16_t NS = 1;				  // ������ ����
const uint16_t CNAME = 2;			  // ������������ ���
const uint16_t SOA = 3;				  // ������ ������ ��������
const uint16_t MB = 4;				  // ��� ������ ��������� �����
const uint16_t WKS = 5;				  // well-known service
const uint16_t PTR = 6;				  // ������ ���������
const uint16_t HINFO = 7;			  // ���������� �� ���
const uint16_t MINFO = 8;			  // ���������� � �������� ����� ��� ������ �������� �������
const uint16_t MX = 9;				  // ������ � �������� �������
const uint16_t TXT = 10;			  // �� ���������������� ������ ASCII ��������
const uint16_t AXFR = 252;			  // ������ ������� ������
const uint16_t ANY = 255;			  // ������ ���� �������

/* Classes */
const uint16_t IN = 0;				  // Internet
const uint16_t CH = 1;				  // Chaos
const uint16_t HS = 2;				  // Hesiod