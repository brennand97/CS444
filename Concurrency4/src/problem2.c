#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "rand.h"

struct container {
	// Agent
	sem_t agentSem;
	sem_t tobacco;
	sem_t paper;
	sem_t match;
	
	// Pushers
	int   isTobacco;
	int   isPaper;
	int   isMatch;
	sem_t tobaccoSem;
	sem_t paperSem;
	sem_t matchSem;
	sem_t mutex;
};

struct input {
	struct container* con;
	int               id;
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

/* Agent Thread function */
void *agent_thread(void *arg) {
	/* Convert void argument */
	struct input *in = (struct input*) arg;
	struct container *con = in->con;
	int id = in->id;
	
	sem_t *semOne, *semTwo;
	char *type1, *type2;

	printf("Started agent %d\n", id);
	switch(id) {
		case 0:
			semOne = &con->tobacco;
			semTwo = &con->paper;
			type1 = "tobacco";
			type2 = "paper";
			break;
		case 1:
			semOne = &con->paper;
			semTwo = &con->match;
			type1 = "paper";
			type2 = "match";
			break;
		case 2:
			semOne = &con->tobacco;
			semTwo = &con->match;
			type1 = "tobacco";
			type2 = "match";
			break;
		default:
			fprintf(stderr, "Invalid id has been passed to thread.");
			return NULL;
	}

	while (1) {
		_sem_wait(&con->agentSem);
		printf("Providing materials: %s and %s\n", type1, type2);
		_sem_post(semOne);
		_sem_post(semTwo);
	}

	free(in);
	return NULL;
}

/* Pusher Thread function */
void *pusher_thread(void *arg) {
	/* Convert void argument */
	struct input *in = (struct input*) arg;
	struct container *con = in->con;
	int id = in->id;
	
	sem_t *sem1, *sem2, *sem3;
	int *flag1, *flag2, *flag3;

	printf("Started pusher %d\n", id);
	switch(id) {
		case 0:
			flag1 = &con->isPaper;
			flag2 = &con->isMatch;
			flag3 = &con->isTobacco;
			sem1  = &con->tobacco;
			sem2  = &con->matchSem;
			sem3  = &con->paperSem;
			break;
		case 1:
			flag1 = &con->isTobacco;
			flag2 = &con->isMatch;
			flag3 = &con->isPaper;
			sem1  = &con->paper;
			sem2  = &con->tobaccoSem;
			sem3  = &con->matchSem;
			break;
		case 2:	
			flag1 = &con->isPaper;
			flag2 = &con->isTobacco;
			flag3 = &con->isMatch;
			sem1  = &con->match;
			sem2  = &con->tobaccoSem;
			sem3  = &con->paperSem;
			break;
		default:
			fprintf(stderr, "Invalid id has been passed to thread.");
			return NULL;
	}

	while (1) {
		_sem_wait(sem1);
		_sem_wait(&con->mutex);
		if(*flag1) {
			*flag1 = 0;
			_sem_post(sem2);
		} else if (*flag2) {
			*flag2 = 0;
			_sem_post(sem3);
		} else {
			*flag3 = 1;
		}

		_sem_post(&con->mutex);
	}

	free(in);
	return NULL;
}

void makeCigarette(char* type) {
	int wait = randomRange(2,4);
	printf("Making cigarette: Smoker with %s\n", type);
	sleep(wait);
}

void smoke(char* type) {
	int wait = randomRange(5,8);
	printf("Smoking: Smoker with %s\n", type);
	sleep(wait);
}

/* Smoker Thread function */
void *smoker_thread(void *arg) {
	/* Convert void argument */
	struct input *in = (struct input*) arg;
	struct container *con = in->con;
	int id = in->id;
	
	sem_t *sem;
	char* name;

	printf("Started smoker %d\n", id);
	switch(id) {
		case 0:
			sem = &con->tobaccoSem;
			name = "tobacco";
			break;
		case 1:
			sem = &con->paperSem;
			name = "papper";
			break;
		case 2:
			sem = &con->matchSem;
			name = "match";
			break;
		default:
			fprintf(stderr, "Invalid id has been passed to thread.");
			return NULL;
	}

	while (1) {
		_sem_wait(sem);
		makeCigarette(name);
		_sem_post(&con->agentSem);
		smoke(name);
	}

	free(in);
	return NULL;
}

/* Main function */
int main(int argc, char* argv[]) {

	int i;
	struct container* con;
	struct input *in;
	pthread_t agents[3];
	pthread_t pushers[2];
	pthread_t smokers[3];
	
	// initialize tabe
	con = (struct container*) malloc(sizeof(struct container));
	_sem_init(&con->agentSem, 0, 1);
	_sem_init(&con->tobacco, 0, 0);
	_sem_init(&con->paper, 0, 0);
	_sem_init(&con->match, 0, 0);
	con->isTobacco = 0;
	con->isPaper = 0;
	con->isMatch = 0;
	_sem_init(&con->tobaccoSem, 0, 0);
	_sem_init(&con->paperSem, 0, 0);
	_sem_init(&con->matchSem, 0, 0);
	_sem_init(&con->mutex, 0, 1);

	// initialize threads
	for (i = 0; i < 3; i++) {

		in = (struct input*) malloc(sizeof(struct input));
		in->con = con;
		in->id = i;
		if(pthread_create(&agents[i], NULL, agent_thread, in)) {
			fprintf(stderr, "Error: failed to create pthread.\n");
			return 1;
		}
		
		in = (struct input*) malloc(sizeof(struct input));
		in->con = con;
		in->id = i;
		if(pthread_create(&pushers[i], NULL, pusher_thread, in)) {
			fprintf(stderr, "Error: failed to create pthread.\n");
			return 1;
		}		

		in = (struct input*) malloc(sizeof(struct input));
		in->con = con;
		in->id = i;
		if(pthread_create(&smokers[i], NULL, smoker_thread, in)) {
			fprintf(stderr, "Error: failed to create pthread.\n");
			return 1;
		}
	}

	/* Join threads */
	for (i = 0; i < 3; i++) {
		if(pthread_join(agents[i], NULL)) {
			fprintf(stderr, "Error: failed to join pthread.\n");
			return 2;
		}
		if(pthread_join(pushers[i], NULL)) {
			fprintf(stderr, "Error: failed to join pthread.\n");
			return 2;
		}
		if(pthread_join(smokers[i], NULL)) {
			fprintf(stderr, "Error: failed to join pthread.\n");
			return 2;
		}
	}

	free(con);
	return 0;
}
