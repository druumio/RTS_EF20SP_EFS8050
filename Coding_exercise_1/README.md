# RTS_EF20SP_EFS8050
Real Time Systems - EF20SP 4_EFS8050  
Coding execise 1  
  
## Task Creation:  
a. Create three tasks: TaskA, TaskB, and TaskC.  
  
b. For each task, you have the option to:  
  
### Arduino Mega 2560:   
Toggle a different LED on the Arduino Mega board (e.g., LEDs connected to pins 12, 13, and 14 respectively) and keep it ON for a second. Ensure the LED pins are set as OUTPUT using pinMode().  
**OR** 
print a message to the serial console using Serial.println(), like "Task A is running". Initialize the Serial communication using Serial.begin(9600) in your setup function.  
FreeRTOS Windows port:  
Print a message to the console indicating which task is running, e.g., "Task A is running".  
c. Assign each task a different priority.  
  
## Task Scheduling:  
a. Start the FreeRTOS scheduler.  
  
b. As a group, observe and discuss the order in which the tasks execute, through LED indications or console messages, based on their priorities.  
  
## Task Delay:  
a. Introduce a delay in one of the tasks using the vTaskDelay() function.  
  
b. Discuss within your group how this affects the scheduling and execution of tasks.  