#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "rand.h"

#define NUM_CHAIRS 10
#define NUM_CUSTOMERS 15
#define moveup(y) printf("\033[%dA", (y))

struct shop {
	sem_t chairs;
	sem_t customers;
	sem_t barber_chair[NUM_CHAIRS];
	sem_t describe_cut;
	sem_t print;
	sem_t index_lock;
	int index;
	int free_index;
	int cut_time;
	int served;
	int sent_away;
};

// Error wrapper for sem_wait
int _sem_wait(sem_t* s) {
	if(sem_wait(s)) {
		fprintf(stderr, "Error: sem_wait failed\n");
		return -1;
	}
	return 0;
}

// Error wrapper for sem_post
int _sem_post(sem_t* s) {
	if(sem_post(s)) {
		fprintf(stderr, "Error: sem_post failed\n");
		return -1;
	}
	return 0;
}


// Error wrapper for sem_init
int _sem_init(sem_t* s, int shared, int count) {
	if(sem_init(s, shared, count)) {
		fprintf(stderr, "Error: sem_init failed\n");
		return -1;
	}
	return 0;
}

// Wrapper around sem_getvalue
int _sem_getvalue(sem_t* s) {
	int value;
	sem_getvalue(s, &value);
	return value;
}

void get_hair_cut(int wait) {
	sleep(wait);
}

void cut_hair(int wait) {
	sleep(wait);
}

void* barber(void *arg) {
	struct shop* shop = (struct shop*) arg;	
	
	while(1) {
		// Wait for a customer
		_sem_wait(&shop->customers);
		
		// Open the barber chair
		_sem_post(&shop->barber_chair[shop->index]);
		
		// Wait for customer to describe the hair cut
		_sem_wait(&shop->describe_cut);
		
		// Post to print
		_sem_post(&shop->print);

		// Cut hair
		cut_hair(shop->cut_time);

		// Post to print
		_sem_post(&shop->print);

		shop->index = (shop->index + 1) % NUM_CHAIRS;
	}	

	return NULL;
}

void* customer(void *arg) {
	struct shop* shop = (struct shop*) arg;	
	int index;

	sleep(randomRange(0,5));

	while(1) {
		
		if(_sem_getvalue(&shop->chairs) > 0) {	
			// Sit in chair
			_sem_wait(&shop->chairs);
			_sem_post(&shop->customers);

			_sem_post(&shop->print);

			_sem_wait(&shop->index_lock);
			index = shop->free_index;
			shop->free_index = (shop->free_index + 1) % NUM_CHAIRS;
			_sem_post(&shop->index_lock);

			// Wait for barber chair to open
			_sem_wait(&shop->barber_chair[index]);
		
			shop->cut_time = randomRange(1,5);

			// Describe the cut time to the barber
			_sem_post(&shop->describe_cut);

			// Get hair cut
			get_hair_cut(shop->cut_time);
	
			shop->served++;

			// Release chair
			_sem_post(&shop->chairs);		

		} else {
			shop->sent_away++;
		}
		// leave
		
		// Wait until needs another haircut
		sleep(randomRange(30,60));
		
	}
	
	return NULL;
}

/* Main function */
int main(int argc, char* argv[]) {

	int i;
	struct shop* shop;
	pthread_t barber_thread;
	pthread_t customers[NUM_CUSTOMERS];
	
	// initialize shop
	shop = (struct shop*) malloc(sizeof(struct shop));
	_sem_init(&shop->chairs, 0, NUM_CHAIRS);
	_sem_init(&shop->customers, 0, 0);
	for (i = 0; i < NUM_CHAIRS; i++) {
		_sem_init(&shop->barber_chair[i], 0, 0);
	}
	_sem_init(&shop->describe_cut, 0, 0);
	_sem_init(&shop->print, 0, 1);
	_sem_init(&shop->index_lock, 0, 1);
	shop->index = 0;
	shop->free_index = 0;
	shop->served = 0;
	shop->sent_away = 0;

	// initialize threads
	if(pthread_create(&barber_thread, NULL, barber, shop)) {
		fprintf(stderr, "Error: failed to create barber pthread.\n");
		return 1;
	}
	for (i = 0; i < NUM_CUSTOMERS; i++) {
		if(pthread_create(&customers[i], NULL, customer, shop)) {
			fprintf(stderr, "Error: failed to create pthread.\n");
			return 2;
		}
	}

	int up = 0;
	char tmp[5];
	while(1) {
		_sem_wait(&shop->print);
		
		if (up) {
			moveup(3);
			printf("\r");
			fflush(stdout);
		} else {
			up = 1;
		}

		sprintf(tmp, "%d", shop->cut_time);

		printf(" Barber   | Wait | Served | Sent Away | Chairs\n"
		       "----------------------------------------");
		for (i = 0; i < NUM_CHAIRS * 4 - 1; i++) {
			printf("-");
		}
		printf("\n");
		printf(" %-8s | %-4s | %-6d | %-9d | ",
               (_sem_getvalue(&shop->chairs) < NUM_CHAIRS ? "Cutting" : "Sleeping"),
		       (_sem_getvalue(&shop->chairs) < NUM_CHAIRS ? tmp : "----"),
		       shop->served, shop->sent_away);
		for (i = 0; i < NUM_CHAIRS; i++) {
			printf("[%c] ", ((NUM_CHAIRS - _sem_getvalue(&shop->chairs)) > (i + NUM_CHAIRS - shop->index) % NUM_CHAIRS ? 'x' : ' '));
		}
		printf("\n");

		fflush(stdout);
	}

	/* Join threads */
	if(pthread_join(barber_thread, NULL)) {
		fprintf(stderr, "Error: failed to join pthread.\n");
		return 3;
	}
	for (i = 0; i < NUM_CUSTOMERS; i++) {
		if(pthread_join(customers[i], NULL)) {
			fprintf(stderr, "Error: failed to join pthread.\n");
			return 4;
		}
	}

	free(shop);
	return 0;
}
