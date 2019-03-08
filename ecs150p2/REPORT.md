# ecs150p2 

## Authors
Samuel Waters

Armand Nasseri

### High-Level Design Choices

### Testing
##### Phase 1 Test
We wrote test_queue.c to account for all possible cases when using
our queue. We documented this in comments, but to give a high level overview: 
We created a queue "q" to pass around to our different methods used to test
each functionality of our queue implementation. We also declared an array of 
numbers to enqueue. We made sure queue_create() didn't fail on malloc()ing, 
and within test_enqueue(), test_dequeue(), and test_delete() verified that
passing in a NULL queue or NULL data resulted in an error for those
respective methods. We also made sure to check that queue_length was
returning the expected values for certain stages within our enqueueing and
dequeueing tests. After enqueuing our 5 integers as void pointers, we
asserted that deleting the number "3" resulted in the proper behavior (same
address as our 3 from the array and the queue's length is now 3), then
dequeued the next 3 elements, leaving only the number 1 left in the queue.
We then tried destroying the queue with one element, which we verified
resulted in an error. Finally, we dequeued the last element and destroyed the
queue. Test_iterator was taken from Professor Porquet's example in the doc.
##### Phase 2 Test
For phase 2, we tested our uthread library by verifying we received the proper
output for uthread_hello and uthread_yield.

##### Phase 3 Test
In our test for phase 3, we're testing the behavior of our join method by
joining our main thread with thread 1 (blocking main thread). Then, thread1
joins thread 2 (thread 1 now also blocked), then yields to thread 2. Thread 2
and newly-created thread 3 then yield to each other until thread2 prints its
TID and finishes execution, thus unblocking thread 1. Then, thread 3 prints its
execution before finishing, which results in thread1 resuming execution and
finally printing its TID.

##### Phase 4 Test
For our phase 4 test, we're testing preemption by using a custom delay function
to cause a thread to pause while mid-execution, and allow time for our alarm to
fire again and automatically yield to the next available thread. TO show this,
we're spawning a new thread off of main which calls the thread1 function, then 
joining it from main. Our thread1 then prints its first print statement, then 
spawns a new thread and delays for one second. As a result of our preemtion
being enabled, thread1 automatically yields to thread 2 as it's mid-delay, and 
thread2 prints its message. Then, we delay once more and allow for preemtion to
take effect and switch back to thread1. Thread1 prints its second message, then
finishes execution along with thread2.

### More Detailed Implementation Decisions:
#### Phase 1:
###### Makefile
We wrote our makefile to generate a static library, libuthread.a, by using
"ar rcs" after compiling our .c files into object files using gcc.
We refined our makefile by using "%" for repeated filenames in the same rule.
By doing this, we created one blanket rule for compiling our .c files into .o
files using our .c and .h files. We additionally employed variables to reduce
redundancy, and put in an option to debug our make process with either
shortened or more verbose output.
###### Queue
We created a FIFO queue using a linkedlist implementation. We relied on two 
pointers to keep track of a head and a tail variable in our linked list queue 
so that we could keep track of when to shift nodes accordingly due to enqueue or 
dequeue operations. The assignment description required us to use a data 
structure in which "all operations (apart from the iterate and delete 
operation) must be O(1)." Therefore, a linked list proved to be the most 
suitable data structure because our enqueue and dequeueoperations took at most
O(1) time due to simple pointer manipulation. All otherfunctions in the queue.c
file had similar pointer manipulation characteristics as well as the standard 
linked list traversal through a node traversal through the queue.

#### Phase 2:
We decided to hold all of our information for each thread in its own struct,
which we called "TCB". We also defined macros for the thread's state, with
RUNNING = 0, READY = 1, BLOCKED = 2, and ZOMBIE = 2. To additionally facilitate
the stages of a thread's lifecycle, we created separate queues for ready and
blocked threads. To help with context switching, our currently running thread
is stored in current_thread, and our previously running thread is stored in 
prev_thread.

#### Phase 3:
In this phase, we needed to take into account that our threads' resources
cannot automatically be freed when a thread exits. In order to account for this
we had to refactor our uthread_join() method. If a thread ended, we needed to
iterate through our join_state queue to see if a thread was blocking anything
before we joined it. The join_state queue is meant to keep track of everything 
we blocked in the queue. As we iterated through the queue, we applied our
tid_exist function to every element in the queue to see if our blocked thread
existed there. Then we needed to save the return value in our data.
#### Phase 4:
In this phase, we needed to implement the preemption methods in order to enable
or disable a thread's yield functionality. Using the GNU's example template for
basic signal handling, we adapted it to meet the frequency requirement of 
firing a signal at 100 HZ everytime the function was called. We used the 
sigprocmask() function to block other possible signals for a thread even though
we were officially only dealing with one possible signal. In our uthread 
program, we called the preempt_disable() function at the beginning of all our
functions to ensure that global variable do not get changed midway through a 
thread's execution. It allowed us to become in a state where a variable was
stable for switching between threads. At the end of each function, we called
the preempt_enable() function so that we could yield during a stable state
based on the time of thread execution.
### Sources
https://www.geeksforgeeks.org/operarting-system-thread/

https://www.geeksforgeeks.org/lifecycle-and-states-of-a-thread-in-java/

https://www.geeksforgeeks.org/linked-list-set-1-introduction/ 

https://www.geeksforgeeks.org/time-delay-c/

We refered to these sources when we were trying to understand the high level
design of processing threads according to their state in the thread scheduling
cycle as well as how to store them accordingly in our queue data structure.
