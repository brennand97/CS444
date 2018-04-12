#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "buffer.h"
#include "rand.h"

typedef struct message message;

/* Producer thread function */
void *producer(void *arg) {
	/* Convert void argument to buffer type */
	struct buffer* buffer = (struct buffer*) arg;

	/* Produce items forever */
	while(1) {
		/* Create message */
		message msg = { .wait = randomRange(2, 9), .show = randomRange(0,10000) };
		putBuffer(msg, buffer);

		/* Sleep a random amount of time from 3 to 7 */
		sleep(randomRange(3, 7));
	}

	return NULL;
}

/* Consumer thread function */
void *consumer(void *arg) {	
	/* Convert void argument to buffer type */
	struct buffer* buffer = (struct buffer*) arg;

	/* Consume items forever */
	while(1) {
		/* Create mesage object for retrieval and retrieve from buffer */
		message msg;
		popBuffer(&msg, buffer);

		//printf("Sleep For: %d\n", msg.wait);
		/* Sleep for the wait period of the message then show the show value */
		sleep(msg.wait);
		printf("%d\n", msg.show);
	}

	return NULL;
}

/* Main function */
int main(int argc, char* argv[]) {

	int c_count, p_count;
	int i;
	struct buffer* buffer;
	
	if (argc < 2 || argc > 3) {
		printf("Syntax: %s <consumer count> [producer count]\n", argv[0]);
		return 666;
	} else {
		c_count = atoi(argv[1]);
		if (argc == 3) {
			p_count = atoi(argv[2]);
		} else {
			p_count = 1;
		}
	}
	
	/* Initialize the buffer */
	buffer = (struct buffer*) malloc(sizeof(struct buffer));
	if(initBuffer(buffer)) {
		fprintf(stderr, "Error: failed to initialize buffer.\n");
		return 1;
	}

	pthread_t c_thread[c_count], p_thread[p_count];

	/* Create consumer threads */
	for (i = 0; i < c_count; i++) {
		if(pthread_create(&c_thread[i], NULL, consumer, buffer)) {
			fprintf(stderr, "Error: failed to create consumer thread #%d.\n", i);
			return 2;
		}
	}

	/* Create producer threads */
	for (i = 0; i < p_count; i++) {
		if(pthread_create(&p_thread[i], NULL, producer, buffer)) {
			fprintf(stderr, "Error: failed to create producer thread #%d.\n", i);
			return 3;
		}
	}

	/* Join consumer threads */
	for (i = 0; i < c_count; i++) {
		if(pthread_join(c_thread[i], NULL)) {
			fprintf(stderr, "Error: failed to join consumer thread.\n");
			return 4;
		}
	}

	/* Join producer threads */
	for (i = 0; i < p_count; i++) {
		if(pthread_join(p_thread[i], NULL)) {
			fprintf(stderr, "Error: failed to join producer thread.\n");
			return 5;
		}
	}

	/* Celan up the buffer */
	destroyBuffer(buffer);
	free(buffer);
	return 0;
}
