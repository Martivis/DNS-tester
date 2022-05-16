#ifndef DNSCONSTANTS_H
#define DNSCONSTANTS_H

#include <stdint.h>
// Constants for DNS

enum QR
{	/* Type (Query / Reply) */
	QUERY = 0,
	REPLY = 1
};

enum OPCode
{	/* Query type */
	SQUERY = 0,			  // Standart
	IQUERY = 1,			  // Inverse query
	STATUS = 2,			  // Status
	NOTIFY = 4,			  // Data for zone has changed
	UPDATE = 5			  // Allows resource records to change
};

enum RCode
{	/* Response code */
	NOERROR = 0,		  // Compiled successfully
	FORMERR = 1,		  // Format error
	SERFFAIL = 2,		  // Server failed to compile DNS request
	NXDOMAIN = 3,		  // Nonexistent domain
	NOTIMP = 4,			  // Function not implemented
	REFUSED = 5,		  // The server refused to answer for the query
	YXDOMAIN = 6,		  // Name that should not exist, does exist
	XRRSET = 7,			  // RRset that should not exist, does exist
	NRRSET = 8,			  // RRset does not exist
	NOTAUTH = 9,		  // Server not authoritable for the zone
	NOTZONE = 10		  // Name not in zone
};
enum QType
{	/* Query type */
	A = 1,				  // IP-address
	NS = 2,				  // An authoritative name server
	MD = 3,				  // A mail destination (Obsolete - use MX)
	MF = 4,				  // A mail forwarder (Obsolete - use MX)
	CNAME = 5,			  // The canonical name for an alias
	SOA = 6,			  // Marks the start of a zone of authority
	MB = 7,				  // A mailbox domain name (EXPERIMENTAL)
	MG = 8,				  // A mail group member (EXPERIMENTAL)
	MR = 9,				  // A mail rename domain name (EXPERIMENTAL)
	_NULL_ = 10,		  // A null RR (EXPERIMENTAL)
	WKS = 11,			  // A well known service description
	PTR = 12,			  // a domain name pointer
	HINFO = 13,			  // host information
	MINFO = 14,			  // mailbox or mail list information
	MX = 15,			  // mail exchange
	TXT = 16,			  // text strings
	AXFR = 252,			  // A request for a transfer of an entire zone
	MAILB = 253,		  // A request for mailbox-related records (MB, MG or MR)
	MAILA = 254,		  // A request for mail agent RRs (Obsolete - see MX)
	ANY = 255			  // A request for all records
};
enum QClass
{	/* Query classes */
	IN = 1,				  // Internet
	CS = 2,				  // CSNET
	CH = 3,				  // Chaos
	HS = 4				  // Hesiod
};

#endif
