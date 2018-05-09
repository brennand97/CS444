#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "rand.h"

#define MAX 3
#define MAX_T 3
#define moveup(y) printf("\033[%dA", (y))

struct lock {
	sem_t busy;
	sem_t avaiable;
	sem_t add;
	int   locked;
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


void start(struct lock *l) {
	_sem_wait(&l->add);
	
	_sem_wait(&l->avaiable);
	_sem_wait(&l->busy);

	if (_sem_getvalue(&l->busy) == 0) {
		l->locked = 1;
		printf("LOCKED\n");
	}
	
	_sem_post(&l->add);
}

void complete(struct lock *l) {
	int i;
	
	_sem_post(&l->add);

	_sem_post(&l->busy);
	if (l->locked && _sem_getvalue(&l->busy) == MAX) {
		l->locked = 0;
		for (i = 0; i < MAX; i++) {
			_sem_post(&l->avaiable);
		}
		printf("UNLOCKED\n");
	} else if (!l->locked) {
		_sem_post(&l->avaiable);
	}

	_sem_post(&l->add);
}

/* Thread function */
void *thread(void *arg) {
	/* Convert void argument */
	struct lock *lock = (struct lock*) arg;
	int wait;
	
	while (1) {
		start(lock);

		wait = randomRange(2,8);
		printf("Start (sem: %d/%d) waiting  for %d\n", MAX - _sem_getvalue(&lock->busy), MAX, wait);
		fflush(stdout);
		sleep(wait);

		complete(lock);

		wait = randomRange(6,12);
		printf("Done  (sem: %d/%d) cooldown for %d\n", MAX - _sem_getvalue(&lock->busy), MAX, wait);
		fflush(stdout);
		sleep(wait);
	}

	return NULL;
}

/* Main function */
int main(int argc, char* argv[]) {

	int i;
	struct lock* lock;
	pthread_t threads[MAX_T];
	
	// initialize tabe
	lock = (struct lock*) malloc(sizeof(struct lock));
	_sem_init(&lock->avaiable, 0, MAX);
	_sem_init(&lock->busy, 0, MAX);
	_sem_init(&lock->add, 0, 1);

	for (i = 0; i < MAX_T; i++) {
		if(pthread_create(&threads[i], NULL, thread, lock)) {
			fprintf(stderr, "Error: failed to create pthread.\n");
			return 2;
		}
		sleep(2);
	}

	/* Join threads */
	for (i = 0; i < MAX_T; i++) {
		if(pthread_join(threads[i], NULL)) {
			fprintf(stderr, "Error: failed to join pthread.\n");
			return 5;
		}
	}

	free(lock);
	return 0;
}
