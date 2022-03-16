/*
 * queue.c
 *
 *  Created on: Mar 16, 2022
 *      Author: bergma
 */

#include "queue.h"

struct Queue *queue;

// function to create a queue
// of given capacity.
// It initializes size of queue as 0
struct Queue* createQueue(unsigned capacity)
{
	struct Queue *queue = (struct Queue*) malloc(sizeof(struct Queue));
	queue->capacity = capacity;
	queue->front = queue->size = 0;

	// This is important, see the enqueue
	queue->rear = capacity - 1;
	//queue->array = (char**) malloc(queue->capacity * QUEUE_ELEMENT_SIZE * sizeof(char));
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
		queue->size -= 1;
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
		return NULL;
	}
	char *item = queue->array[queue->front];
	queue->front = (queue->front + 1) % queue->capacity;
	queue->size = queue->size - 1;
	return item;
}

// Function traverses all items and returns 2d array
// without dequeueing
char* traversequeue(struct Queue *queue, char *text, int destructive)
{
	if (isEmpty(queue))
	{
		return "";
	}

	// Allocate memory in text pointer
	text = (char*) malloc(queue->size * QUEUE_ELEMENT_SIZE * sizeof(char));
	memset(text,0,queue->size * QUEUE_ELEMENT_SIZE * sizeof(char));

	if (destructive)
	{
		while (!isEmpty(queue))
		{
			strcat(text, dequeue(queue));
			strcat(text, "\n");
		}
	}
	else
	{
		for (int pos = 0; pos < queue->size; pos++)
		{
			strcat(text, queue->array[(queue->front + pos) % queue->capacity]);
			strcat(text, "\n");
		}
	}

	return text;
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
void emptyqueue(struct Queue *queue)
{
	queue->rear = queue->capacity - 1;
	queue->front = queue->size = 0;
	return;
}

