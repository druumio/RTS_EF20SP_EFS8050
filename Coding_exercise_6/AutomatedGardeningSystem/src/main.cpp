/*
Libraries
*/
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "semphr.h"
#include "task.h"
#include <queue.h>
#include <event_groups.h>
#include <queue.h>
#include <timers.h>
/*
Definitions
*/
#define LEDPIN PB5 // 11
/*
Globals
*/
SemaphoreHandle_t xSerialSemaphore;
/*
Function declarations
*/
void SoilMoistureTask(void *pvParameters);
void LightManagementTask(void *pvParameters);
void WaterControlTask(void *pvParameters);
void SensorSimulationTask(void *pvParameters);
void UserInputTask(void *pvParameters);
void ReportTask(void *pvParameters);
void setup(void);
void loop(void);
/*
Function Definitions
*/
void setup(void)
{
  // Setup Serial and related semaphore
  Serial.begin(9600);
  Serial.println("Setup Start");
  if (xSerialSemaphore == NULL)
  {
    xSerialSemaphore = xSemaphoreCreateMutex();
    if ((xSerialSemaphore) != NULL)
      xSemaphoreGive((xSerialSemaphore));
  }
  /*

  Something

  */
  Serial.println("Starting Task Scheduler");
  vTaskStartScheduler();
}

void loop(void)
{
  // Nothing to see here
}

void SoilMoistureTask(void *pvParameters)
{
  /*
  Setup for this task
  */
  for (;;)
  {
    /*
    Running tasks
    */
  }
}