#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "buffer.h"

// Main function
int main(void) {

	printf("Starting Buffer Test...\n");
	
	struct buffer* buffer = (struct buffer*) malloc(sizeof(struct buffer));
	if(initBuffer(buffer)) {
		printf("Error: failed to initialize buffer.\n");
		return 1;
	}

	int i;
	for(i = 0; i < 10; i++) {
		struct message t_msg = { .wait = 3, .show = i };
		putBuffer(t_msg, buffer);
	}

	struct message msg;
	while(popBuffer(&msg, buffer) == 0) {
		printf("Msg: Show: %d\n", msg.show);
	}

	destroyBuffer(buffer);
	free(buffer);
	return 0;
}
