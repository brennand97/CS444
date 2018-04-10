#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "buffer.h"

// Initialize the buffer
int initBuffer(struct buffer* b) {
	b->head = NULL;
	b->tail = NULL;
	b->size = 0;
	
	if (pthread_mutex_init(&b->lock, NULL) != 0)
    {
        printf("Error: mutex init failed.\n");
        return 1;
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
	// Lock the buffer mutex
	pthread_mutex_lock(&b->lock);

	if (b->size >= MAX_COUNT) {
		// Unlock the buffer mutex
		pthread_mutex_unlock(&b->lock);
		return 1;
	}

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

	// Unlock the buffer mutex
	pthread_mutex_unlock(&b->lock);
	
	return 0;
}

// Pop message off buffer
int popBuffer(struct message* msg, struct buffer* b) {	
	// Lock the buffer mutex
	pthread_mutex_lock(&b->lock);

	if (b->head == NULL) {
		// Unlock the buffer mutex
		pthread_mutex_unlock(&b->lock);
		return 1;
	}
	(*msg) = b->head->msg;

	struct buffer_element* head = b->head;
	b->head = head->next;
	if (head->next == NULL) { b->tail == NULL; }
	free(head);

	b->size--;

	// Unlock the buffer mutex
	pthread_mutex_unlock(&b->lock);

	return 0;
}
