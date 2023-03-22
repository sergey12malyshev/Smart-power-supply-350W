#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "hardware.h"
#include "mainTask.h"
#include "ADC.h"


uint32_t task10msCnt = 0;

uint16_t voltage;
uint16_t current;


void voltageConvertion(uint16_t adc_1)
{
  voltage = (adc_1 * 33 * 107) / 4096; // 9,1K 910 OHM -> 11*3=33 (36)V MAX INPUT
}

void currentConvertion(uint16_t adc_2)
{
  uint16_t volt_do_current = ((adc_2 * 3300) / 4096);
  if (volt_do_current < getZeroAD712())
  {
    current = (getZeroAD712() - volt_do_current) * 10; // ACS712 invert connect
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
    
    setADC1value(adc1_convertion());
    setADC2value(adc2_convertion());

    voltageConvertion(getADC1value());
    currentConvertion(getADC2value());

    reset_WDT();
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}
