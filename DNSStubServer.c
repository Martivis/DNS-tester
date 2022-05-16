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

	if (mainSocket < 0 ||
		bind(mainSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{												// Error
		perror("bind");
		perror("socket");
		perror("listen");
	}
	else
	{
		printf("Server started\n");

		char buff[1024] = { 0 };
		struct msghdr msgh;
		struct iovec io[1];
		struct sockaddr clientAddr;
		size_t clientAddrLen;
		msgh.msg_name = &clientAddr;

		msgh.msg_iov = io;
		msgh.msg_iov[0].iov_base = buff;
		msgh.msg_iov[0].iov_len = 1024;
		msgh.msg_iovlen = 1;

		msgh.msg_control = NULL;
		msgh.msg_controllen = 0;
		msgh.msg_flags = 0;

		while (1)
		{
			//size_t msgSize = recvfrom(mainSocket, buff, 1024, 0, &clientAddr, &clientAddrLen);
			size_t msgSize = recvmsg(mainSocket, &msgh, 0);
			//size_t msgSize = recv(mainSocket, buff, 1024, 0);
			if (msgSize >= 0)
			{
				//printf("%d\n", msgSize);
				struct DNSHeader header;

				debuff_DNS_header(&header, buff);

				print_header(&header);
				//header.qr = htons(REPLY);
				//printf("Setting flags...\n");
				//if (header.rd)
				//	header.ra = 1;

				buff_DNS_header(&header, buff);
				//sendto(mainSocket, buff, 1024, 0, &clientAddr, sizeof(clientAddr));
				sendmsg(mainSocket, &msgh, 0);
			}
		}
	}

	close(mainSocket);
}
