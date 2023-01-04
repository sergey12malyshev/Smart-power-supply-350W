#include "FreeRTOS.h"
#include "task.h"
#include "main.h"


void monitorTask(void)
{
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 20 / portTICK_PERIOD_MS; // 20 ms period TASK
  xLastWakeTime = xTaskGetTickCount();

  for(;;)
  {
  //  monitor();
  //  monitor_out_test();
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}
