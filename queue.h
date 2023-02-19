#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#ifndef NBQUEUE_H
#define NBQUEUE_H


typedef struct QUEUE_TYPE
{
	Node * head;
}Queue;

void initQueue(Queue * queue);
void insert(Queue * ,void * value);
void rem(Queue * ,void * value);
void printQueue(Queue * queue,FILE * );
void remAll(Queue * queue,int freeValues=1);

#endif