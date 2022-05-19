#include "DNSmodule/Query.h"
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

int main()
{	
		int mainSocket = socket(AF_INET, SOCK_DGRAM, 0);
		if (mainSocket < 0)
        {
			printf("Error\n");
			perror("bind");
			perror("socket");
		}
        else
		{
		    struct Iterator* it = iterator_create(create_DNS_list("dns.txt"));

			struct msghdr msgh;		// sendmsg header
			struct iovec io[1];		// buffer struct
			struct sockaddr_in tgAddr;	// Address of the target

			tgAddr.sin_family = AF_INET;
			tgAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // Localhost address
			tgAddr.sin_port = htons(2556);

			clock_t timer = 0;
			uint64_t recvd_count = 0;

            char recvBuff[sizeof(struct DNSHeader)] = {0}; // Buffer for replies

			msgh.msg_name = &tgAddr;			// Put target address to sendmsg header
			msgh.msg_namelen = sizeof(tgAddr);
			msgh.msg_iov = io;
			msgh.msg_iovlen = 1;
			msgh.msg_control = NULL;
			msgh.msg_controllen = 0;
			msgh.msg_flags = 0;

			while (clock() < 1000000)
			{	
				if (clock() - timer > 10)
				{
					timer = clock();

                    size_t msgSize = it->ptr->dataSize;
					msgh.msg_iov->iov_base = it->ptr->data;
					msgh.msg_iov->iov_len = it->ptr->dataSize;
		
		//struct DNSQuery q;
		//debuff_DNS_header(&q.header, it->ptr->data);
		//debuff_DNS_question(&q.question, it->ptr->data + sizeof(struct DNSHeader));
		//printf("Sent %lu bytes:\n", msgSize);
		//print_query(&q);

					iterator_advance(it);

					sendmsg(mainSocket, &msgh, 0);
					

					int recvMsgSize = recvfrom(mainSocket, recvBuff, sizeof(struct DNSQuery), 0, NULL, NULL);
                    if (recvMsgSize >= 0)
					{
						//struct DNSHeader recieverHeader;
						recvd_count++;
						//printf("\n\n\nRecieved:\n");
						//debuff_DNS_header(&recieverHeader, recvBuff);
						//print_header(&recieverHeader);
					}
                }
			}
			clear_list(it->begin);
			free(it);
			printf("Recieved: %lu\n", recvd_count);
		}
		
    close(mainSocket);  
	printf("Program ended\n");
	return 0;
}
