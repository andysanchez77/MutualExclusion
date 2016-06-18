/*
 * linkedList.h
 *
 * John Nelson - jpnelson
 */

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stddef.h>
#include <sys/time.h>

struct _llNode {
	int recipe;						//the recipe
	int ticket;						//the order number
	int expectedTime;				//the expected time to complete the recipe
	struct timeval startTime;	//the time this order was made
	struct _llNode *next;		//next node in the list
};

typedef struct _llNode llNode;

llNode *addNode(llNode *head, int _recipe, int _ticket);

llNode *removeNode(llNode *head, int _ticket);

#endif
