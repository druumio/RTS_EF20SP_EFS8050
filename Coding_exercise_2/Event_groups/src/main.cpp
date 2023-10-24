#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <event_groups.h>

#define TASK_BIT_1 (1 << 0)
#define TASK_BIT_2 (1 << 1)

/**
 * Event Groups.
 * https://esp32tutorials.com/esp32-freertos-event-groups-esp-idf/
 * https://embeddedexplorer.com/freertos-event-group-tutorial-with-arduino/
 * http://www.iotsharing.com/2017/06/how-to-use-event-group-synchronizing-multiple-tasks-broadcasting-events.html
 */

// Global variables
EventGroupHandle_t event_group;

// put function declarations here:
void TaskEventSetter5s(void *pvParameters);
void TaskEventSetter25s(void *pvParameters);
void TaskEventListener(void *pvParameters);

void setup()
{
  // put your setup code here, to run once:
  event_group = xEventGroupCreate();

  if (event_group != NULL)
  {

    // Create task that Sets event bit
    xTaskCreate(TaskEventSetter5s, /*Task function*/
                "5seconds",          /*A name just for humans*/
                128,               /*This stack size can be checked & adjusted by reading the Stack Highwater*/
                NULL,
                2, /*Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.*/
                NULL);

    // Create task that Sets event bit
    xTaskCreate(TaskEventSetter25s, /*Task function*/
                "2_5seconds",           /*A name just for humans*/
                128,                /*This stack size can be checked & adjusted by reading the Stack Highwater*/
                NULL,
                2, /*Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.*/
                NULL);

    // Create task that publish data in the queue if it was created.
    xTaskCreate(TaskEventListener, /*Task function*/
                "Listener",        /*Task name*/
                128,               /*Stack size*/
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
void TaskEventSetter5s(void *pvParameters)
{
  (void)pvParameters;

  int value = 0;

  for (;;)
  {
    xEventGroupSetBits(event_group, TASK_BIT_1);
    value++;
    vTaskDelay(5000);
  }
}

void TaskEventSetter25s(void *pvParameters)
{
  (void)pvParameters;

  int value = 0;

  for (;;)
  {
    xEventGroupSetBits(event_group, TASK_BIT_2);
    value++;
    vTaskDelay(2500);
  }
}

void TaskEventListener(void *pvParameters)
{
  (void)pvParameters;

  // Init Arduino serial
  Serial.begin(9600);

  // Wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  while (!Serial)
  {
    vTaskDelay(1);
  }

  const EventBits_t xBitsToWaitFor = (TASK_BIT_1 | TASK_BIT_2);
  EventBits_t xEventGroupValue;

  for (;;)
  {

    xEventGroupValue = xEventGroupWaitBits(event_group,
                                           xBitsToWaitFor,
                                           pdTRUE,
                                           pdTRUE,
                                           portMAX_DELAY);
    if ((xEventGroupValue & TASK_BIT_1) != 0)
    {
      Serial.println("Task1 event occured");
    }
    if ((xEventGroupValue & TASK_BIT_2 != 0))
    {
      Serial.println("Task2 event occured");
    }
  }
}