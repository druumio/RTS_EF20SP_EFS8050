#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "semphr.h"
#include "task.h"

/*
https://exploreembedded.com/wiki/Resuming_Task_From_ISR#Downloads
https://www.renesas.com/us/en/products/gadget-renesas/reference/gr-rose/rtos-semaphore
*/

#define BUTTONPIN PE4 // D2
#define LEDPIN PB5 // D11

SemaphoreHandle_t xBinarySemaphore;

void ExternalInterrupt(void);
void taskTogleLED(void *pvParameters);

void setup(void)
{
  Serial.begin(9600);
  Serial.println("Setup Start");
  xTaskCreate(taskTogleLED,"LEDtask",128,NULL,2,NULL);
  attachInterrupt(digitalPinToInterrupt(2 /*BUTTONPIN*/), ExternalInterrupt /*Function to call when triggered*/, FALLING /*Edge to trigger to*/);
  xBinarySemaphore = xSemaphoreCreateBinary();
  Serial.println("Starting Task Scheduler");
  vTaskStartScheduler();
}

void loop(void)
{
  // Mty
}

void ExternalInterrupt(void)
{
  xSemaphoreGiveFromISR(xBinarySemaphore, pdFALSE);
}

void taskTogleLED(void *pvParameters)
{
  DDRB |= _BV(LEDPIN);
  PORTB &= _BV(LEDPIN);
  Serial.println("Starting LED-task");
  for (;;)
  {
    xSemaphoreTake(xBinarySemaphore, portMAX_DELAY);
    Serial.println("Got semaphore for alarm ");
    PORTB ^= _BV(LEDPIN);
  }
}