/*
 * Thread creation and yielding test
 *
 * Tests the creation of multiples threads and the fact that a parent thread
 * should get returned to before its child is executed. The way the printing,
 * thread creation and yielding is done, the program should output:
 *
 * thread1
 * thread2
 * thread3
 */

#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

int thread3(void* arg)
{
	uthread_yield();
	printf("thread%d\n", uthread_self());
	return 22;
}

int thread2(void* arg)
{
	uthread_create(thread3, NULL);
	uthread_yield();
	printf("thread%d\n", uthread_self());
	return 33;
}

int thread1(void* arg)
{
	int t2 = uthread_create(thread2, NULL);
	uthread_join(t2, NULL);
	uthread_yield();
	printf("thread%d\n", uthread_self());
	uthread_yield();
	return 55;
}

int main(void)
{
	int ret_val;
	uthread_join(uthread_create(thread1, NULL), &ret_val);
	return 0;
}
