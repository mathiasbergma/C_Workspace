/*
 * queue.h
 *
 *  Created on: Mar 16, 2022
 *      Author: bergma
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define QUEUE_ELEMENT_SIZE	200
#define QUEUE_SIZE 			10

// A structure to represent a queue
struct Queue
{
	int front, rear, size;
	unsigned capacity;
	char array[QUEUE_SIZE][QUEUE_ELEMENT_SIZE];
};

// function to create a queue
// of given capacity.
// It initializes size of queue as 0
struct Queue* createQueue(unsigned capacity);

// Queue is full when size becomes
// equal to the capacity
int isFull(struct Queue *queue);

// Queue is empty when size is 0
int isEmpty(struct Queue *queue);

// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue *queue, char *item);

// Function to remove an item from queue.
// It changes front and size
char* dequeue(struct Queue *queue);

// Function traverses all items and return 2d array
// without dequeueing
char* traversequeue(struct Queue *queue, char * text, int destructive);

// Function to get front of queue
char* front(struct Queue *queue);


// Function to get rear of queue
char* rear(struct Queue *queue);

// Function to empty the queue - disregarding data
void emptyqueue(struct Queue *queue);

extern struct Queue *queue;


#endif /* QUEUE_H_ */
