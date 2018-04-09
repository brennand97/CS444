#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

struct message {
	int wait;
	int show;
};

struct buffer_element {
	struct buffer_element*   next;
	struct message           msg;
};

struct buffer {
	struct buffer_element*   head;
	struct buffer_element*   tail;
};

// Initialize the buffer
void initBuffer(struct buffer* b) {
	b->head = NULL;
	b->tail = NULL;
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
}

// Put message in buffer
void putBuffer(struct message msg, struct buffer* b) {
	struct buffer_element* n = (struct buffer_element*) malloc(sizeof(struct buffer_element));
	n->next = NULL;
	n->msg  = msg;

	if (b->head == NULL) {
		b->head = n;
	}
	b->tail->next = n;
	b->tail       = n;
}

// Pop message off buffer
int popBuffer(struct message* msg, struct buffer* b) {
	if (b->head == NULL) {
		return 1;
	}
	(*msg) = b->head->msg;

	struct buffer_element* head = b->head;
	b->head = head->next;
	if (head->next == NULL) { b->tail == NULL; }
	free(head);

	return 0;
}

// Main function
int main(void) {

	printf("hello\n");

	return 0;
}
