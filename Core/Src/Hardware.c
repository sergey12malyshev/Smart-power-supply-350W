#include <stdbool.h>
#include "main.h"
#include "hardware.h"

extern uint8_t state_set_pwr;
uint16_t zero_ad712;

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern IWDG_HandleTypeDef hiwdg;
extern UART_HandleTypeDef huart1;

//-------------- WDT -------------------//

void reset_WDT(void)
{
  HAL_IWDG_Refresh(&hiwdg);
}

//-------------- GPIO -------------------//

void on_ps(void)
{
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
  state_set_pwr = true;
  HAL_Delay(10);
}

void off_ps(void)
{
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
  state_set_pwr = false;
}

//-------------- ADC -------------------//
void calibr_zero_AD712(void)
{
  HAL_Delay(100); // delay t
  HAL_ADC_Start(&hadc2);
  HAL_ADC_PollForConversion(&hadc2, 100);
  uint16_t adc_2 = (uint32_t)HAL_ADC_GetValue(&hadc2);
  zero_ad712 = ((adc_2 * 3300) / 4096);
  HAL_ADC_Stop(&hadc2);

  return;
}

uint16_t adc1_convertion(void)
{ // voltage in 0.01 V
  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1, 100);
  uint16_t adc_1 = (uint32_t)HAL_ADC_GetValue(&hadc1);

  HAL_ADC_Stop(&hadc1);

  return adc_1;
}

uint16_t adc2_convertion(void)
{
  HAL_ADC_Start(&hadc2);
  HAL_ADC_PollForConversion(&hadc2, 100);
  uint16_t adc_2 = (uint32_t)HAL_ADC_GetValue(&hadc2);

  HAL_ADC_Stop(&hadc2);

  return adc_2;
}


