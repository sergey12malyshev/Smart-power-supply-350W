#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "task.h"
#include "main.h"
#include "hardware.h"
#include "checkTask.h"
#include "monitorTask.h"
#include "workState.h"

extern uint16_t voltage;
extern uint16_t current;

bool overVoltageState = false;
bool overCurrentState = false;

static void controlStatePower(void)
{
  if (checkStatePower() != PinPowerEnableState())
  {
    setState(ALARM_STATE);
    sendUART_WARNING();
    osDelay(1200);
  }
  else if (PinPowerEnableState())
  {
    setState(SWITCH_ON_STATE);
  }
  else
  {
    setState(SWITCH_OFF_STATE);
  }
}

static void overVoltageControl(void)
{
  const uint16_t maxVoltage = 3300; // 33 В
  if (voltage > maxVoltage)
  {
    if(!overVoltageState) sendUART("Over Voltage!\r\n");
    overVoltageState = true;
  }
  else
  {
    if(overVoltageState) sendUART("Over Voltage end\r\n");
    overVoltageState = false;
  }

}

static void overCurrentControl(void)
{
  const uint16_t maxCurrent = 16000; // 16 А
  if (current > maxCurrent)
  {
    if(!overCurrentState) sendUART("Over Current!\r\n");
    overCurrentState = true;
  }
  else
  {
    if(overCurrentState) sendUART("Over Current end\r\n");
    overCurrentState = false;
  }
}

/*
* Задача безопасности и контроля
*/
void checkTask(void)
{
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 10 / portTICK_PERIOD_MS; // 10 ms period TASK
  xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    controlStatePower();
    overVoltageControl();
    overCurrentControl();
    reset_WDT();

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}
