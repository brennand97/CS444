#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "rand.h"

#define INSERTERS 2
#define DELETERS  2
#define SEARCHERS 10

#define WAIT 1

#define moveup(y) printf("\033[%dA", (y))

struct list {
	struct node  *head;
	sem_t         deleter;
	sem_t		  inserter;
	sem_t         searches;
	sem_t         active_searches;
	sem_t         all_searches_complete;
	int           waiting;
	int			  deleting;
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

int active(struct list *l) {
	return (_sem_getvalue(&l->deleter) == 0) + _sem_getvalue(&l->active_searches);
}

// list search function
struct node* search(struct list *l, int value) {
	struct node *n;
	if (l->deleting) {
		// If deleting is currently happening wait on searches
		// and increment waiting counter
		l->waiting++;
		_sem_wait(&l->searches);
	}
	// post to active search semaphore to infrom list of activity
	_sem_post(&l->active_searches);
	if (_sem_getvalue(&l->all_searches_complete) > 0) {
		// if all searches complete is true set to false while running
		_sem_wait(&l->all_searches_complete);
	}
	printf("S--    (%d Active)\n", active(l));
	
	// find node that matches value
	n = l->head;
	while (n != NULL) {
		if (n->value == value) {
			break;
		}
		n = n->next;
	}

	// sleep so that interactions can be observed
	sleep(WAIT);
	
	// decrement active search
	_sem_wait(&l->active_searches);
	if (_sem_getvalue(&l->active_searches) == 0) {
		// if last active search set all searches complte to true
		_sem_post(&l->all_searches_complete);
	}

	// return node (or null)
	return n;
}

// list insert function
void insert(struct list *l, int value) {
	struct node *n, *nt;

	// wait for deleter "mutex"
	_sem_wait(&l->deleter);
	// wait for inserter "mutex"
	_sem_wait(&l->inserter);
	printf("-I-    (%d Active)\n", active(l));

	// create new node
	n = (struct node*) malloc(sizeof(struct node));
	n->value = value;
	n->next = NULL;

	// get tail of list
	nt = l->head;
	while(nt != NULL && nt->next != NULL) {
		nt = nt->next;
	}

	if (nt != NULL) {
		// set next of tail to new node
		nt->next = n;
	} else {
		// if no head set to head
		l->head = n;
	}

	// sleep so that interactions can be observed
	sleep(WAIT);

	// release "mutexes"
	_sem_post(&l->deleter);
	_sem_post(&l->inserter);
}

// list delete function
void delete(struct list *l, int value) {
	int i;
	struct node *n, *np;

	// wait for deleter "mutex"
	_sem_wait(&l->deleter);
	// wait for insterer "mutex"
	_sem_wait(&l->inserter);
	if (_sem_getvalue(&l->active_searches) > 0) {
		// if active searches wait for all to complete
		_sem_wait(&l->all_searches_complete);
	}
	// set deleting flag
	l->deleting = 1;
	printf("--D    (%d Active)\n", active(l));

	// find first node with value
	n = l->head;
	np = NULL;
	while(n != NULL) {
		if (n->value == value) {
			break;
		}
		np = n;
		n = n->next;
	}

	if (n != NULL) {
		// update to new connection
		if (np != NULL) {
			// set previous to next
			np->next = n->next;
		} else {
			// if pervious is null set head to next
			l->head = n->next;
		}
		// free the node
		free(n);
	}

	// sleep so that interactions can be observed
	sleep(WAIT);

	// reset deleting flag
	l->deleting = 0;
	// reset all searches complete flag
	_sem_post(&l->all_searches_complete);
	for (i = 0; i < l->waiting; i++) {
		// release all waiting searches
		_sem_post(&l->searches);
	}
	l->waiting = 0;
	// release deleter and inerter "mutexes"
	_sem_post(&l->deleter);
	_sem_post(&l->inserter);
}

/* Thread function */
void *inserter(void *arg) {
	/* Convert void argument */
	struct list* list = (struct list*) arg;
	int wait;

	while(1) {

		// wait and insert random values
		wait = randomRange(1,10);
		sleep(wait);
		insert(list, randomRange(0,20));

	}

	return NULL;
}

/* Thread function */
void *deleter(void *arg) {
	/* Convert void argument */
	struct list* list = (struct list*) arg;
	int wait;

	while(1) {

		// wait and delter random values
		wait = randomRange(1,10);
		sleep(wait);
		delete(list, randomRange(0,20));

	}

	return NULL;
}

/* Thread function */
void *searcher(void *arg) {
	/* Convert void argument */
	struct list* list = (struct list*) arg;
	int wait;

	while(1) {

		// wait and serach for random values
		wait = randomRange(1,10);
		sleep(wait);
		search(list, randomRange(0,20));

	}

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
	_sem_init(&list->all_searches_complete, 0, 1);

	printf("S: Searching\nI: Inserting\nD: Deleting\n");

	// initilize threads
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

	// free list
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
