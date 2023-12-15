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

static uint8_t sensorCount = 0;
static struct
{
  String sensorName;
  uint8_t sensorAddress;
  uint8_t pumpAddress;
  uint16_t pumpTreshold = 300;
  uint16_t reading = 0;
} globalSensors[5];
/*
Function declarations
*/
void SoilMoistureTask(void *pvParameters);
void LightManagementTask(void *pvParameters);
void WaterControlTask(void *pvParameters);
void UserInputTask(void *pvParameters);
void ReportTask(void *pvParameters);
static void addSensor(String, uint8_t, uint8_t);
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
  // Construct fake sensors
  for (int i; i < 5; i++)
  {
    String name = "Moisture_sensor_" + i;
    addSensor(name, i, i);
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

static void addSensor(String name, uint8_t sAddress, uint8_t pAddress)
{
  globalSensors[sensorCount].sensorName = name;
  globalSensors[sensorCount].sensorAddress = sAddress;
  globalSensors[sensorCount++].pumpAddress = pAddress;
}


void SoilMoistureTask(void *pvParameters)
{
  /*
  For simulation purposes it's assumed that five capasitive soil-moisture sensors
  with inverse relation to moisture are used.
  In previous study it was determined that the sensor used has reading of 250mV
  when fully submerged in water and reading of 500mV when fully dry in open air.
  From there tresholds for 0% 500mV and 100% soil-moisture levels were determined.
  0% 500mV
  100% 250mV
  Sensor reading is mapped between those values and displayed to user in %.
  Direct measurement values are used within the program.

  Each sensor is read every /TBD/ minutes and values compared to tresholds
  set for the individual sensor.
  Different plants enjoy different environments and need more/less water.
  If treshold is reached /TBD cross thread com method/ is sent and
  pump related to sensor is set to be activated.

  Measures and keeps track of each sensors reading.

  Setup for this task
  */

  for (;;)
  {
    /*
    Running tasks
    */
  }
}

void LightManagementTask(void *pvParameters)
{
  /*
  Light management is divided in two different parts:
  Sceduled: User defined
  or
  Real time:
    Night time: All lights off
    Day time: Light level is measured and amount of lights is adjusted to.

  Setup for this task
  */

  for (;;)
  {
    /*
    Running tasks
    */
  }
}

void WaterControlTask(void *pvParameters)
{
  /*
  There are five pumps, each assosiated with one of the moisture-sensors.
  Pumps work only /USER DEFINED/ amout of times during day, they never work during night and
  run for /TBD amount of time/.

  Keeps track of when to pump water from each pump

  Setup for this task
  */

  for (;;)
  {
    /*
    Running tasks
    */
  }
}

void UserInputTask(void *pvParameters)
{
  /*
  Task for setting user defined settings in program.

  IF input:
    switch(input):
      case():
        test
      case():
        test
      case():
        test
      case():
        test
    if change case called changed something important
      run POR -sequence

  Setup for this task
  */

  for (;;)
  {
    /*
    Running tasks
    */
  }
}

void ReportTask(void *pvParameters)
{
  /*
  Print current status of the program and its variables for user on set interval
  Will not run while UserInputTask is being actively used.

  Setup for this task
  */

  for (;;)
  {
    /*
    Running tasks
    */
  }
}