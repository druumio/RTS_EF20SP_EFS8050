#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "semphr.h"
#include "task.h"

/*
https://exploreembedded.com/wiki/Resuming_Task_From_ISR#Downloads
https://www.renesas.com/us/en/products/gadget-renesas/reference/gr-rose/rtos-semaphore
https://www.arduino.cc/en/Tutorial/BuiltInExamples/Button
*/

#define BUTTONPIN PE4 // D2
#define LEDPIN PB5    // D11
#define DEBOUNCING_TIME (uint32_t)150

SemaphoreHandle_t xBinarySemaphore;
volatile uint32_t last_interrupt;

void ExternalInterrupt(void);
void taskTogleLED(void *pvParameters);

void setup(void)
{
  Serial.begin(9600);
  Serial.println("Setup Start");
  xTaskCreate(taskTogleLED, "LEDtask", 128, NULL, 2, NULL);
  attachInterrupt(digitalPinToInterrupt(2 /*BUTTONPIN*/), ExternalInterrupt /*Function to call when triggered*/, FALLING /*Edge to trigger to*/);
  xBinarySemaphore = xSemaphoreCreateBinary();
  Serial.println("Starting Task Scheduler");
  vTaskStartScheduler();
}

void loop(void)
{
  // Actually this is FreeRTOS Idle task
}

// Function to be called when ISR trigger
void ExternalInterrupt(void)
{
  //Debounce interrupt button
  if ((uint32_t)(micros() - last_interrupt) >= (DEBOUNCING_TIME * 1000))
  {
    xSemaphoreGiveFromISR(xBinarySemaphore, pdFALSE);
    last_interrupt = micros();
  }
}

void taskTogleLED(void *pvParameters)
{
  DDRB |= _BV(LEDPIN);  // LEDPIN output
  PORTB &= _BV(LEDPIN); // Turn LED off
  Serial.println("Starting LED-task");
  for (;;)
  {
    xSemaphoreTake(xBinarySemaphore, portMAX_DELAY); // Wait for semaphore
    Serial.println("Got semaphore for alarm ");
    PORTB ^= _BV(LEDPIN); // Switch LED state on/off
  }
}