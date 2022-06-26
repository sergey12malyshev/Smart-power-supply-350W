#include "FreeRTOS.h"
#include "task.h"
#include "main.h"


////uint32_t task50msCnt = 0;

//void mainTask(void)
//{
//    /* USER CODE BEGIN Task10msHandler */
//    TickType_t xLastWakeTime;
//    const TickType_t xFrequency = 10 / portTICK_PERIOD_MS;
//    xLastWakeTime = xTaskGetTickCount();
//    /* Infinite loop */
//    for(;;)
//    {
//        // Add code here
////				task50msCnt++;
//        vTaskDelayUntil(&xLastWakeTime, xFrequency);
//    }
//    /* USER CODE END Task10msHandler */
//}