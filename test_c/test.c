/*
 * test.c
 *
 *  Created on: Mar 16, 2022
 *      Author: bergma
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define QUEUE_ELEMENT_SIZE 50
#define QUEUE_SIZE 10

// A structure to represent a queue
struct Queue
{
	int front, rear, size;
	unsigned capacity;
	char array[QUEUE_SIZE][QUEUE_ELEMENT_SIZE];
};

struct Queue *queue;

// function to create a queue
// of given capacity.
// It initializes size of queue as 0
struct Queue* createQueue(unsigned capacity)
{
	queue = (struct Queue*) malloc(sizeof(struct Queue));
	queue->capacity = capacity;
	queue->front = queue->size = 0;

	// This is important, see the enqueue
	queue->rear = capacity - 1;
	//queue->array = (char**) malloc(queue->capacity * sizeof(char));
	//int i = 0;
	//int N = queue->capacity;
	//queue->array[i] = (char*)malloc(QUEUE_ELEMENT_SIZE * sizeof(char));

	/*
	for (int i = 0; i < queue->capacity; i++)
	{
		queue->array[i] = (char*)malloc(QUEUE_ELEMENT_SIZE * sizeof(char));
	}
	*/

	return queue;
}

// Queue is full when size becomes
// equal to the capacity
int isFull(struct Queue *queue)
{
	return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isEmpty(struct Queue *queue)
{
	return (queue->size == 0);
}

// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue *queue, char *item)
{
	if (isFull(queue))
	{
		queue->front = (queue->front + 1) % queue->capacity;
	}
	queue->rear = (queue->rear + 1) % queue->capacity;
	strcpy(queue->array[queue->rear], item);
	queue->size = queue->size + 1;
	printf("%s enqueued to queue\n", item);
}

// Function to remove an item from queue.
// It changes front and size
char* dequeue(struct Queue *queue)
{
	if (isEmpty(queue))
	{
		return "";
	}
	char *item = queue->array[queue->front];
	queue->front = (queue->front + 1) % queue->capacity;
	queue->size = queue->size - 1;
	return item;
}

// Function to get front of queue
char* front(struct Queue *queue)
{
	if (isEmpty(queue))
	{
		return "";
	}
	return queue->array[queue->front];
}

// Function to get rear of queue
char* rear(struct Queue *queue)
{
	if (isEmpty(queue))
	{
		return "";
	}
	return queue->array[queue->rear];
}

void handler(int sig)
{
	char str[] = "Disconnecting mqtt client....goodbye\n";
	write(STDERR_FILENO, str, strlen(str));
	/********** Disconnect & destroy *********/

	while (!isEmpty(queue))
	{
		printf("%s\n", dequeue(queue));
	}

	//free(queue->array);

	exit(EXIT_SUCCESS);

}

// Driver program to test above functions./
int main()
{
	struct sigaction act;

	// Trap SIGINT : delivered on user pressing ^C
	memset(&act, 0, sizeof(act));
	act.sa_handler = handler;

	if (sigaction(SIGINT, &act, NULL) == -1)
		printf("sigaction SIGINT failed\n");

	queue = createQueue(10);

	enqueue(queue, "kø1");
	enqueue(queue, "kø2");
	enqueue(queue, "kø3");
	enqueue(queue, "kø4");

	printf("%s dequeued from queue\n\n", dequeue(queue));

	printf("Front item is %s\n", front(queue));
	printf("Rear item is %s\n", rear(queue));

	char str[50];
	while (1)
	{
		scanf("%s", str);
		//printf("%s\n", str);
		enqueue(queue, str);
	}

	return 0;
}
