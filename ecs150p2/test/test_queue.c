#include <stdio.h>
#include <queue.h>
#include <assert.h>

/* Callback function that increments items by a certain value */
static int inc_item(void *data, void *arg)
{
	int *a = (int*)data;
	int inc = (int)(long)arg;

	*a += inc;

	return 0;
}

/* Callback function that finds a certain item according to its value */
static int find_item(void *data, void *arg)
{
	int *a = (int*)data;
	int match = (int)(long)arg;

	if (*a == match)
	return 1;

	return 0;
}

void test_iterator(void)
{
	queue_t q;
	int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	int i;
	int *ptr;
	
	/* Initialize the queue and enqueue items */
	q = queue_create();
	for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
		queue_enqueue(q, &data[i]);
	
	/* Add value '1' to every item of the queue */
	queue_iterate(q, inc_item, (void*)1, NULL);
	assert(data[0] == 2);
	
	/* Find and get the item which is equal to value '5' */
	ptr = NULL;
	queue_iterate(q, find_item, (void*)5, (void**)&ptr);
	assert(ptr != NULL);
	assert(*ptr == 5);
	assert(ptr == &data[3]);
}


void test_dequeue(queue_t *queue, void* one) {
	void *data;
	// Return: -1 if queue or data are NULL, or if the queue is empty.
	assert(queue_dequeue(NULL, (void*)5) == -1);
	assert(queue_dequeue(*queue, NULL) == -1);
	// Oldest element should be the number 1
	queue_dequeue(*queue, &data);
	assert(data == one);
	// Dequeue remaining 3 elements
	for (int i=0; i<3; i++) {
		queue_dequeue(*queue, &data);
	}
	// Try dequeueing when empty, should return -1
	assert(queue_dequeue(*queue, &data) == -1);;
}

void test_create(queue_t *queue) {
	*queue = queue_create();
	// make sure malloc didn't fail
	assert(*queue != NULL);
}

void test_enqueue(queue_t *queue, int items[]) {
	// Return -1 if queue or data are NULL
	assert(queue_enqueue(NULL, (void*)5) == -1);
	assert(queue_enqueue(*queue, NULL) == -1);
	for (int i = 0; i < 5; i++) {
		// Make sure malloc doesn't fail
		assert(queue_enqueue(*queue, &items[i]) != -1);        
	}
}

// Takes queue and correct length
void test_length(queue_t *queue, int length) {
	assert(queue_length(*queue) == length);	
}

void test_destroy(queue_t *queue) {
	void *data;
	// Null pointer -- queue doesn't exist
	assert(queue_destroy(NULL) == -1);      
	// Enqueue one item
	queue_enqueue(*queue, (void*)1);    
	test_length(queue, 1);
	// Queue has items in it
	assert(queue_destroy(*queue) == -1);
	// Dequeue last element
	queue_dequeue(*queue, &data);
	
	// Verify length is 0
	test_length(queue, 0);
	// Destroy
	assert(queue_destroy(*queue) == 0);
}

void test_delete(queue_t *queue, void *data) {
	// Return -1 if queue or data are NULL
	assert(queue_delete(NULL, (void*)5) == -1);
	assert(queue_delete(*queue, NULL) == -1);
	// Or if data not found
	assert(queue_delete(*queue, (void*)1) == -1);
	// Find and delete the number 3
	queue_delete(*queue, data);
	// Queue length should be down to 4 after deleting the number 3
	assert(queue_length(*queue) == 4);
}

int main(void) {
	// Array of items to enqueue
	int test[] = {1,2,3,4,5};
	// Declare test queue
	queue_t q;
	// Pass it to create() to test initializing it
	test_create(&q);
	// Make sure its length is listed as 0 (no items yet)
	test_length(&q, 0);
	// Test error cases and enqueueing numbers 1 through 5
	test_enqueue(&q, test);
	// Length should now be 5
	test_length(&q, 5);
	// Delete the number 3 from the queue
	test_delete(&q, &test[2]);
	// Dequeue next 3 elements, make sure oldest element (the number 1) is dequeued into data
	test_dequeue(&q, &test[0]);
	// Verifies errors for trying to delete with items still in queue, then dequeues last item and destroys queue
	test_destroy(&q);

	// Self contained -- creates its own new queue for iterator test
	test_iterator();
  
	printf("All tests passed!\n");
	return 0;
}
