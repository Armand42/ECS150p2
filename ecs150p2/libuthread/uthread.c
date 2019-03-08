#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "context.h"
#include "preempt.h"
#include "queue.h"
#include "uthread.h"

#define RUNNING 0
#define READY 1
#define BLOCKED 2
#define ZOMBIE 3

// Thread Control Block contains (Thread ID, state, child thread's return value, context, stack)
struct TCB {
	uthread_t tid;
	uthread_ctx_t thread_ctx;
	int child_return;
	void *stack;
	int state;
};

// Blocked TCB is some parent thread that is being blocked
// by a still-running child thread with tid of blocking_TID
struct TCB2{
	struct TCB *blocked_TCB;
	uthread_t blocking_TID;

};

// Global Threads
static struct TCB *current_thread; 
static struct TCB *prev_thread; 

// Number of current active threads
int thread_count = 0;

// Number to assign to next new thread's TID
int tid_increment = 0;

// Global Queues for specific thread states
queue_t ready_state;	// 1
queue_t dead_tids;  // 2
queue_t join_state;	// 3

/* Registers main thread to the library by making its own tcb and adding it to running queue */
void initialize_main_thread(void) {
	dead_tids = queue_create();
	preempt_start();
	current_thread = (struct TCB*)malloc(sizeof(struct TCB));
	prev_thread = (struct TCB*)malloc(sizeof(struct TCB));

	// Main thread gets TID of 0
	current_thread->tid = 0;
	current_thread->state = RUNNING;
	
	thread_count++;
	tid_increment++;
}

/*
 * This function is to be called from the currently active and running thread in
 * order to yield for other threads to execute.
 */
void uthread_yield(void){
	// Don't want to switch to different thread while we're mid-yield
	preempt_disable();
	// Save the currently running thread
	prev_thread = current_thread;
	if (current_thread->state != BLOCKED) {
		queue_enqueue(ready_state, current_thread);
	}
	// Dequeue oldest ready thread and save it into save_thread
	void *save_thread;
	queue_dequeue(ready_state, &save_thread);
	// Typecast it back to a TCB* because it was a void ptr
	struct TCB* save_thread_casted = (struct TCB*)save_thread;
	// Our new current running thread is what we just dequeued from the ready queue
	current_thread = save_thread_casted;

	preempt_enable();
	uthread_ctx_switch(&prev_thread->thread_ctx, &current_thread->thread_ctx);
	// Enable preempt once we're done context switching
}

/* Returns current TCB ID */
uthread_t uthread_self(void)
{
	preempt_disable();
	preempt_enable();
	return current_thread->tid;
}

int uthread_create(uthread_func_t func, void *arg){
	preempt_disable(); // not switch to a different thread, if you enable preempting you can possibly switch to a new thread without being created
	// Getting called for the first time
	if (thread_count == 0) {
		ready_state = queue_create();
		join_state = queue_create();
		initialize_main_thread();
	}
	
	// Init the other thread
	struct TCB *thread = (struct TCB*)malloc(sizeof(struct TCB));
	thread->tid = tid_increment;
	thread->stack = uthread_ctx_alloc_stack();
	thread->state = READY;
	// Init thread context -- give it pointer to ctx, stack, function, arg
	uthread_ctx_init(&thread->thread_ctx, thread->stack, func, arg);
	// Put it in the read queue indicating its ready to be run
	queue_enqueue(ready_state, thread); // first main thread stays running, main thread, enqueue new thread if not the first time calling it
	// Return tid of newly created child thread and increment tid_increment for the next uthread_create() call
	preempt_enable();
	return tid_increment++; // main thread tid is 0, but increment to next thread and return that tid
}

/* Checks for existence of a TID in a queue */
int tid_exist(void* current_tid, void *arg) {
        struct TCB2* data_TID1  = (struct TCB2*)current_tid;
        int data_TID2 = *(int*)arg;
        if (data_TID1->blocking_TID == data_TID2) { // looking to see if child was in some queue
                return 1;
        }
        return 0;
}

int find(void* data, void* arg)
{
	if(*(int*)data == *(int*)arg)
	{
		return 1;
	}
	return 0;
}

void uthread_exit(int retval) {
	preempt_disable();
	queue_enqueue(dead_tids, &current_thread->tid);
	void *data = NULL;
	uthread_ctx_t dead_thread;
	void *save_thread = (struct TCB*)malloc(sizeof(struct TCB));
	// Thread that just ended is current thread. If its tid is in join state, it was blocking something.
	queue_iterate(join_state, tid_exist, &current_thread->tid, &data);

	// if found, free the current process and reenqueue the blocked process	to a new queue
	if (data != NULL) {
		uthread_ctx_destroy_stack(current_thread->stack);
		free(current_thread);
		struct TCB2* data_found = (struct TCB2*)data;

		// Put blocked parent thread back into ready queue
		data_found->blocked_TCB->state = READY;
		queue_enqueue(ready_state, data_found->blocked_TCB); // enqueue thing that was just blocked, take parent that was being blocked
		// Take the return value from that child that was blocking the parent, set our child_return value in the parent struct
		data_found->blocked_TCB->child_return = retval;		
		
	}
	// Dequeue next thread from the queue and mark that as our new current thread
	queue_dequeue(ready_state, &save_thread);	
	struct TCB* save_thread_casted = (struct TCB*)save_thread;
	current_thread = save_thread_casted;

	uthread_ctx_switch(&dead_thread, &current_thread->thread_ctx);
	preempt_enable();
}

/*
 * This function makes the calling thread wait for the thread @tid to complete
 * and assign the return value of the finished thread to @retval (if @retval is
 * not NULL).
 * Note: A parent usually "joins" its child -- like waiting for a child process
 */
int uthread_join(uthread_t tid, int *retval) {
	// The main thread cannot be joined
	preempt_disable();
	if (tid == 0) {
		return -1;
	}

	int check = 0;
	
	void* found_dead = NULL;	//on queue of dead tids, if we find something, set found dead to whatever find returns
	queue_iterate(dead_tids, find, &tid, &found_dead); // if we find it we stop
	// we need to check if a tid is dead or we will end up yielding to nothing
	if(found_dead) {
		current_thread->state = READY; // if child haden't finished it be blocked, its not blocking parent so we can set the parent to be ready and not blocked
	}
	else {
		struct TCB2 *state_process = (struct TCB2*)malloc(sizeof(struct TCB2));
		state_process->blocked_TCB = current_thread; // if found in dead threads, parent is ready
		state_process->blocking_TID = tid;
		queue_enqueue(join_state, state_process);
		current_thread->state = BLOCKED;
	}
	
	// While there are still threads left to run (and that are ready)
	while (queue_length(ready_state) != 0 ) {
		// Current thread yields
		uthread_yield();
		// Make sure we don't set the return value multiple times
		if(check == 0) {
			check = 1;			
			if(retval) {
				*(retval) = current_thread->child_return;
			}
		}
	}
	preempt_enable();
	return 0;
}
