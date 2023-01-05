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


static void controlStatePower(void)
{
  if (checkStatePower() != PinPowerEnableState())
  {
    sendUART_WARNING();
    osDelay(900);
  }
}

void checkTask(void)
{
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 10 / portTICK_PERIOD_MS; // 10 ms period TASK
  xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    controlStatePower();
    reset_WDT();

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}
