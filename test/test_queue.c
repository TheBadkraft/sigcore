// test_queue.c
#include "sigtest.h"
#include "sigcore.h"

// Assert.isTrue(condition, "fail message");
// Assert.isFalse(condition, "fail message");
// Assert.areEqual(obj1, obj2, INT, "fail message");
// Assert.areEqual(obj1, obj2, PTR, "fail message");
// Assert.areEqual(obj1, obj2, STRING, "fail message");

/* test the basic interface */
void queue_interface(void) {
	fprintf(stdout, "\n");
	fflush(stdout);
	
	int expCapacity = 4;
	int expCount = 0;
	
	queue q = Queue.new(expCapacity);
	int actCapacity = Queue.capacity(q);
	int actCount = Queue.count(q);
	
	Assert.isTrue(expCapacity == actCapacity, "capacity mismatch");
	Assert.isTrue(expCount == actCount, "count mismatch");
	Assert.isTrue(Queue.isEmpty(q), "q should be empty");
	Assert.isFalse(Queue.isFull(q), "q should not be full");
	
	Queue.free(q);
}
/* test enqueue */
void queue_enqueue_item(void) {
	fprintf(stdout, "\n");
	fflush(stdout);
	
	int expCapacity = 4;
	int expCount = 1;
	
	queue q = Queue.new(expCapacity);
	Queue.enqueue(q, (object)1);
	int actCount = Queue.count(q);
	
	Assert.isTrue(expCount == actCount, "count mismatch");
	Assert.isFalse(Queue.isFull(q), "q should not be full");
	
	Queue.free(q);
}
/* test clear queue */
void queue_clear(void) {
	fprintf(stdout, "\n");
	fflush(stdout);
	
	int expCapacity = 4;
	int expCount = 0;
	
	queue q = Queue.new(expCapacity);
	Queue.enqueue(q, (object)1);
	Queue.clear(q);
	int actCapacity = Queue.capacity(q);
	int actCount = Queue.count(q);
	
	Assert.isTrue(expCapacity == actCapacity, "capacity mismatch");
	Assert.isTrue(expCount == actCount, "count mismatch");
	Assert.isTrue(Queue.isEmpty(q), "q should be empty");
	Assert.isFalse(Queue.isFull(q), "q should not be full");

	Queue.free(q);
}
/* test dequeue item */
void queue_dequeue(void) {
	fprintf(stdout, "\n");
	fflush(stdout);
	
	int expCapacity = 4;
	int expCount = 0;
	int expValue = 538;
	
	queue q = Queue.new(expCapacity);
	Queue.enqueue(q, &expValue);
	int actValue = *(int*)Queue.dequeue(q);
	int actCount = Queue.count(q);
	
	flogf(stdout, "Actual value=%d", actValue);
	flogf(stdout, "Actual count=%d", actCount);
	
	Assert.isTrue(expValue == actValue, "value mismatch");
	Assert.isTrue(expCount == actCount, "count mismatch");
	Assert.isTrue(Queue.isEmpty(q), "q should be empty");
	
	Queue.free(q);
}
/* test peek at front */
void queue_peek(void) {
	fprintf(stdout, "\n");
	fflush(stdout);
	
	int expCapacity = 4;
	int expCount = 1;
	int expValue = 5;
	
	queue q = Queue.new(expCapacity);
	Queue.enqueue(q, &expValue);
	int actValue = *(int*)Queue.peek(q);
	int actCount = Queue.count(q);
	
	flogf(stdout, "Actual value=%d", actValue);
	
	Assert.isTrue(expValue == actValue, "value mismatch");
	Assert.isTrue(expCount == actCount, "count mismatch");
	
	Queue.free(q);
}
/* test circular wrap */
void queue_circular_wrap(void) {
    fprintf(stdout, "\n");
    fflush(stdout);
    
    int expCapacity = 3;
    int expValue1 = 1, expValue2 = 2, expValue3 = 3;
    
    queue q = Queue.new(expCapacity);
    Queue.enqueue(q, &expValue1);
    Queue.enqueue(q, &expValue2);
    Assert.isTrue(Queue.isFull(q), "q should be full");  // Check before 3rd enqueue
    
    Queue.enqueue(q, &expValue3);  // Wraps last
    int actValue1 = *(int*)Queue.dequeue(q);
    Assert.isTrue(expValue1 == actValue1, "value1 mismatch");
    Assert.isFalse(Queue.isFull(q), "q should not be full");
    
    Queue.enqueue(q, &expValue3);  // Wraps again
    int actValue2 = *(int*)Queue.dequeue(q);
    Assert.isTrue(expValue2 == actValue2, "value2 mismatch");
    
    int actCount = Queue.count(q);
    Assert.isTrue(actCount == 2, "count mismatch");
    
    Queue.free(q);
}
/* test queue resize */
void queue_resize(void) {
    fprintf(stdout, "\n");
    fflush(stdout);
    
    int expCapacity = 2;
    int expValue1 = 1, expValue2 = 2, expValue3 = 3;
    
    queue q = Queue.new(expCapacity);
    Queue.enqueue(q, &expValue1);
    Assert.isTrue(Queue.isFull(q), "q should be full");  // Check before 2nd enqueue
    
    Queue.enqueue(q, &expValue2);  // Fills, then resizes
    Queue.enqueue(q, &expValue3);  // Adds after resize
    int newCapacity = Queue.capacity(q);
    Assert.isTrue(newCapacity >= 4, "capacity should double");
    Assert.isTrue(Queue.count(q) == 3, "count mismatch");
    
    int actValue1 = *(int*)Queue.dequeue(q);
    Assert.isTrue(expValue1 == actValue1, "value1 mismatch");
    
    Queue.free(q);
}
/* test full dequeue */
void queue_full_dequeue(void) {
    fprintf(stdout, "\n");
    fflush(stdout);
    
    int expCapacity = 3;
    int expValue1 = 1, expValue2 = 2, expValue3 = 3;
    
    queue q = Queue.new(expCapacity);
    Queue.enqueue(q, &expValue1);
    Queue.enqueue(q, &expValue2);
    Assert.isTrue(Queue.isFull(q), "q should be full");  // Check before 3rd enqueue
    
    Queue.enqueue(q, &expValue3);  // Wraps or resizes
    int actValue1 = *(int*)Queue.dequeue(q);
    int actValue2 = *(int*)Queue.dequeue(q);
    int actValue3 = *(int*)Queue.dequeue(q);
    
    Assert.isTrue(expValue1 == actValue1, "value1 mismatch");
    Assert.isTrue(expValue2 == actValue2, "value2 mismatch");
    Assert.isTrue(expValue3 == actValue3, "value3 mismatch");
    Assert.isTrue(Queue.isEmpty(q), "q should be empty");
    
    Queue.free(q);
}
/* test NULL handling */
void queue_null_handling(void) {
    fprintf(stdout, "\n");
    fflush(stdout);
    
    int expProperty = 0;
    int actProperty = 0;
    
    queue q = NULL;
    Assert.isTrue(Queue.isEmpty(q), "NULL queue should be empty");
    Assert.isFalse(Queue.isFull(q), "NULL queue should not be full");
    actProperty = Queue.count(q);
    Assert.areEqual(&expProperty, &actProperty, INT, "count should be 0");
    actProperty = Queue.capacity(q);
    Assert.areEqual(&expProperty, &actProperty, INT, "capacity should be 0");
    Assert.areEqual(Queue.dequeue(q), NULL, PTR, "dequeue NULL should return NULL");
    Assert.areEqual(Queue.peek(q), NULL, PTR, "peek NULL should return NULL");
    
    q = Queue.new(2);
    Queue.clear(q);
    Assert.areEqual(Queue.dequeue(q), NULL, PTR, "dequeue empty should return NULL");
    Assert.areEqual(Queue.peek(q), NULL, PTR, "peek empty should return NULL");
    
    Queue.free(q);
}
/* test queue stress */
void queue_stress(void) {
    fprintf(stdout, "\n");
    fflush(stdout);
    
    int expCapacity = 4;
    int values[] = {1, 2, 3, 4, 5, 6};
    
    queue q = Queue.new(expCapacity);
    for (int i = 0; i < 6; i++) {
        Queue.enqueue(q, &values[i]);
    }
    Assert.isTrue(Queue.capacity(q) >= 6, "capacity should resize");
    
    int i = 0;
    while (!Queue.isEmpty(q)) {
        int actValue = *(int*)Queue.dequeue(q);
        Assert.isTrue(values[i] == actValue, "value mismatch");
        
        ++i;
    }
    Assert.isTrue(Queue.isEmpty(q), "q should be empty");
    
    Queue.free(q);
}

// Register test cases
__attribute__((constructor)) void init_sigtest_tests(void) {
	register_test("queue_interface", queue_interface);
	register_test("queue_enqueue_item", queue_enqueue_item);
	register_test("queue_clear", queue_clear);
	register_test("queue_dequeue", queue_dequeue);
	register_test("queue_peek", queue_peek);
	register_test("queue_circular_wrap", queue_circular_wrap);
	register_test("queue_resize", queue_resize);
	register_test("queue_full_dequeue", queue_full_dequeue);
	register_test("queue_null_handling", queue_null_handling);
	register_test("queue_stress", queue_stress);
}
