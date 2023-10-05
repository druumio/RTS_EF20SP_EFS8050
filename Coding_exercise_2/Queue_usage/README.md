# RTS_EF20SP_EFS8050
Real Time Systems - EF20SP 4_EFS8050
Coding execise 2

Queues in FreeRTOS allow tasks to send messages between them.

## Task:

- Create two tasks: TaskSender and TaskReceiver.
- The TaskSender sends a message (a value or structure) to a queue every few seconds.
- The TaskReceiver waits for a message on the queue. When it receives a message, it processes (or displays) it.