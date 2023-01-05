#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "hardware.h"
#include "mainTask.h"

extern uint16_t zero_ad712;

uint32_t task10msCnt = 0;

volatile uint16_t adc1_value;
volatile uint16_t adc2_value;
uint16_t voltage;
uint16_t current;

void voltageConvertion(uint16_t adc_1)
{
  voltage = (adc_1 * 33 * 107) / 4096; // 9,1K 910 OHM -> 11*3=33 (36)V MAX INPUT
}

void currentConvertion(uint16_t adc_2)
{
  uint16_t volt_do_current = ((adc_2 * 3300) / 4096);
  if (volt_do_current < zero_ad712)
  {
    current = (zero_ad712 - volt_do_current) * 10; // ACS712 invert connect
  }
  else
  {
    current = 0;
  }
}

void mainTask(void)
{
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 10 / portTICK_PERIOD_MS; // 10 ms period TASK
  xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    task10msCnt++;
    
    adc1_value = adc1_convertion();
    adc2_value = adc2_convertion();
    voltageConvertion(adc1_value);
    currentConvertion(adc2_value);

    reset_WDT();
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}
