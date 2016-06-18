/*
 *tooManyCooks.c
 *
 *John Nelson - jpnelson
 *
*/

#define TOTAL_ORDERS 30

//Recipe step times
#define R1a 3
#define R1b 4
#define R1c 2
#define R1d 2
#define R1e 5
#define R1f 10

#define R2a 5
#define R2b 3
#define R2c 15

#define R3a 10
#define R3b 5
#define R3c 5

#define R4a 15
#define R4b 5
#define R4c 4

#define R5a 2
#define R5b 3
#define R5c 2
#define R5d 2
#define R5e 3
#define R5f 4

#include "linkedList.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>

pthread_mutex_t prep_mutex;
pthread_cond_t prep_c;
int prep_occupied;

pthread_mutex_t oven_mutex;
pthread_cond_t oven_c;
int oven_occupied;

pthread_mutex_t stove_mutex;
pthread_cond_t stove_c;
int stove_occupied;

pthread_mutex_t sink_mutex;
pthread_cond_t sink_c;
int sink_occupied;

pthread_mutex_t orders_mutex;
pthread_cond_t orders_c;

pthread_mutex_t ordersStarted_mutex;
pthread_cond_t ordersStarted_c;

llNode *orders;
int ordersStarted;

int cookrecipe[3];
int cookstep [3];

void *cook(void *ptr);
void *getOrders(void *ptr);
struct timeval timeDiff(struct timeval start, struct timeval end);

int main(int argc, char *argv[]) {
	pthread_t c1, c2, c3, waiter;
	ordersStarted = 0;
	orders = NULL;
	prep_occupied = 0;
	oven_occupied = 0;
	stove_occupied = 0;
	sink_occupied = 0;
	
	cookrecipe[0] = 0;
	cookrecipe[1] = 0;
	cookrecipe[2] = 0;
	cookstep[0] = 0;
	cookstep[1] = 0;
	cookstep[3] = 0;
	
	//Allocate chef numbers
	int *chefNum1 = malloc(sizeof(*chefNum1));
	if(chefNum1 == NULL) {
		printf("Couldn't allocate memory to chefNum1.\n");
		return 1;
	}
	*chefNum1 = 1;
	
	int *chefNum2 = malloc(sizeof(*chefNum2));
	if(chefNum2 == NULL) {
		printf("Couldn't allocate memory to chefNum2.\n");
		return 1;
	}
	*chefNum2 = 2;
	
	int *chefNum3 = malloc(sizeof(*chefNum3));
	if(chefNum3 == NULL) {
		printf("Couldn't allocate memory to chefNum3.\n");
		return 1;
	}
	*chefNum3 = 3;
	
	//Seed random generator
	srand(0);
	
	//Create a queue for waiting orders
	
	//Initialize mutexes
	if(pthread_mutex_init(&prep_mutex, NULL) != 0) {
		printf("Prep mutex init failed\n");
		return 1;
	}
	if(pthread_mutex_init(&oven_mutex, NULL) != 0) {
		printf("Oven mutex init failed\n");
		return 1;
	}
	if(pthread_mutex_init(&stove_mutex, NULL) != 0) {
		printf("Stove mutex init failed\n");
		return 1;
	}
	if(pthread_mutex_init(&sink_mutex, NULL) != 0) {
		printf("Sink mutex init failed\n");
		return 1;
	}
	if(pthread_mutex_init(&orders_mutex, NULL) != 0) {
		printf("Orders mutex init failed\n");
		return 1;
	}
	if(pthread_mutex_init(&ordersStarted_mutex, NULL) != 0) {
		printf("ordersStarted mutex init failed\n");
		return 1;
	}
	
	//Initialize conditional variables
	pthread_cond_init(&prep_c, NULL);
	pthread_cond_init(&oven_c, NULL);
	pthread_cond_init(&stove_c, NULL);
	pthread_cond_init(&sink_c, NULL);
	pthread_cond_init(&orders_c, NULL);
	pthread_cond_init(&ordersStarted_c, NULL);
	
	//Create cook threads
	pthread_create(&waiter, NULL, getOrders, NULL);
	pthread_create(&c1, NULL, cook, chefNum1);
	pthread_create(&c2, NULL, cook, chefNum2);
	pthread_create(&c3, NULL, cook, chefNum3);
	
	//Join threads
	pthread_join(c3, NULL);
	pthread_join(c2, NULL);
	pthread_join(c1, NULL);
	pthread_join(waiter, NULL);
	
	//Destroy Conditional variables
	pthread_cond_destroy(&prep_c);
	pthread_cond_destroy(&oven_c);
	pthread_cond_destroy(&stove_c);
	pthread_cond_destroy(&sink_c);
	pthread_cond_destroy(&orders_c);
	pthread_cond_destroy(&ordersStarted_c);
	
	//Destroy mutexes
	pthread_mutex_destroy(&prep_mutex);
	pthread_mutex_destroy(&oven_mutex);
	pthread_mutex_destroy(&stove_mutex);
	pthread_mutex_destroy(&sink_mutex);
	pthread_mutex_destroy(&orders_mutex);
	pthread_mutex_destroy(&ordersStarted_mutex);
	
	return 0;
}

/*
 *cook1
 *
 *Thread function for cook 1
 */
void *cook(void *ptr) {
	int curRecipe = 0;
	int curOrderNum = 0;
	int keepServing = 1;
	int expectedTime = 0;
	struct timeval startTime;
	struct timeval endTime;
	
	int chefNum = *((int *) ptr);
	free(ptr);
	
	printf("Cook %d ready to work.\n", chefNum);
	
	while(keepServing) {
		if(curRecipe == 0) {
			//check if more orders need to be made
			pthread_mutex_lock(&ordersStarted_mutex);	//lock ordersStarted mutex
			
			if(ordersStarted >= 30) {
				pthread_mutex_unlock(&ordersStarted_mutex);	//unlock ordersStarted mutex
				break;
			}
		
			pthread_mutex_unlock(&ordersStarted_mutex);	//unlock ordersStarted mutex
			
			//grab a recipe
			pthread_mutex_lock(&orders_mutex);	//lock orders mutex
			
			//check if more orders need to be made
			pthread_mutex_lock(&ordersStarted_mutex);	//lock ordersStarted mutex
			
			if(ordersStarted >= 30) {
				pthread_mutex_unlock(&ordersStarted_mutex);	//unlock ordersStarted mutex
				break;
			}
		
			pthread_mutex_unlock(&ordersStarted_mutex);	//unlock ordersStarted mutex
			
			while(orders == NULL)
				pthread_cond_wait(&orders_c, &orders_mutex);
		
			
			//get the best order or go back to waiting
			llNode *currentOrder = orders;
			int haveOrder = 0;
			
			struct timeval curTime;
			gettimeofday(&curTime, NULL);
			
			//Check if the first order has been waiting too long
			if(currentOrder != NULL) {
				if(timeDiff(orders->startTime, curTime).tv_sec > (int)(orders->expectedTime * 1.5)) {
					if(chefNum == 1) {
						if(!(((cookrecipe[1] == 4 || cookrecipe[2] == 4) && (currentOrder->recipe == 1 || currentOrder->recipe == 3 || currentOrder->recipe == 5)) || 
							  (currentOrder->recipe == 4 && (((cookrecipe[1] == 1 && cookstep[1] < 5) || (cookrecipe[2] == 1 && cookstep[2] < 5)) || 
							  											((cookrecipe[1] == 3 && cookstep[1] < 2) || (cookrecipe[2] == 3 && cookstep[2] < 2)) || 
							  											((cookrecipe[1] == 5 && cookstep[1] < 5) || (cookrecipe[2] == 5 && cookstep[2] < 5)))) ||
							  ((currentOrder->recipe == 1 || currentOrder->recipe == 2 || currentOrder->recipe == 3 || currentOrder->recipe == 5) && 
							   ((cookrecipe[1] == 1 && cookrecipe[2] == 1 && (cookstep[1] < 4 || cookstep[2] < 4)) ||
							    (cookrecipe[1] == 1 && cookrecipe[2] == 5 && (cookstep[1] < 4 || cookstep[2] < 4)) ||
							    (cookrecipe[1] == 5 && cookrecipe[2] == 1 && (cookstep[1] < 4 || cookstep[2] < 4)) ||
							    (cookrecipe[1] == 5 && cookrecipe[2] == 5 && (cookstep[1] < 4 || cookstep[2] < 4)) ||
							    (cookrecipe[1] == 1 && cookrecipe[2] == 3 && (cookstep[1] < 4)) ||
							    (cookrecipe[1] == 3 && cookrecipe[2] == 1 && (cookstep[2] < 4)) ||
							    (cookrecipe[1] == 5 && cookrecipe[2] == 3 && (cookstep[1] < 4)) ||
							    (cookrecipe[1] == 3 && cookrecipe[2] == 5 && (cookstep[2] < 4)))))) {
							haveOrder = 1;
						}
					}
					else if(chefNum == 2) {
						if(!(((cookrecipe[0] == 4 || cookrecipe[2] == 4) && (currentOrder->recipe == 1 || currentOrder->recipe == 3 || currentOrder->recipe == 5)) || 
							  (currentOrder->recipe == 4 && (((cookrecipe[0] == 1 && cookstep[0] < 5) || (cookrecipe[2] == 1 && cookstep[2] < 5)) || 
							  											((cookrecipe[0] == 3 && cookstep[0] < 2) || (cookrecipe[2] == 3 && cookstep[2] < 2)) || 
							  											((cookrecipe[0] == 5 && cookstep[0] < 5) || (cookrecipe[2] == 5 && cookstep[2] < 5)))) ||
							  ((currentOrder->recipe == 1 || currentOrder->recipe == 2 || currentOrder->recipe == 3 || currentOrder->recipe == 5) && 
							   ((cookrecipe[0] == 1 && cookrecipe[2] == 1 && (cookstep[0] < 4 || cookstep[2] < 4)) ||
							    (cookrecipe[0] == 1 && cookrecipe[2] == 5 && (cookstep[0] < 4 || cookstep[2] < 4)) ||
							    (cookrecipe[0] == 5 && cookrecipe[2] == 1 && (cookstep[0] < 4 || cookstep[2] < 4)) ||
							    (cookrecipe[0] == 5 && cookrecipe[2] == 5 && (cookstep[0] < 4 || cookstep[2] < 4)) ||
							    (cookrecipe[0] == 1 && cookrecipe[2] == 3 && (cookstep[0] < 4)) ||
							    (cookrecipe[0] == 3 && cookrecipe[2] == 1 && (cookstep[2] < 4)) ||
							    (cookrecipe[0] == 5 && cookrecipe[2] == 3 && (cookstep[0] < 4)) ||
							    (cookrecipe[0] == 3 && cookrecipe[2] == 5 && (cookstep[2] < 4)))))) {
							haveOrder = 1;
						}
					}
					else if(chefNum == 3) {
						if(!(((cookrecipe[1] == 4 || cookrecipe[0] == 4) && (currentOrder->recipe == 1 || currentOrder->recipe == 3 || currentOrder->recipe == 5)) || 
							  (currentOrder->recipe == 4 && (((cookrecipe[1] == 1 && cookstep[1] < 5) || (cookrecipe[0] == 1 && cookstep[0] < 5)) || 
							  											((cookrecipe[1] == 3 && cookstep[1] < 2) || (cookrecipe[0] == 3 && cookstep[0] < 2)) || 
							  											((cookrecipe[1] == 5 && cookstep[1] < 5) || (cookrecipe[0] == 5 && cookstep[0] < 5)))) ||
							  ((currentOrder->recipe == 1 || currentOrder->recipe == 2 || currentOrder->recipe == 3 || currentOrder->recipe == 5) && 
							   ((cookrecipe[1] == 1 && cookrecipe[0] == 1 && (cookstep[1] < 4 || cookstep[0] < 4)) ||
							    (cookrecipe[1] == 1 && cookrecipe[0] == 5 && (cookstep[1] < 4 || cookstep[0] < 4)) ||
							    (cookrecipe[1] == 5 && cookrecipe[0] == 1 && (cookstep[1] < 4 || cookstep[0] < 4)) ||
							    (cookrecipe[1] == 5 && cookrecipe[0] == 5 && (cookstep[1] < 4 || cookstep[0] < 4)) ||
							    (cookrecipe[1] == 1 && cookrecipe[0] == 3 && (cookstep[1] < 4)) ||
							    (cookrecipe[1] == 3 && cookrecipe[0] == 1 && (cookstep[0] < 4)) ||
							    (cookrecipe[1] == 5 && cookrecipe[0] == 3 && (cookstep[1] < 4)) ||
							    (cookrecipe[1] == 3 && cookrecipe[0] == 5 && (cookstep[0] < 4)))))) {
							haveOrder = 1;
						}
					}
				}
				else {	//otherwise check naturally
					while(currentOrder != NULL) {
						if(chefNum == 1) {
							if(!(((cookrecipe[1] == 4 || cookrecipe[2] == 4) && (currentOrder->recipe == 1 || currentOrder->recipe == 3 || currentOrder->recipe == 5)) || 
								  (currentOrder->recipe == 4 && (((cookrecipe[1] == 1 && cookstep[1] < 5) || (cookrecipe[2] == 1 && cookstep[2] < 5)) || 
								  											((cookrecipe[1] == 3 && cookstep[1] < 2) || (cookrecipe[2] == 3 && cookstep[2] < 2)) || 
								  											((cookrecipe[1] == 5 && cookstep[1] < 5) || (cookrecipe[2] == 5 && cookstep[2] < 5)))) ||
							  ((currentOrder->recipe == 1 || currentOrder->recipe == 2 || currentOrder->recipe == 3 || currentOrder->recipe == 5) && 
							   ((cookrecipe[1] == 1 && cookrecipe[2] == 1 && (cookstep[1] < 4 || cookstep[2] < 4)) ||
							    (cookrecipe[1] == 1 && cookrecipe[2] == 5 && (cookstep[1] < 4 || cookstep[2] < 4)) ||
							    (cookrecipe[1] == 5 && cookrecipe[2] == 1 && (cookstep[1] < 4 || cookstep[2] < 4)) ||
							    (cookrecipe[1] == 5 && cookrecipe[2] == 5 && (cookstep[1] < 4 || cookstep[2] < 4)) ||
							    (cookrecipe[1] == 1 && cookrecipe[2] == 3 && (cookstep[1] < 4)) ||
							    (cookrecipe[1] == 3 && cookrecipe[2] == 1 && (cookstep[2] < 4)) ||
							    (cookrecipe[1] == 5 && cookrecipe[2] == 3 && (cookstep[1] < 4)) ||
							    (cookrecipe[1] == 3 && cookrecipe[2] == 5 && (cookstep[2] < 4)))))) {
								haveOrder = 1;
								break;
							}
						}
						else if(chefNum == 2) {
							if(!(((cookrecipe[0] == 4 || cookrecipe[2] == 4) && (currentOrder->recipe == 1 || currentOrder->recipe == 3 || currentOrder->recipe == 5)) || 
								  (currentOrder->recipe == 4 && (((cookrecipe[0] == 1 && cookstep[0] < 5) || (cookrecipe[2] == 1 && cookstep[2] < 5)) || 
								  											((cookrecipe[0] == 3 && cookstep[0] < 2) || (cookrecipe[2] == 3 && cookstep[2] < 2)) || 
								  											((cookrecipe[0] == 5 && cookstep[0] < 5) || (cookrecipe[2] == 5 && cookstep[2] < 5)))) ||
							  ((currentOrder->recipe == 1 || currentOrder->recipe == 2 || currentOrder->recipe == 3 || currentOrder->recipe == 5) && 
							   ((cookrecipe[0] == 1 && cookrecipe[2] == 1 && (cookstep[0] < 4 || cookstep[2] < 4)) ||
							    (cookrecipe[0] == 1 && cookrecipe[2] == 5 && (cookstep[0] < 4 || cookstep[2] < 4)) ||
							    (cookrecipe[0] == 5 && cookrecipe[2] == 1 && (cookstep[0] < 4 || cookstep[2] < 4)) ||
							    (cookrecipe[0] == 5 && cookrecipe[2] == 5 && (cookstep[0] < 4 || cookstep[2] < 4)) ||
							    (cookrecipe[0] == 1 && cookrecipe[2] == 3 && (cookstep[0] < 4)) ||
							    (cookrecipe[0] == 3 && cookrecipe[2] == 1 && (cookstep[2] < 4)) ||
							    (cookrecipe[0] == 5 && cookrecipe[2] == 3 && (cookstep[0] < 4)) ||
							    (cookrecipe[0] == 3 && cookrecipe[2] == 5 && (cookstep[2] < 4)))))) {
								haveOrder = 1;
								break;
							}
						}
						else if(chefNum == 3) {
							if(!(((cookrecipe[1] == 4 || cookrecipe[0] == 4) && (currentOrder->recipe == 1 || currentOrder->recipe == 3 || currentOrder->recipe == 5)) || 
								  (currentOrder->recipe == 4 && (((cookrecipe[1] == 1 && cookstep[1] < 5) || (cookrecipe[0] == 1 && cookstep[0] < 5)) || 
								  											((cookrecipe[1] == 3 && cookstep[1] < 2) || (cookrecipe[0] == 3 && cookstep[0] < 2)) || 
								  											((cookrecipe[1] == 5 && cookstep[1] < 5) || (cookrecipe[0] == 5 && cookstep[0] < 5)))) ||
							  ((currentOrder->recipe == 1 || currentOrder->recipe == 2 || currentOrder->recipe == 3 || currentOrder->recipe == 5) && 
							   ((cookrecipe[0] == 1 && cookrecipe[1] == 1 && (cookstep[0] < 4 || cookstep[1] < 4)) ||
							    (cookrecipe[0] == 1 && cookrecipe[1] == 5 && (cookstep[0] < 4 || cookstep[1] < 4)) ||
							    (cookrecipe[0] == 5 && cookrecipe[1] == 1 && (cookstep[0] < 4 || cookstep[1] < 4)) ||
							    (cookrecipe[0] == 5 && cookrecipe[1] == 5 && (cookstep[0] < 4 || cookstep[1] < 4)) ||
							    (cookrecipe[0] == 1 && cookrecipe[1] == 3 && (cookstep[0] < 4)) ||
							    (cookrecipe[0] == 3 && cookrecipe[1] == 1 && (cookstep[1] < 4)) ||
							    (cookrecipe[0] == 5 && cookrecipe[1] == 3 && (cookstep[0] < 4)) ||
							    (cookrecipe[0] == 3 && cookrecipe[1] == 5 && (cookstep[1] < 4)))))) {
								haveOrder = 1;
								break;
							}
						}
						currentOrder = currentOrder->next;	//increment order pointer
					}
				}
			}
			
			//If an order is appropriate, get it
			if(haveOrder) {
				curRecipe = currentOrder->recipe;			//get the node's recipe
				curOrderNum = currentOrder->ticket;			//get the node's ticket
				expectedTime = currentOrder->expectedTime;//get the expected time of the node's recipe
			
				cookrecipe[chefNum - 1] = curRecipe;
			
				printf("Chef %d has begun to prepare Order %d (Recipe %d).\n", chefNum, curOrderNum, curRecipe);
				
				orders = removeNode(orders, curOrderNum);	//remove the order from the linkedList
				
				//Increment ordersStarted
				pthread_mutex_lock(&ordersStarted_mutex);	//lock ordersStarted mutex
			
				ordersStarted++;
			
				pthread_mutex_unlock(&ordersStarted_mutex);	//unlock ordersStarted mutex
				
				pthread_mutex_unlock(&orders_mutex);	//unlock orders mutex
			}
			else {
				pthread_mutex_unlock(&orders_mutex);	//unlock orders mutex
			}
		}
		else {	//perform recipe
			
			gettimeofday(&startTime, NULL);
		
			if(curRecipe == 1) {
				//Recipe 1: prep (3), stove (4), sink (2), prep (2), oven (5), sink (10)
				//Prep
				cookstep[chefNum - 1] = 1;
				
				pthread_mutex_lock(&prep_mutex);
	
				while(prep_occupied != 0)
					pthread_cond_wait(&prep_c, &prep_mutex);
				
				prep_occupied = chefNum;
	
					printf("Chef %d is in the prep area.\n", chefNum);
				
				sleep(R1a);	//Use prep for 3 seconds
				
				pthread_mutex_lock(&stove_mutex);	//attempt to enter stove area
				
				while(stove_occupied != 0)
					pthread_cond_wait(&stove_c, &stove_mutex);
				
				//Exit prep
				prep_occupied = 0;
				pthread_cond_signal(&prep_c);
				pthread_mutex_unlock(&prep_mutex);	//unlock prep after entering stove area
	
				printf("Chef %d has left the prep area for the stove.\n", chefNum);
				
				//Stove
				cookstep[chefNum - 1] = 2;
				
				stove_occupied = chefNum;
	
				sleep(R1b);	//Use stove for 4 seconds
				
				pthread_mutex_lock(&sink_mutex);	//attempt to enter sink area
	
				while(sink_occupied != 0)
					pthread_cond_wait(&sink_c, &sink_mutex);
				
				//Exit stove
				stove_occupied = 0;
				pthread_cond_signal(&stove_c);
				pthread_mutex_unlock(&stove_mutex);	//unlock stove after entering sink area
	
				printf("Chef %d has left the stove area for the sink.\n", chefNum);
				
				//Sink
				cookstep[chefNum - 1] = 3;
				
				sink_occupied = chefNum;
	
				sleep(R1c);	//Use sink for 2 seconds
				
				pthread_mutex_lock(&prep_mutex);	//attempt to enter the prep area
	
				while(prep_occupied != 0)
					pthread_cond_wait(&prep_c, &prep_mutex);
				
				//Exit sink
				sink_occupied = 0;
				pthread_cond_signal(&sink_c);
				pthread_mutex_unlock(&sink_mutex);	//unlock sink after entering prep area
	
				printf("Chef %d has left the sink area for the prep.\n", chefNum);
				
				//Prep
				cookstep[chefNum - 1] = 4;
				
				prep_occupied = chefNum;
				
				sleep(R1d);	//Use prep for 2 seconds
				
				pthread_mutex_lock(&oven_mutex);	//attempt to enter the oven area
	
				while(oven_occupied != 0)
					pthread_cond_wait(&oven_c, &oven_mutex);
				
				//Exit prep
				prep_occupied = 0;
				pthread_cond_signal(&prep_c);
				pthread_mutex_unlock(&prep_mutex);	//unlock prep after entering oven area
	
				printf("Chef %d has left the prep area for the oven.\n", chefNum);
				
				//Oven
				cookstep[chefNum - 1] = 5;
				
				oven_occupied = chefNum;
	
				sleep(R1e);	//Use oven for 5 seconds
				
				pthread_mutex_lock(&sink_mutex);	//attempt to enter sink area
	
				while(sink_occupied != 0)
					pthread_cond_wait(&sink_c, &sink_mutex);
				
				//Exit oven
				oven_occupied = 0;
				pthread_cond_signal(&oven_c);
				pthread_mutex_unlock(&oven_mutex);	//unlock oven after entering sink area
	
				printf("Chef %d has left the oven area for the sink.\n", chefNum);
				
				//Sink
				cookstep[chefNum - 1] = 6;
				
				sink_occupied = chefNum;
	
				sleep(R1f);	//Use sink for 10 seconds
				
				//Exit sink
				sink_occupied = 0;
				pthread_cond_signal(&sink_c);
				pthread_mutex_unlock(&sink_mutex);	//unlock sink area
	
				printf("Chef %d has left the sink area.\n", chefNum);
			}
			else if(curRecipe == 2) {
				//Recipe 2: prep (5), stove (3), sink (15)
				//Prep
				cookstep[chefNum - 1] = 1;
				
				pthread_mutex_lock(&prep_mutex);	//lock prep mutex
	
				while(prep_occupied != 0)
					pthread_cond_wait(&prep_c, &prep_mutex);
				
				prep_occupied = chefNum;
	
				printf("Chef %d is in the prep area.\n", chefNum);
				
				sleep(R2a);	//Use prep for 5 seconds
				
				pthread_mutex_lock(&stove_mutex);	//attempt to enter stove area
	
				while(stove_occupied != 0)
					pthread_cond_wait(&stove_c, &stove_mutex);
				
				//Exit prep
				prep_occupied = 0;
				pthread_cond_signal(&prep_c);
				pthread_mutex_unlock(&prep_mutex);	//unlock prep after entering stove area
	
				printf("Chef %d has left the prep area for the stove.\n", chefNum);
				
				//Stove
				cookstep[chefNum - 1] = 2;
				
				stove_occupied = chefNum;
	
				sleep(R2b);	//Use stove for 3 seconds
				
				pthread_mutex_lock(&sink_mutex);	//attempt to enter sink area
	
				while(sink_occupied != 0)
					pthread_cond_wait(&sink_c, &sink_mutex);
				
				//Exit stove
				stove_occupied = 0;
				pthread_cond_signal(&stove_c);
				pthread_mutex_unlock(&stove_mutex);	//unlock stove after entering sink area
	
				printf("Chef %d has left the stove area for the sink.\n", chefNum);
				
				//Sink
				cookstep[chefNum - 1] = 3;
				
				sink_occupied = chefNum;
	
				sleep(R2c);	//Use sink for 15 seconds
				
				//Exit sink
				sink_occupied = 0;
				pthread_cond_signal(&sink_c);
				pthread_mutex_unlock(&sink_mutex);	//unlock sink
	
				printf("Chef %d has left the sink area.\n", chefNum);
			}
			else if(curRecipe == 3) {
				//Recipe 3: prep (10), oven (5), sink (5)
				//Prep
				cookstep[chefNum - 1] = 1;
				
				pthread_mutex_lock(&prep_mutex);	//lock prep mutex
	
				while(prep_occupied != 0)
					pthread_cond_wait(&prep_c, &prep_mutex);
				
				prep_occupied = chefNum;
	
				printf("Chef %d is in the prep area.\n", chefNum);
				
				sleep(R3a);	//Use prep for 10 seconds
				
				pthread_mutex_lock(&oven_mutex);	//attempt to enter oven area
	
				while(oven_occupied != 0)
					pthread_cond_wait(&oven_c, &oven_mutex);
					
				//Exit prep
				prep_occupied = 0;
				pthread_cond_signal(&prep_c);
				pthread_mutex_unlock(&prep_mutex);	//unlock prep after entering oven area
	
				printf("Chef %d has left the prep area for the oven.\n", chefNum);
				
				//Oven
				cookstep[chefNum - 1] = 2;
				
				oven_occupied = chefNum;
				
				printf("Chef %d is in the oven area.\n", chefNum);
	
				sleep(R3b);	//Use oven for 5 seconds
				
				pthread_mutex_lock(&sink_mutex);	//attempt to enter sink
	
				while(sink_occupied != 0)
					pthread_cond_wait(&sink_c, &sink_mutex);
					
				//Exit oven
				oven_occupied = 0;
				pthread_cond_signal(&oven_c);
				pthread_mutex_unlock(&oven_mutex);	//unlock oven after entering sink area
	
				printf("Chef %d has left the oven area for the sink.\n", chefNum);
				
				//Sink
				cookstep[chefNum - 1] = 3;
				
				sink_occupied = chefNum;
				
				printf("Chef %d is in the sink area.\n", chefNum);
	
				sleep(R3c);	//Use sink for 5 seconds
				
				//Exit sink
				sink_occupied = 0;
				pthread_cond_signal(&sink_c);
				pthread_mutex_unlock(&sink_mutex);	//unlock sink
	
				printf("Chef %d has left the sink area.\n", chefNum);
			}
			else if(curRecipe == 4) {
				//Recipe 4: oven (15), prep (5), sink (4)
				//Oven
				cookstep[chefNum - 1] = 1;
				
				pthread_mutex_lock(&oven_mutex);	//lock oven mutex
	
				while(oven_occupied != 0)
					pthread_cond_wait(&oven_c, &oven_mutex);
				
				oven_occupied = chefNum;
				
				printf("Chef %d is in the oven area.\n", chefNum);
	
				sleep(R4a);	//Use oven for 15 seconds
				
				pthread_mutex_lock(&prep_mutex);	//attempt to enter prep
	
				while(prep_occupied != 0)
					pthread_cond_wait(&prep_c, &prep_mutex);
				
				//Exit oven
				oven_occupied = 0;
				pthread_cond_signal(&oven_c);
				pthread_mutex_unlock(&oven_mutex);	//unlock oven after entering prep area
	
				printf("Chef %d has left the oven area for the prep.\n", chefNum);
				
				//Prep
				cookstep[chefNum - 1] = 2;
				
				prep_occupied = chefNum;
	
				printf("Chef %d is in the prep area.\n", chefNum);
				
				sleep(R4b);	//Use prep for 5 seconds
				
				pthread_mutex_lock(&sink_mutex);	//attempt to enter sink area
	
				while(sink_occupied != 0)
					pthread_cond_wait(&sink_c, &sink_mutex);
				
				//Exit prep
				prep_occupied = 0;
				pthread_cond_signal(&prep_c);
				pthread_mutex_unlock(&prep_mutex);	//unlock prep after entering sink area
	
				printf("Chef %d has left the prep area for the sink.\n", chefNum);
				
				//Sink
				cookstep[chefNum - 1] = 3;
				
				sink_occupied = chefNum;
				
				printf("Chef %d is in the sink area.\n", chefNum);
	
				sleep(R4c);	//Use sink for 4 seconds
				
				//Exit sink
				sink_occupied = 0;
				pthread_cond_signal(&sink_c);
				pthread_mutex_unlock(&sink_mutex);
	
				printf("Chef %d has left the sink area.\n", chefNum);
			}
			else if(curRecipe == 5) {
				//Recipe 5: prep (2), oven (3), sink (2), prep (2), oven (3), sink (4)
				//Prep
				cookstep[chefNum - 1] = 1;
				
				pthread_mutex_lock(&prep_mutex);	//lock prep mutex
	
				while(prep_occupied != 0)
					pthread_cond_wait(&prep_c, &prep_mutex);
				
				prep_occupied = chefNum;
	
				printf("Chef %d is in the prep area.\n", chefNum);
				
				sleep(R5a);	//Use prep for 2 seconds
				
				pthread_mutex_lock(&oven_mutex);	//attempt to enter oven area
	
				while(oven_occupied != 0)
					pthread_cond_wait(&oven_c, &oven_mutex);
					
				//Exit prep
				prep_occupied = 0;
				pthread_cond_signal(&prep_c);
				pthread_mutex_unlock(&prep_mutex);	//unlock prep after entering oven area
	
				printf("Chef %d has left the prep area for the oven.\n", chefNum);
				
				//Oven
				cookstep[chefNum - 1] = 2;
				
				oven_occupied = chefNum;
				
				printf("Chef %d is in the oven area.\n", chefNum);
	
				sleep(R5b);	//Use oven for 3 seconds
				
				pthread_mutex_lock(&sink_mutex);	//attempt to enter sink
	
				while(sink_occupied != 0)
					pthread_cond_wait(&sink_c, &sink_mutex);
					
				//Exit oven
				oven_occupied = 0;
				pthread_cond_signal(&oven_c);
				pthread_mutex_unlock(&oven_mutex);	//unlock oven after entering sink area
	
				printf("Chef %d has left the oven area for the sink.\n", chefNum);
				
				//Sink
				cookstep[chefNum - 1] = 3;
				
				sink_occupied = chefNum;
				
				printf("Chef %d is in the sink area.\n", chefNum);
	
				sleep(R5c);	//Use sink for 2 seconds
				
				pthread_mutex_lock(&prep_mutex);	//attempt to enter prep area
	
				while(prep_occupied != 0)
					pthread_cond_wait(&prep_c, &prep_mutex);
					
				//Exit sink
				sink_occupied = 0;
				pthread_cond_signal(&sink_c);
				pthread_mutex_unlock(&sink_mutex);	//unlock sink
	
				printf("Chef %d has left the sink area for the prep.\n", chefNum);
				
				//Prep
				cookstep[chefNum - 1] = 4;
				
				prep_occupied = chefNum;
	
				printf("Chef %d is in the prep area.\n", chefNum);
				
				sleep(R5d);	//Use prep for 2 seconds
				
				pthread_mutex_lock(&oven_mutex);	//attempt to enter oven area
	
				while(oven_occupied != 0)
					pthread_cond_wait(&oven_c, &oven_mutex);
					
				//Exit prep
				prep_occupied = 0;
				pthread_cond_signal(&prep_c);
				pthread_mutex_unlock(&prep_mutex);	//unlock prep after entering oven area
	
				printf("Chef %d has left the prep area for the oven.\n", chefNum);
				
				//Oven
				cookstep[chefNum - 1] = 5;
				
				oven_occupied = chefNum;
				
				printf("Chef %d is in the oven area.\n", chefNum);
	
				sleep(R5e);	//Use oven for 3 seconds
				
				pthread_mutex_lock(&sink_mutex);	//attempt to enter sink
	
				while(sink_occupied != 0)
					pthread_cond_wait(&sink_c, &sink_mutex);
					
				//Exit oven
				oven_occupied = 0;
				pthread_cond_signal(&oven_c);
				pthread_mutex_unlock(&oven_mutex);	//unlock oven after entering sink area
	
				printf("Chef %d has left the oven area for the sink.\n", chefNum);
				
				//Sink
				cookstep[chefNum - 1] = 6;
				
				sink_occupied = chefNum;
				
				printf("Chef %d is in the sink area.\n", chefNum);
	
				sleep(R5f);	//Use sink for 4 seconds
				
				//Exit sink
				sink_occupied = 0;
				pthread_cond_signal(&sink_c);
				pthread_mutex_unlock(&sink_mutex);	//unlock sink
	
				printf("Chef %d has left the sink area.\n", chefNum);
			}
			
			gettimeofday(&endTime, NULL);
			
			struct timeval cookTime = timeDiff(startTime, endTime);
			
			//Return to "waiting area"
			printf("Chef %d has finished Order %d (Recipe %d).\n", chefNum, curOrderNum, curRecipe);
			
			printf("Chef %d, Order %d: expected time %d; actual time %ld.\n", chefNum, curOrderNum, expectedTime, cookTime.tv_sec);
			curOrderNum = 0;
			curRecipe = 0;
			expectedTime = 0;
			cookrecipe[chefNum - 1] = 0;
			cookstep[chefNum - 1] = 0;
		}
	}
	
	printf("No remaining orders. Chef %d is leaving the kitchen.\n", chefNum);
	
	pthread_exit(0);
}

/*
 *getOrders
 *
 *Thread command for the waiter
 *Creates orders and places them in the queue
 */
void *getOrders(void *ptr) {
	int i, r;
	for(i = 0; i < TOTAL_ORDERS; i++) {
		//Gain access to orders
		pthread_mutex_lock(&orders_mutex);
		
		//Create Order
		r = (int)(rand() % 5 + 1);
		
		orders = addNode(orders, r, i);
		
		printf("Order %d (Recipe %d) has arrived.\n", i, r);
		
		//Signal cooks that an order has arrived
		pthread_cond_signal(&orders_c);
		
		//release access to orders
		pthread_mutex_unlock(&orders_mutex);
		
		//Wait for a set time
		sleep(rand() % 10 + 1);
	}
	
	printf("All orders have been taken.\n");
	
	pthread_exit(0);
}

struct timeval timeDiff(struct timeval start, struct timeval end) {
	struct timeval diff; //the difference of the two times

	if (end.tv_usec < start.tv_usec) {
		end.tv_sec -= 1;
		end.tv_usec += 1000000;
		diff.tv_sec = (end.tv_sec - start.tv_sec);
		diff.tv_usec = (end.tv_usec - start.tv_usec);
	}
	else{
		diff.tv_sec = (end.tv_sec - start.tv_sec);
		diff.tv_usec = (end.tv_usec - start.tv_usec);
	}
	return diff;
}





