#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h> // add the FreeRTOS functions for Semaphores (or Flags).

#define LEDPIN1 PB5 // 11
#define LEDPIN2 PB6 // 12
#define LEDPIN3 PB7 // 13

/*
Reference:
  FreeRTOS\examples\AnalogRead_DigitalRead
  FreeRTOS\examples\Blink_AnalogRead

Improvements:
  Tasks 1-3 are identical, they could be simplified to TaskLed(LEDPINX)
*/

// Declare a mutex Semaphore Handle which we will use to manage the Serial Port.
// It will be used to ensure only one Task is accessing this resource at any time.
SemaphoreHandle_t xSerialSemaphore;

// Function declarations
void TaskLED1(void *pvParameters);
void TaskLED2(void *pvParameters);
void TaskLED3(void *pvParameters);
void printMessage(String msg);

void setup()
{
  // put your setup code here, to run once:

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  // Semaphores are useful to stop a Task proceeding, where it should be paused to wait,
  // because it is sharing a resource, such as the Serial port.
  // Semaphores should only be used whilst the scheduler is running, but we can set it up here.
  if (xSerialSemaphore == NULL) // Check to confirm that the Serial Semaphore has not already been created.
  {
    xSerialSemaphore = xSemaphoreCreateMutex(); // Create a mutex semaphore we will use to manage the Serial Port
    if ((xSerialSemaphore) != NULL)
      xSemaphoreGive((xSerialSemaphore)); // Make the Serial Port available for use, by "Giving" the Semaphore.
  }

  xTaskCreate(
      TaskLED1,
      "BlinkLED1" /*A name just for humans*/,
      128 /*This stack size can be checked & adjusted by reading the Stack Highwater*/,
      NULL,
      2 /*Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.*/,
      NULL);

  xTaskCreate(
      TaskLED2,
      "BlinkLED2" /*A name just for humans*/,
      128 /*This stack size can be checked & adjusted by reading the Stack Highwater*/,
      NULL,
      2 /*Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.*/,
      NULL);

  xTaskCreate(
      TaskLED3,
      "BlinkLED3" /*A name just for humans*/,
      128 /*This stack size can be checked & adjusted by reading the Stack Highwater*/,
      NULL,
      2 /*Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.*/,
      NULL);

  vTaskStartScheduler();
}

void loop()
{
  // put your main code here, to run repeatedly:
  // Dis empty lol
}

void TaskLED1(void *pvParameters __attribute__((unused))) // This is a Task.
{
  /*
  Task for blinking 1st led
  */

  // Setup LEDPIN1 as output
  // pinMode(LEDPIN1, OUTPUT);
  DDRB |= _BV(LEDPIN1);

  // Safe Serial.print()
  printMessage("Starting Task1");

  for (;;) // A Task shall never return or exit.
  {
    printMessage("Task1 triggered");

    // turn the LED on (HIGH is the voltage level)
    // digitalWrite(LEDPIN1, HIGH);
    PORTB |= _BV(LEDPIN1);

    vTaskDelay(1000 / portTICK_PERIOD_MS); // wait for one second

    // turn the LED off by making the voltage LOW
    // digitalWrite(LEDPIN1, LOW);
    PORTB &= _BV(LEDPIN1);

    vTaskDelay(1000 / portTICK_PERIOD_MS); // wait for one second
  }
}

void TaskLED2(void *pvParameters __attribute__((unused))) // This is a Task.
{
  /*
  Task for blinking 2st led
  */

  // Setup LEDPIN2 as output
  // pinMode(LEDPIN2, OUTPUT);
  DDRB |= _BV(LEDPIN2);

  // Safe Serial.print()
  printMessage("Starting Task2");

  for (;;) // A Task shall never return or exit.
  {
    printMessage("Task2 triggered");

    // turn the LED on (HIGH is the voltage level)
    // digitalWrite(LEDPIN2, HIGH);
    PORTB |= _BV(LEDPIN2);

    vTaskDelay(2000 / portTICK_PERIOD_MS); // wait for 2 second

    // turn the LED off by making the voltage LOW
    // digitalWrite(LEDPIN2, LOW);
    PORTB &= _BV(LEDPIN2);

    vTaskDelay(2000 / portTICK_PERIOD_MS); // wait for 2 second
  }
}

void TaskLED3(void *pvParameters __attribute__((unused))) // This is a Task.
{
  /*
  Task for blinking 3st led
  */

  // Setup LEDPIN3 as output
  // pinMode(LEDPIN3, OUTPUT);
  DDRB |= _BV(LEDPIN3);

  // Safe Serial.print()
  printMessage("Starting Task3");

  for (;;) // A Task shall never return or exit.
  {
    printMessage("Task3 triggered");

    // turn the LED on (HIGH is the voltage level)
    // digitalWrite(LEDPIN3, HIGH);
    PORTB |= _BV(LEDPIN3);

    vTaskDelay(3000 / portTICK_PERIOD_MS); // wait for 3 second

    // turn the LED off by making the voltage LOW
    // digitalWrite(LEDPIN3, LOW);
    PORTB &= _BV(LEDPIN3);

    vTaskDelay(3000 / portTICK_PERIOD_MS); // wait for 3 second
  }
}

void printMessage(String msg)
{
  if (xSemaphoreTake(xSerialSemaphore, (TickType_t)5) == pdTRUE)
  {
    // We were able to obtain or "Take" the semaphore and can now access the shared resource.
    // We want to have the Serial Port for us alone, as it takes some time to print,
    // so we don't want it getting stolen during the middle of a conversion.
    Serial.println(msg);

    xSemaphoreGive(xSerialSemaphore); // Now free or "Give" the Serial Port for others.
  }
}