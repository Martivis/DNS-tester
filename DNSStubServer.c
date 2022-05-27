#include "DNSmodule/Query.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

int main()
{
	int mainSocket = socket(AF_INET, SOCK_DGRAM, 0);						// Create socket

	struct sockaddr_in addr;											// Address setup
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(2556);

	if (mainSocket < 0 || bind(mainSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{												// Error
		perror("bind");
		perror("socket");
		perror("listen");
	}
	else
	{
		printf("Server started\n");

		char buff[512] = { 0 };
		struct sockaddr clientAddr;
		socklen_t clientAddrLen = sizeof(clientAddr);
		uint64_t total = 0;
		while (1)
		{
			memset(buff, 0, sizeof(buff));
			size_t msgSize = recvfrom(mainSocket, buff, sizeof(buff), 0, &clientAddr, &clientAddrLen);
			if (msgSize > 0)
			{
				struct DNSHeader header;

				debuff_DNS_header(&header, buff);
				//print_header(&header);
				header.qr = REPLY;
				//printf("Setting flags...\n");
				if (header.rd)
					header.ra = 1;

				buff_DNS_header(&header, buff);
				int bytesSent = sendto(mainSocket, buff, msgSize, 0, &clientAddr, clientAddrLen);
				//if (bytesSent > 0)
					//printf("Recieved %lu\n", total++);
			}
		}
	}

	close(mainSocket);
}
