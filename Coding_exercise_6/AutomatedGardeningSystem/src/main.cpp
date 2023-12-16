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
#define TASKBIT_MOISTURE_READ (1UL << 0UL)
#define TASKBIT_LIGHT_READ (1UL << 1UL)
#define PUMP1 (1UL << 0UL)
#define PUMP2 (1UL << 1UL)
#define PUMP3 (1UL << 2UL)
#define PUMP4 (1UL << 3UL)
#define PUMP5 (1UL << 4UL)
#define LEDPIN3 PB6 // 12
#define LEDPIN2 PB5 // 11
#define LEDPIN1 PB4 // 10
#define SOILMOISTURETASK_DELAY 100
#define LIGHTMANAGETASK_DELAY 1000
/*
Macros
*/
/*Macros for Serial.print()*/
#define write(msg) ThreadSafePrintMessage(msg, 0)
#define writeLine(msg) ThreadSafePrintMessage(msg, 1)

/*
Globals
*/
SemaphoreHandle_t xSerialSemaphore, xSensorsSemaphore, xTimeSemaphore;
EventGroupHandle_t xEventGroup, xPumpGroup;
QueueHandle_t xQueue;
TaskHandle_t MoistureTaskHandle;

enum currentTimeofDay
{
  night,
  day,
  hours,
  minutes,
  seconds
};

volatile uint8_t day_night = day;

static uint8_t sensorCount = 0;
static struct
{
  String sensorName;
  uint8_t sensorAddress;
  uint8_t pumpAddress;
  uint16_t pumpTreshold = 450;
  uint16_t reading = 0;
} globalSensors[5];

struct timeStruct
{
  uint8_t hour = 6;
  uint8_t min = 0;
  uint8_t sec = 0;
};

volatile timeStruct currentTime;

/*
Function declarations
*/
void SoilMoistureTask(void *pvParameters);
void LightManagementTask(void *pvParameters);
void WaterControlTask(void *pvParameters);
void UserInputTask(void *pvParameters);
void ReportTask(void *pvParameters);
void pumpTask(void *pvParameters);
void updateTime(uint8_t, uint8_t);
void setTime(uint8_t, uint8_t);
static void addSensor(String, uint8_t, uint8_t);
void ThreadSafePrintMessage(String, uint8_t);
uint16_t readSensor(uint8_t);
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

  // MUTEX for serial
  if (xSerialSemaphore == NULL)
  {
    xSerialSemaphore = xSemaphoreCreateMutex();
    if ((xSerialSemaphore) != NULL)
    {
      xSemaphoreGive((xSerialSemaphore));
    }
    else
    {
      Serial.println("Failed to create xSerialSemaphore");
    }
  }
  // MUTEX for Sensors
  if (xSensorsSemaphore == NULL)
  {
    xSensorsSemaphore = xSemaphoreCreateMutex();
    if ((xSensorsSemaphore) != NULL)
    {
      xSemaphoreGive((xSensorsSemaphore));
    }
    else
    {
      Serial.println("Failed to create xSensorsSemaphore");
    }
  }
  // MUTEX for Time
  if (xTimeSemaphore == NULL)
  {
    xTimeSemaphore = xSemaphoreCreateMutex();
    if ((xTimeSemaphore) != NULL)
    {
      xSemaphoreGive((xTimeSemaphore));
    }
    else
    {
      Serial.println("Failed to create xTimeSemaphore");
    }
  }
  // Create eventgroup for handling timed tasks
  xEventGroup = xEventGroupCreate();
  // Create eventgroup to handle which pump to operate
  xPumpGroup = xEventGroupCreate();

  xQueue = xQueueCreate(5, sizeof(int16_t));
  ///@todo create tasks!
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
  if (xSemaphoreTake(xSensorsSemaphore, portMAX_DELAY) == pdTRUE)
  {
    // Construct fake sensors
    for (int i; i < 5; i++)
    {
      String name = "Moisture_sensor_" + i;
      addSensor(name, i, i);
    }
    writeLine("Sensors created!");
    xSemaphoreGive(xSensorsSemaphore);
  }

  for (;;)
  {
    /*
    Running tasks
    */
    xEventGroupSetBits(xEventGroup, TASKBIT_MOISTURE_READ);
    vTaskDelay(SOILMOISTURETASK_DELAY / portTICK_PERIOD_MS);
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
  int16_t light_level;
  DDRB |= (_BV(LEDPIN1) | _BV(LEDPIN2) | _BV(LEDPIN3));  // LEDPINs output
  PORTB &= (_BV(LEDPIN1) | _BV(LEDPIN2) | _BV(LEDPIN3)); // Turn LEDs off
  writeLine("LED setup Done!");
  for (;;)
  {
    /*
    Running tasks
    */
    switch (day_night)
    {
    case day:
      /*
      Monitor light level live and adjust the amount of light given by LEDs
      */
      // Start Light mesuring in the event tast
      xEventGroupSetBits(xEventGroup, TASKBIT_LIGHT_READ);
      // Wait for result from queue
      if (xQueueReceive(xQueue, &light_level, portMAX_DELAY) == pdPASS)
      {
        if (light_level == -1)
        {
          writeLine("Light level reading failed!");
        }
        else if (light_level < 20)
        {
          // Turn on all LEDs for low light levels
          PORTB |= (_BV(LEDPIN1) | _BV(LEDPIN2) | _BV(LEDPIN3));
        }
        else if (light_level < 60)
        {
          // Turn on first two LEDs for medium light levels
          PORTB |= (_BV(LEDPIN1) | _BV(LEDPIN2));
        }
        else if (light_level < 100)
        {
          // Turn on the first LED for high light levels
          PORTB |= _BV(LEDPIN1);
        }
        else
        {
          // Turn off all LEDs for very high light levels
          PORTB &= (_BV(LEDPIN1) | _BV(LEDPIN2) | _BV(LEDPIN3)); // Turn LEDs off
        }
      }
      break;

    case night:
      /*
      Lights off during night
      Night-time 18:00 - 6:00
      */
      PORTB &= (_BV(LEDPIN1) | _BV(LEDPIN2) | _BV(LEDPIN3)); // Turn LEDs off
      break;

    default:
      break;
    }
    vTaskDelay(LIGHTMANAGETASK_DELAY / portTICK_PERIOD_MS);
  }
}

void WaterControlTask(void *pvParameters)
{
  /*
  There are five pumps, each assosiated with one of the moisture-sensors.
  Pumps work only 4 times during day, they never work during night and
  run for /TBD amount of time/.

  Keeps track of when to pump water from each pump

  Setup for this task
  */
  const EventBits_t xBitsToWaitFor = (PUMP1 | PUMP2 | PUMP3 | PUMP4 | PUMP5);
  EventBits_t xEventGroupValue;
  int *pumpNum;
  for (;;)
  {
    /*
    Running tasks
    */

    // Wait for any of the specified pump bits to be set in the event group.
    xEventGroupValue = xEventGroupWaitBits(xPumpGroup,
                                           xBitsToWaitFor,
                                           pdTRUE,         // Clear the bits in the event group on exit.
                                           pdTRUE,         // Wait for all specified bits to be set.
                                           portMAX_DELAY); // Block indefinitely until the bits are set.

    // Check which pump bits are set and create a corresponding pump task.
    if ((xEventGroupValue & PUMP1) != 0)
    {
      int pumpValue = 0;
      pumpNum = &pumpValue;
      xTaskCreate(pumpTask, "PumpTask_0", 2048, (void *)pumpNum, 1, NULL);
    }
    if ((xEventGroupValue & PUMP2) != 0)
    {
      int pumpValue = 1;
      pumpNum = &pumpValue;
      xTaskCreate(pumpTask, "PumpTask_1", 2048, (void *)pumpNum, 1, NULL);
    }
    if ((xEventGroupValue & PUMP3) != 0)
    {
      int pumpValue = 2;
      pumpNum = &pumpValue;
      xTaskCreate(pumpTask, "PumpTask_2", 2048, (void *)pumpNum, 1, NULL);
    }
    if ((xEventGroupValue & PUMP4) != 0)
    {
      int pumpValue = 3;
      pumpNum = &pumpValue;
      xTaskCreate(pumpTask, "PumpTask_3", 2048, (void *)pumpNum, 1, NULL);
    }
    if ((xEventGroupValue & PUMP5) != 0)
    {
      int pumpValue = 4;
      pumpNum = &pumpValue;
      xTaskCreate(pumpTask, "PumpTask_4", 2048, (void *)pumpNum, 1, NULL);
    }
  }
}

void MainEventTask(void *pvParameters)
{
  const EventBits_t xBitsToWaitFor = (TASKBIT_MOISTURE_READ | TASKBIT_LIGHT_READ);
  EventBits_t xEventGroupValue;
  uint16_t localTreshold, localReading;
  for (;;)
  {
    xEventGroupValue = xEventGroupWaitBits(xEventGroup,
                                           xBitsToWaitFor,
                                           pdTRUE,         // Clear the bits in the event group on exit.
                                           pdTRUE,         // Wait for all specified bits to be set.
                                           portMAX_DELAY); // Block indefinitely until the bits are set.
    if ((xEventGroupValue & TASKBIT_MOISTURE_READ) != 0)
    {
      writeLine("Soil Moisture Event Flag received\nSuspending SoilMoistureTask");
      vTaskSuspend(MoistureTaskHandle);
      if (xSemaphoreTake(xSensorsSemaphore, portMAX_DELAY) == pdTRUE)
      {
        // Reading All sensors
        for (int i; i < 5; i++)
        {
          localTreshold = globalSensors[i].pumpTreshold;
          localReading = readSensor(globalSensors[i].sensorAddress);
          if (localReading > 0) // If reading sensor was succesfull. -1 == ERROR
          {
            globalSensors[i].reading = localReading;
            if (localReading > localTreshold)
            {
              xEventGroupSetBits(xPumpGroup, (1UL << i));
            }
          }
          else
          {
            write("Error while reading sensor number: ");
            String str = (String)i;
            writeLine(str);
          }
        }
        writeLine("Reading Sensors Done resuming SoilMoistureTask");
        xSemaphoreGive(xSensorsSemaphore);
        vTaskResume(MoistureTaskHandle);
      }
    }
    if ((xEventGroupValue & TASKBIT_LIGHT_READ) != 0)
    {
      ///@todo
    }
  }
}

///@todo
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

///@todo
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

/// @brief Task function to control a pump.
/// @param pvParameters A pointer to the pump number (integer) passed as a task parameter.
void pumpTask(void *pvParameters)
{
  /*
  Start and Stop pump defined by param

  Setup for this task
  */
  // Extract the pump number from the task parameters.
  int *local_pumpNum;
  local_pumpNum = (int *)pvParameters;
  // Generate a message string indicating the pump number.
  String msg = "Pump_" + String(*local_pumpNum) + " ";

  /*
  Running task
  */
  // Perform pump start operations.
  write(msg);
  writeLine("Start");
  // Delay for running the pump
  vTaskDelay(30 / portTICK_PERIOD_MS);
  // Perform pump stop operations.
  write(msg);
  writeLine("Stop. Deleting task");
  // Delete the task. This is typically done to self-terminate the task.
  vTaskDelete(NULL);
}

/// @brief Fake sensor responce
/// @param address Address of the sensor being read
/// @return random value between 250 and 500
uint16_t readSensor(uint8_t address)
{
  int reading = random(250, 500);
  return reading;
}

/// @brief Update the time based on the specified part (hours, minutes, or seconds).
/// @param part The time part to update (hours, minutes, or seconds).
/// @param amount The amount by which is added to the specified time part.
void updateTime(uint8_t part, uint8_t amount)
{
  if (xSemaphoreTake(xTimeSemaphore, portMAX_DELAY) == pdTRUE)
  {
    if (part == hours)
    {
      uint8_t h = currentTime.hour + amount;
      if (h >= 24)
      {
        h = h - 24;
      }
      if (day_night == day && (h >= 18 || h < 6))
      {
        day_night = night;
      }
      else if (day_night == night && (h >= 6 && h < 18))
      {
        day_night = day;
      }

      currentTime.hour = h;
      xSemaphoreGive(xTimeSemaphore);
    }
    else if (part == minutes)
    {
      uint8_t m = currentTime.min + amount;
      if (m >= 60)
      {
        currentTime.min = m - 60;
        xSemaphoreGive(xTimeSemaphore);
        updateTime(hours, (uint8_t)1);
      }
      else
      {
        currentTime.hour = m;
        xSemaphoreGive(xTimeSemaphore);
      }
    }
    else if (part == seconds)
    {
      uint8_t s = currentTime.min + amount;
      if (s >= 60)
      {
        currentTime.sec = s - 60;
        xSemaphoreGive(xTimeSemaphore);
        updateTime(minutes, (uint8_t)1);
      }
      else
      {
        currentTime.hour = s;
        xSemaphoreGive(xTimeSemaphore);
      }
    }
    else
    {
      /// @note This case is reached when the specified 'part' is invalid.
      xSemaphoreGive(xTimeSemaphore);
      writeLine("Time update failed!");
    }
  }
}

/// @brief Set the time based on the specified part (hours, minutes, or saeconds).
/// @param part The time part to update (hours, minutes, or seconds).
/// @param amount The amount by which the specified time part is set to.
void setTime(uint8_t part, uint8_t amount)
{
  if (xSemaphoreTake(xTimeSemaphore, portMAX_DELAY) == pdTRUE)
  {
    if (part == hours)
    {
      currentTime.hour = amount;
    }
    else if (part == minutes)
    {
      currentTime.min = amount;
    }
    else if (part == seconds)
    {
      currentTime.sec = amount;
    }
    else
    {
      /// @note This case is reached when the specified 'part' is invalid.
      writeLine("Time update failed!");
    }
  }
  xSemaphoreGive(xTimeSemaphore);
}

/// @brief Adds a sensor to the global sensor array.
/// @param name The name of the sensor.
/// @param sAddress The sensor address.
/// @param pAddress The pump address associated with the sensor.
static void addSensor(String name, uint8_t sAddress, uint8_t pAddress)
{
  // Assign the provided values to the current sensor in the global array.
  globalSensors[sensorCount].sensorName = name;
  globalSensors[sensorCount].sensorAddress = sAddress;
  globalSensors[sensorCount++].pumpAddress = pAddress;
}

/// @brief Prints a message to the Serial monitor in a thread-safe manner.
/// @param msg The message to be printed.
/// @param line The line number on the Serial monitor (1 for println, 0 for print).
void ThreadSafePrintMessage(String msg, uint8_t line)
{
  // Attempt to take the serial access semaphore with a timeout of 5 ticks.
  if (xSemaphoreTake(xSerialSemaphore, (TickType_t)5) == pdTRUE)
  {
    // Successfully acquired the semaphore, safe to print to Serial.
    if (line == 1)
    {
      // Print the message followed by a newline.
      Serial.println(msg);
    }
    else
    {
      // Print the message without a newline.
      Serial.print(msg);
    }
    // Release the serial access semaphore.
    xSemaphoreGive(xSerialSemaphore);
  }
  // Unable to acquire the semaphore within the timeout.
  /// @todo create retry mechanism
}
