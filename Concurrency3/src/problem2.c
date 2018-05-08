#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "rand.h"

#define SEATS 5
#define moveup(y) printf("\033[%dA", (y))

struct llist {
	struct llist *next;
	int			  value;
};

struct table {
	sem_t print;
	sem_t seats;
	sem_t forks[SEATS];
	char*  names[SEATS];
};

struct person {
	struct table* table;
	int           id;
	char*         status, left[2], right[2];
	int           wait;
	pthread_t     thread;
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

void print(char* string, struct table* table) {
	_sem_wait(&table->print);
	printf(string);
	_sem_post(&table->print);
}

/* Thread function */
void *thread(void *arg) {
	/* Convert void argument to buffer type */
	struct person* person = (struct person*) arg;
	int left, right, id;
	char* name;
	char buffer[100];

	id    = person->id;
	left  = id;
	right = (id - 1 + SEATS) % SEATS;
	name  = person->table->names[id];

	//sprintf(buffer, "%s (%d) exists\n", name, id);
	//print(buffer, person->table);

	while (1) {

		// Thinks
		person->wait = randomRange(1, 20);
		person->status = "think";
		_sem_post(&person->table->print);

		sleep(person->wait);
		person->wait = -1;

		// Get seat
		person->status = "wait ";
		_sem_post(&person->table->print);

		_sem_wait(&person->table->seats);
		
		// Get Forks
		_sem_wait(&person->table->forks[left]);
		sprintf(person->left, "%d", left);
		_sem_wait(&person->table->forks[right]);		
		sprintf(person->right, "%d", right);

		// Eat
		person->wait = randomRange(2, 9);
		person->status = "eat  ";
		_sem_post(&person->table->print);
		
		sleep(person->wait);
		person->wait = -1;

		// Put Forks
		_sem_post(&person->table->forks[left]);
		sprintf(person->left, "-");
		_sem_post(&person->table->forks[right]);
		sprintf(person->right, "-");

		// Stand up
		_sem_post(&person->table->seats);

	}

	return NULL;
}

/* Main function */
int main(int argc, char* argv[]) {

	int i;
	struct table* table;
	struct person* people[SEATS];
	char buffer[100];
	int fork_v, first = 1;
	
	// initialize tabe
	table = (struct table*) malloc(sizeof(struct table));
	_sem_init(&table->print, 0, 1);
	_sem_init(&table->seats, 0, SEATS - 1);
	for (i = 0; i < SEATS; i++) {
		_sem_init(&table->forks[i], 0, 1);
	}
	table->names[0] = "Plato        ";
	table->names[1] = "Aristotle    ";
	table->names[2] = "Socrates     ";
	table->names[3] = "Immanuel Kant";
	table->names[4] = "John Locke   ";
	
	/* Create people and their threads */
	for (i = 0; i < SEATS; i++) {
		people[i] = (struct person*) malloc(sizeof(struct person));
		people[i]->id = i;
		people[i]->table = table;
		people[i]->wait = 0;
		people[i]->status = "     ";
		sprintf(people[i]->left, "-");
		sprintf(people[i]->right, "-");		

		if(pthread_create(&people[i]->thread, NULL, thread, people[i])) {
			fprintf(stderr, "Error: failed to create pthread.\n");
			return 2;
		}
	}

	while (1) {
		_sem_wait(&table->print);

		if (first) {
			first = 0;
		} else {
			if (argc > 1 && strcmp(argv[1], "-f") == 0) {
				printf("\n");
			} else {
				moveup(SEATS+3);
			}
		}

		printf("Avaiable Forks: [");
		for (i = 0; i < SEATS; i++) {
			sem_getvalue(&table->forks[i], &fork_v);
			if (fork_v > 0) {
				sprintf(buffer, "%d", i);
			} else {
				sprintf(buffer, "-");
			}
			if (i < SEATS - 1) {
				printf("%s,", buffer);
			} else {
				printf("%s]\n", buffer);
			}
		}
	
		printf("Status:\nName          Id  Left Right Status Wait\n");

		for(i = 0; i < SEATS; i++) {
			printf("%s ", table->names[i]);
			printf("(%d) ", i);
			printf("%s    ", people[i]->left);
			printf("%s     ", people[i]->right);
			printf("%s  ", people[i]->status);
			if (people[i]->wait < 0) {
				printf("%s   \n", "-");
			} else {
				printf("%d   \n", people[i]->wait);
			}
		}

		fflush(stdout);

	}

	/* Join threads */
	for (i = 0; i < SEATS; i++) {
		if(pthread_join(people[i]->thread, NULL)) {
			fprintf(stderr, "Error: failed to join pthread.\n");
			return 5;
		}
	}

	for (i = 0; i < SEATS; i++) {
		free(people[i]);
	}
	free(table);
	return 0;
}
