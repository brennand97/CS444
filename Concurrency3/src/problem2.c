#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "rand.h"

#define INSERTERS 1
#define DELETERS 1
#define SEARCHERS 3

#define moveup(y) printf("\033[%dA", (y))

struct list {
	struct node  *head;
	sem_t         deleter;
	sem_t		  inserter;
	sem_t         searches;
	sem_t         active_searches;
	sem_t         all_searches_complete;
	int           waiting;
};

struct node {
	struct node  *next;
	int			  value;
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

struct node* search(struct list *l, int value) {
	struct node *n;
	if (_sem_getvalue(&l->deleter) == 0) {
		l->waiting++;
		_sem_wait(&l->searches);
	}
	_sem_post(&l->active_searches);
	if (_sem_getvalue(&l->all_searches_complete) > 0) {
		_sem_wait(&l->all_searches_complete);
	}
	
	n = l->head;
	while (n != NULL) {
		if (n->value == value) {
			break;
		}
		n = n->next;
	}
	
	_sem_wait(&l->active_searches);
	if (_sem_getvalue(&l->active_searches) == 0) {
		_sem_post(&l->all_searches_complete);
	}

	return n;
}

void insert(struct list *l, int value) {
	struct node *n, *nt;

	_sem_wait(&l->deleter);
	_sem_wait(&l->inserter);

	n = (struct node*) malloc(sizeof(struct node));
	n->value = value;
	n->next = NULL;

	nt = l->head;
	while(nt != NULL && nt->next != NULL) {
		nt = nt->next;
	}

	nt->next = n;

	_sem_post(&l->deleter);
	_sem_post(&l->inserter);
}

void delete(struct list *l, int value) {
	int i;
	struct node *n, *np;

	_sem_wait(&l->deleter);
	_sem_wait(&l->inserter);
	if (_sem_getvalue(&l->active_searches) > 0) {
		_sem_wait(&l->all_searches_complete);
	}


	while(n != NULL) {
		if (n->value == value) {
			break;
		}
		np = n;
		n = n->next;
	}
	if (np != NULL && n != NULL) {
		np->next = n->next;
		free(n);
	}

	_sem_post(&l->all_searches_complete);
	for (i = 0; i < l->waiting; i++) {
		_sem_post(&l->searches);
	}
	_sem_post(&l->deleter);
	_sem_post(&l->inserter);
}

/* Thread function */
void *inserter(void *arg) {
	/* Convert void argument */
	struct list* list = (struct list*) arg;

	return NULL;
}

/* Thread function */
void *deleter(void *arg) {
	/* Convert void argument */
	struct list* list = (struct list*) arg;

	return NULL;
}

/* Thread function */
void *searcher(void *arg) {
	/* Convert void argument */
	struct list* list = (struct list*) arg;

	return NULL;
}

/* Main function */
int main(int argc, char* argv[]) {

	int i;
	struct list *list;
	struct node *n, *np;
	pthread_t inserters[INSERTERS];
	pthread_t deleters[DELETERS];
	pthread_t searchers[SEARCHERS];
	
	// initialize tabe
	list = (struct list*) malloc(sizeof(struct list));
	_sem_init(&list->deleter, 0, 1);
	_sem_init(&list->inserter, 0, 1);
	_sem_init(&list->searches, 0, 0);
	_sem_init(&list->active_searches, 0, 0);
	_sem_init(&list->all_searches_complete, 0, 0);

	for (i = 0; i < INSERTERS; i++) {
		// initilize pthreads
		if(pthread_create(&inserters[i], NULL, inserter, list)) {
			fprintf(stderr, "Error: failed to create pthread.\n");
			return 2;
		}
	}
	
	for (i = 0; i < DELETERS; i++) {
		// initilize pthreads
		if(pthread_create(&deleters[i], NULL, deleter, list)) {
			fprintf(stderr, "Error: failed to create pthread.\n");
			return 2;
		}
	}

	for (i = 0; i < SEARCHERS; i++) {
		// initilize pthreads
		if(pthread_create(&searchers[i], NULL, searcher, list)) {
			fprintf(stderr, "Error: failed to create pthread.\n");
			return 2;
		}
	}


	/* Join threads */
	for (i = 0; i < INSERTERS; i++) {
		if(pthread_join(inserters[i], NULL)) {
			fprintf(stderr, "Error: failed to join pthread.\n");
			return 5;
		}
	}

	/* Join threads */
	for (i = 0; i < DELETERS; i++) {
		if(pthread_join(deleters[i], NULL)) {
			fprintf(stderr, "Error: failed to join pthread.\n");
			return 5;
		}
	}

	/* Join threads */
	for (i = 0; i < SEARCHERS; i++) {
		if(pthread_join(searchers[i], NULL)) {
			fprintf(stderr, "Error: failed to join pthread.\n");
			return 5;
		}
	}

	n = list->head;
	np = NULL;
	while(n != NULL) {
		np = n;
		n = n->next;
		free(np);
	}
	free(list);
	return 0;
}
