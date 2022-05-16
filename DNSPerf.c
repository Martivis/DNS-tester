#include "DNSmodule/Query.h"
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

int main()
{	
		struct Iterator* it = iterator_create(create_DNS_list("/home/martivis/projects/DNS/dns.txt"));

		int mainSocket = socket(AF_INET, SOCK_DGRAM, 0);
		if (mainSocket >= 0)
		{
			struct msghdr msgh;
			struct iovec io[1];
			struct sockaddr_in tgAddr;

			tgAddr.sin_family = AF_INET;
			tgAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
			tgAddr.sin_port = htons(2556);

			clock_t timer = 0;
			uint64_t recvd_count = 0;
			while (clock() < 1000000)
			{
				char recvBuff[1024] = { 0 };
				
				msgh.msg_name = &tgAddr;
				msgh.msg_namelen = sizeof(tgAddr);
				msgh.msg_iov = io;
				msgh.msg_iovlen = 1;
				msgh.msg_control = NULL;
				msgh.msg_controllen = 0;
				msgh.msg_flags = 0;
				if (clock() - timer > 100000)
				{
					timer = clock();
					msgh.msg_iov->iov_base = it->ptr->data;
					msgh.msg_iov->iov_len = sizeof(*it->ptr->data);
					iterator_advance(it);

					sendmsg(mainSocket, &msgh, 0);

		struct DNSHeader header;
		debuff_DNS_header(&header, msgh.msg_iov->iov_base);
		print_header(&header);
					

					printf("sent\n");
					memset(recvBuff, 0, 1024);
					//msgh.msg_iov->iov_base = recvBuff;
					//msgh.msg_iov->iov_len = sizeof(recvBuff);
					//if (recv(mainSocket, recvBuff, 1024, 0))
					if (recvfrom(mainSocket, recvBuff, 1024, 0, NULL, NULL) > 0)
					//if (recvmsg(mainSocket, &msgh, 0))
					{
						recvd_count++;
						printf("recieved:\n");
						debuff_DNS_header(&header, recvBuff);
						print_header(&header);
					}
				}
			}
			clear_list(&it->begin);
			free(it);
			printf("Recieved: %d\n", recvd_count);
		}
		else
		{
			printf("Error\n");
			perror("bind");
			perror("socket");
		}

	printf("Program ended\n");
	getchar();

	clear_list(it->begin);
	close(mainSocket);
	return 0;
}
