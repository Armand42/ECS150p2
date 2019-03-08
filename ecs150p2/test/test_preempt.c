#include <stdio.h>
#include <stdlib.h>
#include <uthread.h>
#include <time.h> 

void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;

    // Stroing start time
    clock_t start_time = clock();

    // looping till required time is not acheived
    while (clock() < start_time + milli_seconds);
}

int thread2(void* arg){
	printf("Hello from thread 2!\n");
	delay(2000);
	return 0;
}

int thread1(void* arg){	
	printf("Hello from thread 1! I'm about to switch to thread 2 without yielding\n");
	uthread_create(thread2, NULL);
	delay(1000);
	printf("Welcome back to thread 1! I just got set back to the current thread thanks to preemption\n");
	return 0;
}

int main(){
	int* retval = NULL;	
	uthread_create(thread1,NULL);	
        uthread_join(2, retval);
}
