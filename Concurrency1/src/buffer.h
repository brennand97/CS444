#ifndef BUFFER_H_
#define BUFFER_H_

#include <pthread.h>
#include <semaphore.h>

#define MAX_COUNT 32

struct message {
	int wait;
	int show;
};

struct buffer_element {
	struct buffer_element* next;
	struct message         msg;
};

struct buffer {
	struct buffer_element* head;
	struct buffer_element* tail;
	pthread_mutex_t        lock;
	sem_t                  free;
	sem_t                  items;
	int                    size;
};

int _sem_wait(sem_t* s);
int _sem_post(sem_t* s);

int  initBuffer(struct buffer* b);
void destroyBuffer(struct buffer* b);
int putBuffer(struct message msg, struct buffer* b);
int  popBuffer(struct message* msg, struct buffer* b);

#endif // BUFFER_H_
