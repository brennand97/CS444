#include <stdio.h>
#include <time.h>

#include "rand.h"

#include "mtwister.h"

unsigned long long rdrnd(void) {
    // Get a random number from the rdrand assembly instruction
    if ((is_rdrnd_aval()) == 0) return (unsigned long long) 0;
    register unsigned long long rand_num;
    __asm__ __volatile__ ( "rdrand %0;" : "=r" ( rand_num ) );
    return rand_num;
}

int is_rdrnd_aval() {
    unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;

	//char vendor[13];
	
	eax = 0x01;

	__asm__ __volatile__(
	                     "cpuid;"
	                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
	                     : "a"(eax)
	                     );
	
	if(ecx & 0x40000000){
		return 1;
	}
	else{
		return 0;
	}
}

MTRand r;
int r_init = 0;
int randomRange(int min, int max) {
    if (is_rdrnd_aval()) {
        return ((unsigned int) rdrnd()) % (max - min) + min;
    } else {
		if (!r_init) {
			r = seedRand(time(NULL));
			r_init = 1;
		}
        return (int) (genRand(&r) * (max - min)) + min;
    }
}
