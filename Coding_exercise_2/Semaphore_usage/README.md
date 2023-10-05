# RTS_EF20SP_EFS8050
Real Time Systems - EF20SP 4_EFS8050
Coding execise 2

Semaphores are commonly used in FreeRTOS to protect resources (mutex) or signal between tasks (binary or counting semaphore).

## Task:

- Create two tasks: TaskProducer and TaskConsumer.
- The TaskProducer will "produce" or generate a value (like incrementing a counter) every second and then give a semaphore.
- The TaskConsumer waits for the semaphore. Once it gets the semaphore, it "consumes" or displays the value.