#define _GNU_SOURCE
#include "DNSmodule/Query.h"
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

int main()
{	
		int RECV_BUFF_SIZE = 33554432;
		int mainSocket = socket(AF_INET, SOCK_DGRAM, 0);

		if (mainSocket < 0 ||
			fcntl(mainSocket, F_SETFL, O_NONBLOCK) == -1 ||				// Set socket to non-block mode)
			setsockopt(mainSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&RECV_BUFF_SIZE, sizeof(RECV_BUFF_SIZE))	== -1) // Increase recv buffet size
        {
			printf("Error\n");
			perror("bind");
			perror("socket");
		}
        else
		{

			struct Node* list = create_DNS_list("dns.txt");
			struct Iterator* it = iterator_create(list);

			struct sockaddr_in tgAddr;	// Address of the target
			tgAddr.sin_family = AF_INET;
			tgAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // Localhost address
			tgAddr.sin_port = htons(2556);

			//** thread
			int mmsgVecLen = 5;

			struct mmsghdr* mmsgVec = (struct mmsghdr*)malloc(mmsgVecLen * sizeof(struct mmsghdr));
			
			struct mmsghdr* recvMmsgVec = (struct mmsghdr*)malloc(mmsgVecLen * sizeof(struct mmsghdr));

			clock_t timer = 0;
			
			// Stats
			uint64_t recvdCount = 0;
			uint64_t sentCount = 0;


			for (size_t i = 0; i < mmsgVecLen; i++)
			{

				mmsgVec[i].msg_hdr.msg_name = &tgAddr;			// Put target address to sendmsg header
				mmsgVec[i].msg_hdr.msg_namelen = sizeof(tgAddr);
				mmsgVec[i].msg_hdr.msg_iov = (struct iovec*)malloc(sizeof(struct iovec));
				mmsgVec[i].msg_hdr.msg_iovlen = 1;
				mmsgVec[i].msg_hdr.msg_control = NULL;
				mmsgVec[i].msg_hdr.msg_controllen = 0;
				mmsgVec[i].msg_hdr.msg_flags = 0;


				recvMmsgVec[i].msg_hdr.msg_name = NULL;			// Put target address to sendmsg header
				recvMmsgVec[i].msg_hdr.msg_namelen = 0;
				recvMmsgVec[i].msg_hdr.msg_iov = (struct iovec*)malloc(sizeof(struct iovec));
				recvMmsgVec[i].msg_hdr.msg_iovlen = 1;
								  
				recvMmsgVec[i].msg_hdr.msg_iov->iov_base = malloc(sizeof(struct DNSQuery));
				recvMmsgVec[i].msg_hdr.msg_iov->iov_len = sizeof(struct DNSQuery);
								  
				recvMmsgVec[i].msg_hdr.msg_control = NULL;
				recvMmsgVec[i].msg_hdr.msg_controllen = 0;
				recvMmsgVec[i].msg_hdr.msg_flags = 0;
			}


			while (clock() < 10000000)
			{	
				if (clock() - timer > 100)
				{
					timer = clock();

					for (size_t i = 0; i < mmsgVecLen; i++)
					{
						mmsgVec[i].msg_hdr.msg_iov->iov_base = it->ptr->data;
						mmsgVec[i].msg_hdr.msg_iov->iov_len = it->ptr->dataSize;
						iterator_advance(it);
					}
		
		//struct DNSQuery q;
		//debuff_DNS_header(&q.header, it->ptr->data);
		//debuff_DNS_question(&q.question, it->ptr->data + sizeof(struct DNSHeader));
		//printf("Sent %lu bytes:\n", msgSize);
		//print_query(&q);


					int sentMsgCount = sendmmsg(mainSocket, mmsgVec, mmsgVecLen, 0);
					if (sentMsgCount)
						sentCount += sentMsgCount;

					int recvMsgCount = recvmmsg(mainSocket, recvMmsgVec, mmsgVecLen, 0, NULL);
                    if (recvMsgCount > 0)
					{
						//struct DNSHeader recieverHeader;
						recvdCount += recvMsgCount;
						//printf("\n\n\nRecieved:\n");
						//debuff_DNS_header(&recieverHeader, recvBuff);
						//print_header(&recieverHeader);
					}
                }
			}
			uint16_t buffederMsgsCount = 0;
			int tmp = 0;
			do
			{
				tmp = recvmmsg(mainSocket, recvMmsgVec, mmsgVecLen, MSG_WAITALL, NULL);
				if (tmp > 0)
					buffederMsgsCount += tmp;
			} while (clock() - timer < 1000000);

			for (size_t i = 0; i < mmsgVecLen; i++)
			{
				free(mmsgVec[i].msg_hdr.msg_iov);
				free(recvMmsgVec[i].msg_hdr.msg_iov->iov_base);
				free(recvMmsgVec[i].msg_hdr.msg_iov);
			}
			free(mmsgVec);
			free(recvMmsgVec);

			//*** /thread

			clear_list(it->begin);
			free(it);
			printf("Sent: %lu\nRecieved: %lu\n", sentCount, recvdCount);
			printf("Recieved total: %lu\n", buffederMsgsCount + recvdCount);
			
		}
		
    close(mainSocket);  
	printf("Program ended\n");
	return 0;
}
