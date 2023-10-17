#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <queue.h>

// Global variables
QueueHandle_t integerQueue;

// put function declarations here:
void TaskSender(void *pvParameters);
void TaskReceiver(void *pvParameters);

void setup()
{
  // put your setup code here, to run once:

  /**
   * Create a queue.
   * https://www.freertos.org/a00116.html
   */
  integerQueue = xQueueCreate(10, /*Queue length*/
                              sizeof(int) /*Queue item size*/);

  if (integerQueue != NULL)
  {

    // Create task that consumes the queue if it was created.
    xTaskCreate(TaskSender, /*Task function*/
                "Sender",   /*A name just for humans*/
                128,        /*This stack size can be checked & adjusted by reading the Stack Highwater*/
                NULL,
                2, /*Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.*/
                NULL);

    // Create task that publish data in the queue if it was created.
    xTaskCreate(TaskReceiver, /*Task function*/
                "Receiver",   /*Task name*/
                128,          /*Stack size*/
                NULL,
                1, /*Priority*/
                NULL);
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
  // Empty!
}

// put function definitions here:
void TaskSender(void *pvParameters)
{
  (void)pvParameters;

  int value = 0;

  for (;;)
  {
    xQueueSend(integerQueue, &value, portMAX_DELAY);
    value++;
    vTaskDelay(1000);
  }
}

void TaskReceiver(void *pvParameters)
{
  (void)pvParameters;

  // Init Arduino serial
  Serial.begin(9600);

  // Wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  while (!Serial) {
    vTaskDelay(1);
  }

  int valueFromQueue = 0;

  for (;;) 
  {

    /**
     * Read an item from a queue.
     * https://www.freertos.org/a00118.html
     */
    if (xQueueReceive(integerQueue, &valueFromQueue, portMAX_DELAY) == pdPASS) {
      Serial.println(valueFromQueue);
    }
  }
}