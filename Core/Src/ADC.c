#include "main.h"
#include "hardware.h"
#include "mainTask.h"
#include "monitorTask.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

volatile uint16_t adc1_value;
volatile uint16_t adc2_value;

uint16_t zero_ad712;
static const uint8_t timoutAdc = 20u;

/* ADC API start */
void setADC1value(uint16_t adc1_result)
{
  adc1_value = adc1_result;
}

uint16_t getADC1value(void)
{
  return adc1_value;
}

void setADC2value(uint16_t adc2_result)
{
  adc2_value = adc2_result;
}

uint16_t getADC2value(void)
{
  return adc2_value;
}

void setZeroAD712(uint16_t zero)
{
  zero_ad712 = zero;
}

uint16_t getZeroAD712(void)
{
  return zero_ad712;
}

void calibr_zero_AD712(void)
{
  HAL_Delay(100); // delay t
  HAL_ADC_Start(&hadc2);
  HAL_ADC_PollForConversion(&hadc2, 100);
  uint16_t adc_2 = (uint32_t)HAL_ADC_GetValue(&hadc2);
  setZeroAD712((adc_2 * 3300) / 4096);
  HAL_ADC_Stop(&hadc2);
}

/* Регулярные каналы, преобразования АЦП запускаем программно */
uint16_t adc1_convertion(void)
{ 
  uint16_t adc_1 = 0; // voltage in 0.01 V
  
  HAL_ADC_Start(&hadc1);
  if (HAL_ADC_PollForConversion(&hadc1, timoutAdc) == HAL_OK)
  {
    adc_1 = (uint32_t)HAL_ADC_GetValue(&hadc1);
  }
  else
  {
    sendUART("ADC error");
  }

  HAL_ADC_Stop(&hadc1);

  return adc_1;
}

uint16_t adc2_convertion(void)
{
  HAL_ADC_Start(&hadc2);
  HAL_ADC_PollForConversion(&hadc2, timoutAdc);
  uint16_t adc_2 = (uint32_t)HAL_ADC_GetValue(&hadc2);

  HAL_ADC_Stop(&hadc2);

  return adc_2;
}
/* ADC API end */

