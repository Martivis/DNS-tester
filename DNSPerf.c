#define _GNU_SOURCE
#include "DNSmodule/Query.h"
#include <sys/time.h>
#include <time.h>
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

pthread_mutex_t mutex;
int RECV_BUFF_SIZE = 33554432;

struct ThreadData
{
	struct sockaddr_in* tgAddr;
	struct Node* list;
	int qps;
	unsigned long int finishTime;
};

void* send_messages(struct ThreadData* data);
unsigned long int num_power(int x);

void timer_start(struct timeval* tv);
unsigned long int timer_check(struct timeval* tv);

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

		struct sockaddr_in tgAddr;	// Address of the target

		// IPv4 setup
		if (inet_pton(AF_INET, argv[IPv4], &tgAddr.sin_addr.s_addr) <= 0) // Address
			printf("Invalid IPv4 address\n");
		else
		{
			tgAddr.sin_family = AF_INET;								  // Family
			tgAddr.sin_port = htons(atoi(argv[PORT]));					  // Port



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
					threadData[i].list = list;
					threadData[i].tgAddr = &tgAddr;
					threadData[i].qps = qpsPerThread;
					threadData[i].finishTime = workTime * 1000000;
				}
				threadData[threadCount -1].qps += atoi(argv[QPS]) % threadCount;

				struct timeval totalTime;
				timer_start(&totalTime);
				for (size_t i = 0; i < threadCount - 1; i++)				// Begin additional threads
					pthread_create(threads + i, NULL, (void*)send_messages, (void*)(threadData + i));

				

				send_messages(threadData + threadCount - 1);				// Function for main thread


				for (size_t i = 0; i < threadCount - 1; i++)				// Finish additional threads
					pthread_join(threads[i], NULL);

				unsigned long int estimatedWorkTime = timer_check(&totalTime) - 1000000;

				unsigned long int workSeconds = estimatedWorkTime / 1000000;
				unsigned long int workMicroseconds = estimatedWorkTime % 1000000;

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


	}
	return 0;
}

void* send_messages(struct ThreadData* data)
{
	int mainSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (mainSocket < 0)
		perror("socket");
	else
	{
		// Increase recv buffet size
		if (setsockopt(mainSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&RECV_BUFF_SIZE, sizeof(RECV_BUFF_SIZE)) == -1)
			printf("Unable to increase recv buffer size. Some datagrams may be lost\n");

		// Set socket to non-blocking mode
		fcntl(mainSocket, F_SETFL, O_NONBLOCK);

		size_t mmsgVecLen = (size_t)num_power(data->qps);										// Count messages per call
		unsigned int sleepTime = 1000000 * mmsgVecLen / data->qps / 3;				    // Count time for usleep()
		printf("qpc: %lu\nst: %u\n", mmsgVecLen, sleepTime);
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
		//printf("finishTime: %lu\nprogStart: %lu\n", data->finishTime, progStart.tv_sec);

		struct timeval sendTimer, workTimer;
		timer_start(&workTimer);
		timer_start(&sendTimer);
		while (inf_time == 1 || timer_check(&workTimer) < data->finishTime)
		{	// Main sending/recieving loop

			for (size_t i = 0; i < mmsgVecLen; i++)
			{	// Take data from query list
				mmsgVec[i].msg_hdr.msg_iov->iov_base = it->ptr->data;
				mmsgVec[i].msg_hdr.msg_iov->iov_len = it->ptr->dataSize;
				iterator_advance(it);
			}

			// Send query
			int sentMsgCount = sendmmsg(mainSocket, mmsgVec, mmsgVecLen, 0);
			if (sentMsgCount)
				sentCount += sentMsgCount;	// Count sent messages

			// Recieve answers
			int recvMsgCount = recvmmsg(mainSocket, recvMmsgVec, mmsgVecLen, 0, NULL);

			if (recvMsgCount > 0)
				recvdCount += recvMsgCount; // Count recieved answers

			unsigned long int sendTime = timer_check(&sendTimer);
			if (sendTime < sleepTime)
				usleep(sleepTime - sendTime);
			timer_start(&sendTimer);
		}

		unsigned long int workTime = timer_check(&workTimer);
		timer_start(&workTimer);

		while (timer_check(&workTimer) < 1000000)
		{	// Recieve buffered and delayed answers
			int tmp = recvmmsg(mainSocket, recvMmsgVec, mmsgVecLen, MSG_WAITALL, NULL);
			if (tmp > 0)
				recvdCount += tmp;
		}

		// Critical section
		pthread_mutex_lock(&mutex);
		totalSent += sentCount;
		totalRecieved += recvdCount;

		printf("Thread:\nWork time:\t%lu s %lu us\nTarget QPS:\t%d\nEstimated QPS:\t%ld\nEstimated RPS:\t%ld\nSent:\t\t%lu\nRecieved:\t%lu\n---------------\n",
			workTime / 1000000, workTime % 1000000, data->qps, sentCount / (workTime / 1000000), recvdCount / (workTime / 1000000), sentCount, recvdCount);
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
		close(mainSocket);
	}
	return NULL;
}

unsigned long int num_power(int x)
{
	int result = 0;
	while (x > 10)
	{
		x /= 10;
		result++;
	}
	return result;
}

void timer_start(struct timeval* tv)
{
	gettimeofday(tv, NULL);
}
unsigned long int timer_check(struct timeval* tv)
{
	struct timeval curTime, result;
	gettimeofday(&curTime, NULL);
	result.tv_sec = curTime.tv_sec - tv->tv_sec;
	result.tv_usec = curTime.tv_usec - tv->tv_usec;
	if (result.tv_usec < 0)
	{
		result.tv_sec--;
		result.tv_usec += 1000000;
	}
	return result.tv_sec * 1000000 + result.tv_usec;
}