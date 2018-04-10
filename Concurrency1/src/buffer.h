#ifndef BUFFER_H_
#define BUFFER_H_

#include <pthread.h>

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
	int                    size;
};

int  initBuffer(struct buffer* b);
void destroyBuffer(struct buffer* b);
int putBuffer(struct message msg, struct buffer* b);
int  popBuffer(struct message* msg, struct buffer* b);

#endif // BUFFER_H_
