#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#include "queue.h"

// Linked List Implementation
// Each node will contain [data, ->]
struct node {
	void *key;
	struct node *next;
};

// Keep track of first and last element of Queue
struct queue {
	struct node *first;
	struct node *last;
	int length;
};

// Create a new queue
queue_t queue_create(void){
        //queue_create - Allocate an empty queue, initialize ptrs, return NULL if fail to allocate
	struct queue *Q = (struct queue*)malloc(sizeof(struct queue));
	if (Q == NULL) {
		return NULL;
	}
	Q->first = NULL;
	Q->last = NULL;
	Q->length = 0;

	return Q;
}
// Destroy a Queue
int queue_destroy(queue_t queue){
	if (queue == NULL) {
		return -1;
	}
	if (queue->first != NULL) {
		return -1;
	}
	free(queue);

	// Return: -1 if @queue is NULL of if @queue is not empty. 0 if @queue was
 	//successfully destroyed.
	return 0;
}
// front 		  rear
// [1] [2] [3] [4]
// Takes in a queue and data as parameters
int queue_enqueue(queue_t queue, void *data){
	if (queue == NULL || data == NULL) {
		return -1;
	}
	
	// initialize and hold current node
	struct node *temp = (struct node*)malloc(sizeof(struct node));
	temp->key = data;
	temp->next = NULL;

	if (temp == NULL) {
		return -1;
	}
	// Queue was empty
	if (queue->first == NULL && queue->last == NULL) {
		queue->first = temp;
		queue->last = temp;	    
	}
	else {
		queue->last->next = temp;	// point to new enqueued node
		queue->last = temp;			// assign the last node name	      
	}
	queue->length++;
	return 0;
}

int queue_dequeue(queue_t queue, void **data){
	if ((queue == NULL) || (data == NULL)) {
		return -1;
	}
	if (queue->first == NULL) {
		return -1;
	}
	
	// Otherwise we have at least one element in our queue
	// assign this item (the value of a pointer) to @data
	*data = queue->first->key;
	// Dequeue first
	queue->first = queue->first->next;
	queue->length--;
	// As we enqueue elements, last keeps updating to be the next newest item in the queue
	// But once we dequeue all the elements, we need to set last to be NULL so that enqueue works as expected for an empty queue
	if (queue->length == 0) {
		queue->last = NULL;
	}
	return 0;
}

int queue_delete(queue_t queue, void *data){
	
	if (queue == NULL || data == NULL) {
		return -1;
	}
	struct node *temp = queue->first;
	struct node *prev = queue->first;

	while (temp != NULL) {
		// if we don't find the node, skip over and assign pointer
		if (temp->key == data) {
			prev->next = temp->next;
			temp->next = NULL;
			queue->length--;
			free(temp);
			return 0;
		}
		// if we don't find it
		prev = temp;
		temp = temp->next;
		
	}
	
	return -1;
	

}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data){

	if (queue == NULL || func == NULL) {
		return -1;
	}

	struct node *temp = queue->first;


	while(temp != NULL) {
		if (func(temp->key, arg) == 1) {
			if (data != NULL) {
				*data = temp->key;
			}
			return 0;
		}
		temp = temp->next;	
	}

 	return 0;
}

int queue_length(queue_t queue){
	if (queue == NULL) {
		return -1;
	}
	return queue->length;
}
