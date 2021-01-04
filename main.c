#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <pthread.h>

#define BUFFER_LEN 5

pthread_mutex_t lock;	// Controls entry into the critical region
pthread_cond_t isFull;	// How many positions are occupied in the buffer
pthread_cond_t isEmpty; // How many positions are empty in the buffer

typedef struct s_buff
{
	int buffer_data[BUFFER_LEN];
	int lastPosition; // Last position
} Buffer;

int isBufferFull; // We created this variable to control when the buffer is full or not

void *producer(void *buffer)
{
	printf("The current producer thread is = %d\n", pthread_self());
	printf("[P - %d] Starting to produce...\n", pthread_self());

	Sleep(2000); // Time for other threads to be created as well

	srand(time(NULL));

	int producedItems = 0;

	while (1)
	{
		int aleatoryPosition = rand() % BUFFER_LEN;
		int aleatoryNumber = rand() % 20 + 1;

		pthread_mutex_lock(&lock); // Asks permission to access the critical region

		while (isBufferFull == 1) // While the buffer is full, it will wait for the consumer thread to empty that buffer
		{
			printf("\n\n[P - %d] Full buffer. Awaiting ...\n\n", pthread_self());
			pthread_cond_wait(&isEmpty, &lock);
		}

		if (((Buffer *)buffer)->buffer_data[aleatoryPosition] == 0)
		{
			((Buffer *)buffer)->buffer_data[aleatoryPosition] = aleatoryNumber;
			printf("[P - %d] Producing number %d in position %d of the buffer\n", pthread_self(), aleatoryNumber, aleatoryPosition);

			int j;
			int filledPositions = 0;
			printf("\n\n [");
			for (j = 0; j < BUFFER_LEN; j++)
			{
				printf(" - %d ", ((Buffer *)buffer)->buffer_data[j]);

				if (((Buffer *)buffer)->buffer_data[j] != 0)
				{
					filledPositions++;
				}
			}
			printf("] \n\n");

			if (filledPositions == 5)
			{
				isBufferFull = 1;
			}

			producedItems++;
		}

		pthread_mutex_unlock(&lock);  // Stop accessing the critical region
		pthread_cond_signal(&isFull); // Sends "full" signal to all threads
	}
}

void *consumer(void *buffer)
{
	printf("The current consumer thread is = %d\n", pthread_self());
	printf("[C - %d] Starting to consume...\n", pthread_self());
	Sleep(50);

	srand(time(NULL));

	int consumedItems = 0;

	while (1)
	{
		int aleatoryPosition = rand() % BUFFER_LEN;
		int numberConsumed = 0;

		pthread_mutex_lock(&lock); // Asks permission to access the critical region

		while (isBufferFull == 0) // While the buffer is empty, it will wait for the producer thread to fill that buffer
		{
			printf("\n\n[C - %d] Buffer vazio. Aguardando ...\n\n", pthread_self());
			pthread_cond_wait(&isFull, &lock);
		}

		numberConsumed = (int *)((Buffer *)buffer)->buffer_data[aleatoryPosition]; // Takes a number in a random buffer position

		if (numberConsumed != 0) // If that number is not zero, it consumes
		{
			((Buffer *)buffer)->buffer_data[aleatoryPosition] = 0;
			printf("[C - %d] Consuming  number %d in position %d of the buffer\n", pthread_self(), numberConsumed, aleatoryPosition);

			int j;
			int consumedPositions = 0;

			printf("\n\n [");
			for (j = 0; j < BUFFER_LEN; j++)
			{
				printf(" - %d ", ((Buffer *)buffer)->buffer_data[j]);

				if (((Buffer *)buffer)->buffer_data[j] == 0)
				{
					consumedPositions++;
				}
			}
			printf("] \n\n");

			if (consumedPositions == 5)
			{
				isBufferFull = 0;
			}

			consumedItems++;
		}

		pthread_mutex_unlock(&lock);   // Stop accessing the critical region
		pthread_cond_signal(&isEmpty); // Sends "empty" signal to all threads
	}
}

int main()
{
	// Shared buffer
	Buffer buffer;
	buffer.lastPosition = 0;

	int i;
	for (i = 0; i < 5; i++)
	{
		buffer.buffer_data[i] = 0;
	}

	int isBufferFull = 0; // Initializing the variable "boolean" with the value "false"

	// Initializing the shared variables between threads
	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&isFull, NULL);
	pthread_cond_init(&isEmpty, NULL);

	// References for Producer and Consumer
	pthread_t prod_thread;
	pthread_t cons_thread;

	// Asks the user, the number of producer threads
	int prod_quantity = 0;
	printf("How many producers threads do you want\?\n");
	scanf("%d", &prod_quantity);

	// Asks the user, the number of consumer threads
	int cons_quantity = 0;
	printf("How many consumer threads do you want\?\n");
	scanf("%d", &cons_quantity);

	// Creates producer
	for (i = 0; i < prod_quantity; i++)
	{
		printf("Creating producer\n");
		if (pthread_create(&prod_thread, NULL, producer, &buffer))
		{
			printf("Error creating Producer\n");
			return 1;
		}
	}

	// Creates consumer
	for (i = 0; i < cons_quantity; i++)
	{
		printf("Creating consumer\n");
		if (pthread_create(&cons_thread, NULL, consumer, &buffer))
		{
			printf("Error creating Consumer\n");
			return 1;
		}
	}

	pthread_join(prod_thread, NULL);
	printf("End!\n");

	return 0;
}
