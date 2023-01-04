#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "hardware.h"
#include "mainTask.h"

uint32_t task10msCnt = 0;

void mainTask(void)
{
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 10 / portTICK_PERIOD_MS; // 10 ms period TASK
  xLastWakeTime = xTaskGetTickCount();
  
  for(;;)
  {
	task10msCnt++;
	reset_WDT();
	adc1_convertion();
	adc2_convertion();
		
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

