#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "buffer.h"

typedef struct message message;

int randomRange(int min, int max) {
	return rand() % (max + 1 - min) + min;
}

void *producer(void *arg) {
	struct buffer* buffer = (struct buffer*) arg;

	while(1) {
		message msg = { .wait = randomRange(2, 9), .show = randomRange(0,100) };
		while(putBuffer(msg, buffer)) {
			sleep(1);
		}
		sleep(randomRange(3, 7));
	}

	return NULL;
}

void *consumer(void *arg) {
	struct buffer* buffer = (struct buffer*) arg;

	while(1) {
		message msg;
		int result = popBuffer(&msg, buffer);
		if (result != 0) {
			sleep(1);
		} else {
			sleep(msg.wait);
			printf("%d\n", msg.show);
		}
	}

	return NULL;
}

/* Main function */
int main(void) {
	srand(time(NULL));
	
	struct buffer* buffer = (struct buffer*) malloc(sizeof(struct buffer));
	if(initBuffer(buffer)) {
		fprintf(stderr, "Error: failed to initialize buffer.\n");
		return 1;
	}

	pthread_t c_thread, p_thread;

	/* Create consumer thread */
	if(pthread_create(&c_thread, NULL, consumer, buffer)) {
		fprintf(stderr, "Error: failed to create consumer thread.\n");
		return 2;
	}

	/* Create producer thread */
	if(pthread_create(&p_thread, NULL, producer, buffer)) {
		fprintf(stderr, "Error: failed to create producer thread.\n");
		return 3;
	}

	/* Join consumer thread */
	if(pthread_join(c_thread, NULL)) {
		fprintf(stderr, "Error: failed to join consumer thread.\n");
		return 4;
	}

	/* Join producer thread */
	if(pthread_join(p_thread, NULL)) {
		fprintf(stderr, "Error: failed to join producer thread.\n");
		return 5;
	}

	destroyBuffer(buffer);
	free(buffer);
	return 0;
}
