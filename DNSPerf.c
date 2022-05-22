#define _GNU_SOURCE
#include "DNSmodule/Query.h"
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>


// Statistics -------------

static uint64_t totalSent = 0;
static uint64_t totalRecieved = 0;
static time_t totalTime = 0;

// /Statistics ------------

pthread_mutex_t mutex;

struct ThreadData
{
	int sock;
	struct sockaddr_in* tgAddr;
	struct Node* list;
};

void* send_messages(struct ThreadData* data);

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
			

			struct sockaddr_in tgAddr;	// Address of the target
			tgAddr.sin_family = AF_INET;
			tgAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // Localhost address
			tgAddr.sin_port = htons(2556);

			//** thread

			int threadCount = 2;

			struct ThreadData* threadData = (struct ThreadData*)malloc(threadCount * sizeof(struct ThreadData));
			pthread_t* threads = (pthread_t*)malloc(threadCount * sizeof(pthread_t));

			pthread_mutex_init(&mutex, NULL);

			for (size_t i = 0; i < threadCount; i++)
			{
				threadData[i].sock = mainSocket;
				threadData[i].list = list;
				threadData[i].tgAddr = &tgAddr;
			}

			for (size_t i = 0; i < threadCount - 1; i++)
				pthread_create(threads + i, NULL, (void*)send_messages, (void*)(threadData + i));
			totalTime = time(0);
			send_messages(threadData + threadCount - 1);
			totalTime = time(0) - 1 - totalTime;
			for (size_t i = 0; i < threadCount - 1; i++)
				pthread_join(threads[i], NULL);

			printf("Sent: %lu\nRecieved: %lu\n", totalSent, totalRecieved);
			printf("Work time: %ld s\n", totalTime);
			pthread_mutex_destroy(&mutex);
			
			free(threadData);
			free(threads);
			//*** /thread

			clear_list(list);
			
			
			
		}
		
    close(mainSocket);  
	printf("Program ended\n");
	return 0;
}

void* send_messages(struct ThreadData* data)
{

	int mmsgVecLen = 5;

	struct Iterator* it = iterator_create(data->list);

	struct mmsghdr* mmsgVec = (struct mmsghdr*)malloc(mmsgVecLen * sizeof(struct mmsghdr));

	struct mmsghdr* recvMmsgVec = (struct mmsghdr*)malloc(mmsgVecLen * sizeof(struct mmsghdr));

	time_t finish_time = time(0) + 5;
	// Stats
	uint64_t recvdCount = 0;
	uint64_t sentCount = 0;


	for (size_t i = 0; i < mmsgVecLen; i++)
	{

		mmsgVec[i].msg_hdr.msg_name = data->tgAddr;			// Put target address to sendmsg header
		mmsgVec[i].msg_hdr.msg_namelen = sizeof(*(data->tgAddr));
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


	while (time(0) < finish_time)
	{

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


			int sentMsgCount = sendmmsg(data->sock, mmsgVec, mmsgVecLen, 0);
			if (sentMsgCount)
				sentCount += sentMsgCount;

			int recvMsgCount = recvmmsg(data->sock, recvMmsgVec, mmsgVecLen, 0, NULL);
			usleep(100);
			if (recvMsgCount > 0)
			{
				//struct DNSHeader recieverHeader;
				recvdCount += recvMsgCount;
				//printf("\n\n\nRecieved:\n");
				//debuff_DNS_header(&recieverHeader, recvBuff);
				//print_header(&recieverHeader);
			}
	}

	int bufferedMsgsCount = 0;
	finish_time += 1;

	while (time(0) < finish_time)
	{
		int tmp = recvmmsg(data->sock, recvMmsgVec, mmsgVecLen, MSG_WAITALL, NULL);
		if (tmp > 0)
			bufferedMsgsCount += tmp;
	} 

	//printf("Sent: %lu\nRecieved: %lu\n", sentCount, recvdCount);
	//printf("Recieved total: %lu\n", bufferedMsgsCount + recvdCount);

	pthread_mutex_lock(&mutex);
	totalSent += sentCount;
	totalRecieved += recvdCount + bufferedMsgsCount;
	pthread_mutex_unlock(&mutex);

	for (size_t i = 0; i < mmsgVecLen; i++)
	{
		free(mmsgVec[i].msg_hdr.msg_iov);
		free(recvMmsgVec[i].msg_hdr.msg_iov->iov_base);
		free(recvMmsgVec[i].msg_hdr.msg_iov);
	}
	free(mmsgVec);
	free(recvMmsgVec);
	free(it);
	return NULL;
}