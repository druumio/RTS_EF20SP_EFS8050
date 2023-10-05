# RTS_EF20SP_EFS8050
Real Time Systems - EF20SP 4_EFS8050
Coding execise 2

Event groups allow tasks to wait for one or more bits in a group to be set.

## Task:

- Create a simple event simulation. For instance, one task (TaskEventSetter) sets an event bit upon some condition (e.g., after every 5 seconds).
- Another task (TaskEventListener) waits for the event bit to be set and then performs some action (e.g., toggling an LED or printing a message).