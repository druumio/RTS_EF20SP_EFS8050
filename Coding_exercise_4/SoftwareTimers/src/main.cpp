#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <timers.h>
#include <task.h>
#include <semphr.h>

/*Define LED pin here*/
#define LEDPIN PB5 // 11
/*Macros for Serial.print()*/
#define write(msg) ThreadSafePrintMessage(msg, 0)
#define writeLine(msg) ThreadSafePrintMessage(msg, 1)

/*
https://microcontrollerslab.com/freertos-create-software-timers-with-arduino/
*/

/*Define ms period in ticks for both timers*/
#define TIMER_1_PERIOD pdMS_TO_TICKS(250)
#define TIMER_2_PERIOD pdMS_TO_TICKS(500)
/*Reference handles for both timers*/
TimerHandle_t xTimer1, xTimer2;
BaseType_t xTimer1Started, xTimer2Started;
/*To make sure only one task is accesing Serial at a time*/
SemaphoreHandle_t xSerialSemaphore;

// put function declarations here:
static void Timer1Callback(TimerHandle_t xTimer);
static void Timer2Callback(TimerHandle_t xTimer);
void ThreadSafePrintMessage(String msg, uint8_t line);

void setup()
{
  /*Setup LED*/
  DDRB |= _BV(LEDPIN);
  /*Start Serial and related Semaphore*/
  Serial.begin(9600);
  if (xSerialSemaphore == NULL) // Check to confirm that the Serial Semaphore has not already been created.
  {
    xSerialSemaphore = xSemaphoreCreateMutex(); // Create a mutex semaphore we will use to manage the Serial Port
    if ((xSerialSemaphore) != NULL)
      xSemaphoreGive((xSerialSemaphore)); // Make the Serial Port available for use, by "Giving" the Semaphore.
  }
  /*Create timer 1 with 250ms period*/
  xTimer1 = xTimerCreate(
      "250msTimer",   /*Txt name for timer, only for human use*/
      TIMER_1_PERIOD, /*Software timer period in ticks*/
      pdTRUE,         /*Auto-reload timer*/
      0,              /*No timer ID*/
      Timer1Callback /*The callback function for software timer*/);
  /*Create timer 2 with 500ms period*/
  xTimer2 = xTimerCreate(
      "500msTimer",   /*Txt name for timer, only for human use*/
      TIMER_2_PERIOD, /*Software timer period in ticks*/
      pdTRUE,         /*Auto-reload timer*/
      0,              /*No timer ID*/
      Timer2Callback /*The callback function for software timer*/);
  /* Check the software timers were created. */
  if ((xTimer1 != NULL) && (xTimer2 != NULL))
  {
    /* Start the software timers, using a block time of 0 (no block time). The scheduler has
    not been started yet so any block time specified here would be ignored anyway. */
    xTimer1Started = xTimerStart(xTimer1, 0);
    xTimer2Started = xTimerStart(xTimer2, 0);
    /* The implementation of xTimerStart() uses the timer command queue, and xTimerStart()
    will fail if the timer command queue gets full. The timer service task does not get
    created until the scheduler is started, so all commands sent to the command queue will
    stay in the queue until after the scheduler has been started. Check both calls to
    xTimerStart() passed. */
    if ((xTimer1Started == pdPASS) && (xTimer2Started == pdPASS))
    {
      /* Start the scheduler. */
      vTaskStartScheduler();
    }
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
}

static void Timer1Callback(TimerHandle_t xTimer)
{
  /*Current tick count*/
  TickType_t xTimeNow;
  xTimeNow = xTaskGetTickCount();
  /*Change LED state and print time on serial*/
  PORTB ^= _BV(LEDPIN); /*XOR led pin to change between high and low everytime timer is triggered*/
  write("LedTimer, time: ");
  writeLine(String(xTimeNow / 31));
}

static void Timer2Callback(TimerHandle_t xTimer)
{
  /*Current tick count*/
  TickType_t xTimeNow;
  xTimeNow = xTaskGetTickCount();
  /*This is the longer period timer, that print out message in serial*/
  write("Timer 2, time: ");
  writeLine(String(xTimeNow / 31));
}

void ThreadSafePrintMessage(String msg, uint8_t line)
{
  if (xSemaphoreTake(xSerialSemaphore, (TickType_t)5) == pdTRUE)
  {
    // We were able to obtain or "Take" the semaphore and can now access the shared resource.
    // We want to have the Serial Port for us alone, as it takes some time to print,
    // so we don't want it getting stolen during the middle of a conversion.

    if (line == 1)
    {
      Serial.println(msg);
    }
    else
    {
      Serial.print(msg);
    }

    xSemaphoreGive(xSerialSemaphore); // Now free or "Give" the Serial Port for others.
  }
}