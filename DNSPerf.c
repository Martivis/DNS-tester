#define _GNU_SOURCE
#include "DNSmodule/Query.h"
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>


// Statistics -------------

static uint64_t totalSent = 0;
static uint64_t totalRecieved = 0;

// /Statistics ------------

uint8_t inf_time = 0;
struct timeval progStart;

pthread_mutex_t mutex;

struct ThreadData 
{
	int sock;
	struct sockaddr_in* tgAddr;
	struct Node* list;
	int qps;
	unsigned long int finishTime;
};

void* send_messages(struct ThreadData* data);

enum argvSpec
{
	PROG_NAME, IPv4, PORT, QPS, THREAD_COUNT, TIME, DNS_FILENAME
};

int main(int argc, char* argv[])
{
	if (argc != 7)
	{	//		argv...		[0]		[1]   [2]    [3]		[4]		   [5]		[6]
		printf("Usage: ./DNS_Perf <IPv4> <port> <QPS> <thread_count> <time> <dns_filename>\n");
	}
	else
	{
		int RECV_BUFF_SIZE = 33554432;
		int mainSocket = socket(AF_INET, SOCK_DGRAM, 0);

		if (mainSocket < 0)
			perror("socket");
		else
		{
			struct sockaddr_in tgAddr;	// Address of the target

			// IPv4 setup
			if (inet_pton(AF_INET, argv[IPv4], &tgAddr.sin_addr.s_addr) <= 0) // Address
				printf("Invalid IPv4 address\n");
			else
			{
				tgAddr.sin_family = AF_INET;								  // Family
				tgAddr.sin_port = htons(atoi(argv[PORT]));					  // Port

				// Increase recv buffet size
				if (setsockopt(mainSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&RECV_BUFF_SIZE, sizeof(RECV_BUFF_SIZE)) == -1) 
					printf("Unable to increase recv buffer size. Some datagrams may be lost\n");

				// Set socket to non-blocking mode
				fcntl(mainSocket, F_SETFL, O_NONBLOCK);

				// Create list of 
				struct Node* list = create_DNS_list(argv[DNS_FILENAME]);
				if (list != NULL)
				{
					int threadCount = 1;
					if (argv[THREAD_COUNT] > 0)
						threadCount = atoi(argv[THREAD_COUNT]);					// Set thread count from argv

					int qpsPerThread = atoi(argv[QPS]) / threadCount;			// Count qps for each thread

					// Memory allocation for thread data structures and threads
					struct ThreadData* threadData = (struct ThreadData*)malloc(threadCount * sizeof(struct ThreadData));
					pthread_t* threads = (pthread_t*)malloc(threadCount * sizeof(pthread_t));

					pthread_mutex_init(&mutex, NULL);

					int workTime = atoi(argv[TIME]);
					if (workTime == 0)
						inf_time = 1;

					for (size_t i = 0; i < threadCount; i++)
					{	// Put data into structs for threads
						threadData[i].sock = mainSocket;
						threadData[i].list = list;
						threadData[i].tgAddr = &tgAddr;
						threadData[i].qps = qpsPerThread;
						threadData[i].finishTime = workTime;
					}

					struct timeval progEnd;

					gettimeofday(&progStart, NULL);
					for (size_t i = 0; i < threadCount - 1; i++)				// Begin additional threads
						pthread_create(threads + i, NULL, (void*)send_messages, (void*)(threadData + i));

					unsigned long int workSeconds, workMicroseconds;

					send_messages(threadData + threadCount - 1);				// Function for main thread


					for (size_t i = 0; i < threadCount - 1; i++)				// Finish additional threads
						pthread_join(threads[i], NULL); 

					gettimeofday(&progEnd, NULL);
					workSeconds = progEnd.tv_sec - progStart.tv_sec - 1;		// -1 to reduce time of recieving buffered and delayed answers
					workMicroseconds = progEnd.tv_usec;// - progStart.tv_usec; Why it's not working normaly?...

					printf("General:\nTarget QPS:\t%d\nEstimated QPS:\t%lu\nEstimated RPS:\t%lu\nSent:\t\t%lu\nRecieved:\t%lu\n",
						atoi(argv[QPS]), totalSent / workSeconds, totalRecieved / workSeconds, totalSent, totalRecieved); // Print stast
					printf("Work time:\t%lu s %lu us\n", workSeconds, workMicroseconds);

					pthread_mutex_destroy(&mutex);

					// Free up allocated memory
					free(threadData);
					free(threads);
					clear_list(list);
				}
			}
			close(mainSocket);

		}
	}
	return 0;
}

void* send_messages(struct ThreadData* data)
{
	int mmsgVecLen = data->qps / 1000 + 1;										// Count messages per call
	unsigned int sleepTime = 1000000 / (data->qps / mmsgVecLen);				// Count time for usleep()

	struct Iterator* it = iterator_create(data->list);

	struct mmsghdr* mmsgVec = (struct mmsghdr*)malloc(mmsgVecLen * sizeof(struct mmsghdr));
	struct mmsghdr* recvMmsgVec = (struct mmsghdr*)malloc(mmsgVecLen * sizeof(struct mmsghdr));

	// Stats
	uint64_t recvdCount = 0;
	uint64_t sentCount = 0;


	for (size_t i = 0; i < mmsgVecLen; i++)
	{
		// Send headers setup
		mmsgVec[i].msg_hdr.msg_name = data->tgAddr;
		mmsgVec[i].msg_hdr.msg_namelen = sizeof(*(data->tgAddr));
		mmsgVec[i].msg_hdr.msg_iov = (struct iovec*)malloc(sizeof(struct iovec));
		mmsgVec[i].msg_hdr.msg_iovlen = 1;
		mmsgVec[i].msg_hdr.msg_control = NULL;
		mmsgVec[i].msg_hdr.msg_controllen = 0;
		mmsgVec[i].msg_hdr.msg_flags = 0;

		// Recieve headers setup
		recvMmsgVec[i].msg_hdr.msg_name = NULL;
		recvMmsgVec[i].msg_hdr.msg_namelen = 0;
		recvMmsgVec[i].msg_hdr.msg_iov = (struct iovec*)malloc(sizeof(struct iovec));
		recvMmsgVec[i].msg_hdr.msg_iovlen = 1;

		recvMmsgVec[i].msg_hdr.msg_iov->iov_base = malloc(sizeof(struct DNSQuery));
		recvMmsgVec[i].msg_hdr.msg_iov->iov_len = sizeof(struct DNSQuery);

		recvMmsgVec[i].msg_hdr.msg_control = NULL;
		recvMmsgVec[i].msg_hdr.msg_controllen = 0;
		recvMmsgVec[i].msg_hdr.msg_flags = 0;
	}
	data->finishTime += progStart.tv_sec;
	//printf("finishTime: %lu\nprogStart: %lu\n", data->finishTime, progStart.tv_sec);
	struct timeval timer, curTime, workTimer;
	gettimeofday(&timer, NULL);
	gettimeofday(&curTime, NULL);
	gettimeofday(&workTimer, NULL);

	while (inf_time == 1 || curTime.tv_sec < data->finishTime)
	{	// Main sending/recieving loop
		gettimeofday(&curTime, NULL);
		if (curTime.tv_usec - timer.tv_usec > sleepTime)
		{
			gettimeofday(&timer, NULL);
			for (size_t i = 0; i < mmsgVecLen; i++)
			{	// Take data from query list
				mmsgVec[i].msg_hdr.msg_iov->iov_base = it->ptr->data;
				mmsgVec[i].msg_hdr.msg_iov->iov_len = it->ptr->dataSize;
				iterator_advance(it);
			}

			// Send query
			int sentMsgCount = sendmmsg(data->sock, mmsgVec, mmsgVecLen, 0);
			if (sentMsgCount)
				sentCount += sentMsgCount;	// Count sent messages

			// Recieve answers
			int recvMsgCount = recvmmsg(data->sock, recvMmsgVec, mmsgVecLen, 0, NULL);

			if (recvMsgCount > 0)
				recvdCount += recvMsgCount; // Count recieved answers
		}
	}

	gettimeofday(&curTime, NULL);
	unsigned long int workTime = curTime.tv_sec - workTimer.tv_sec;
	unsigned long int workTimeUs = curTime.tv_usec;// - workTimer.tv_usec;

	data->finishTime += 1; // Set new finish time to recieve buffered and delayed answers

	while (curTime.tv_sec < data->finishTime)
	{	// Recieve buffered and delayed answers
		gettimeofday(&curTime, NULL);
		int tmp = recvmmsg(data->sock, recvMmsgVec, mmsgVecLen, MSG_WAITALL, NULL);
		if (tmp > 0)
			recvdCount += tmp;
	}

	// Critical section
	pthread_mutex_lock(&mutex);
	totalSent += sentCount;
	totalRecieved += recvdCount;

	printf("Thread:\nWork time:\t%lu s %lu us\nTarget QPS:\t%d\nEstimated QPS:\t%ld\nEstimated RPS:\t%ld\nSent:\t\t%lu\nRecieved:\t%lu\n---------------\n",
		workTime, workTimeUs, data->qps, sentCount / workTime, recvdCount / workTime, sentCount, recvdCount);
	pthread_mutex_unlock(&mutex);

	// Free up allocated memory
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
