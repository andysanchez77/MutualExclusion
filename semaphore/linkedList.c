/*
 * linkedList.c
 *
 * John Nelson - jpnelson
 */

#include "linkedList.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * add
 *
 * @param head A pointer to the head of the queue, NULL if new queue
 * @param _recipe The recipe number of this new order
 * @return A pointer to the head of the queue, or NULL if
 *	    memory was unable to be allocated
 */
llNode *addNode(llNode *head, int _recipe, int _ticket) {
	llNode *newNode;
	llNode *currentNode;

	newNode = (llNode *) malloc(sizeof(struct _llNode));	//allocate memory
	if(newNode == NULL) {
		return NULL;	//memory can't be allocated
	}

	//populate the fields
	newNode->recipe = _recipe;
	newNode->ticket = _ticket;
	gettimeofday(&newNode->startTime, NULL);
	
	if(_recipe == 1) {
		newNode->expectedTime = 26;
	}
	else if(_recipe == 2) {
		newNode->expectedTime = 23;
	}
	else if(_recipe == 3) {
		newNode->expectedTime = 20;
	}
	else if(_recipe == 4) {
		newNode->expectedTime = 24;
	}
	else if(_recipe == 5) {
		newNode->expectedTime = 16;
	}
	else {
		newNode->expectedTime = 0;
	}
	
	newNode->next = NULL;
	
	if(head == NULL) {
		return newNode;	//return the new queue
	}
	
	//add node to the end of the queue
	currentNode = head;
	while(currentNode->next != NULL) {
		currentNode = currentNode->next;
	}
	
	currentNode->next = newNode;

	return head;	//return the head with the added node to the queue
}

/**
 * remove
 *
 * @param head A pointer to the first node in the current linked list.
 * @return A pointer to the next node in the queue, or NULL if the queue is emptied
 */
llNode *removeNode(llNode *head, int _ticket) {
	llNode *newHead;
 	llNode *previousNode = NULL;
 	llNode *nextNode;
 	llNode *currentNode = head;
 		
 	while(currentNode != NULL) {
 		if(currentNode->ticket == _ticket) {
 			if(previousNode == NULL) {
 				newHead = currentNode->next;
 				free(currentNode);
 				return newHead;
 			}
 			else {
 				previousNode->next = currentNode->next;
				
 				free(currentNode);

 				return head;	//hasn't changed
 			}
 		}
 		else {
 			previousNode = currentNode;
 			currentNode = currentNode->next;
 		}
	}
 		
 	return head;	//couldn't find node
}





