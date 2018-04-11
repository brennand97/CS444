#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "buffer.h"

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

// Initialize the buffer
int initBuffer(struct buffer* b) {
	b->head = NULL;
	b->tail = NULL;
	b->size = 0;
	
	if (pthread_mutex_init(&b->lock, NULL) != 0)
    {
        printf("Error: mutex init failed.\n");
        return -11;
    }

	if (sem_init(&b->free,0,MAX_COUNT)) {
		fprintf(stderr, "Error: sem_init failed\n");
	}

	if (sem_init(&b->items,0,0)) {
		fprintf(stderr, "Error: sem_init failed\n");
	}

	return 0;
}

// Destroy the buffer
void destroyBuffer(struct buffer* b) {
	if (b->head == NULL) {
		return;
	}

	struct buffer_element *e_n, *e = b->head;
	while((e_n = e->next) != NULL) {
		free(e);
		e = e_n;
	}
	free(e);

	pthread_mutex_destroy(&b->lock);
}

// Put message in buffer
int putBuffer(struct message msg, struct buffer* b) {
	// Get hold of a free spot
	_sem_wait(&b->free);
	// Lock the buffer mutex
	pthread_mutex_lock(&b->lock);

	struct buffer_element* n = (struct buffer_element*) malloc(sizeof(struct buffer_element));
	n->next = NULL;
	n->msg  = msg;

	if (b->head == NULL) {
		b->head = n;
		b->tail = n;
	} else {
		b->tail->next = n;
		b->tail       = n;
	}
	
	b->size++;

	// Post to item sem
	_sem_post(&b->items);
	// Unlock the buffer mutex
	pthread_mutex_unlock(&b->lock);
	
	return 0;
}

// Pop message off buffer
int popBuffer(struct message* msg, struct buffer* b) {
	// Get hold of an item
	_sem_wait(&b->items);
	// Lock the buffer mutex
	pthread_mutex_lock(&b->lock);

	
	(*msg) = b->head->msg;

	struct buffer_element* head = b->head;
	b->head = head->next;
	if (head->next == NULL) { b->tail == NULL; }
	free(head);

	b->size--;

	// Post to a free item
	_sem_post(&b->free);
	// Unlock the buffer mutex
	pthread_mutex_unlock(&b->lock);

	return 0;
}
