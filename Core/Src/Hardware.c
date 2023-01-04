#include <stdbool.h>
#include "main.h"
#include "hardware.h"

extern uint8_t state_set_pwr;
uint16_t zero_ad712;
uint16_t voltage;
uint16_t current;

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
  HAL_Delay (100); // delay t
  HAL_ADC_Start(&hadc2);
  HAL_ADC_PollForConversion(&hadc2,100);
  uint16_t adc_2 = (uint32_t) HAL_ADC_GetValue(&hadc2);
  zero_ad712 =((adc_2*3300)/4096);
  HAL_ADC_Stop(&hadc2);
}

void adc1_convertion(void)
{  // voltage in 0.01 V
  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1,100);
  uint16_t adc_1 = (uint32_t) HAL_ADC_GetValue(&hadc1);
  voltage = (adc_1*33*107)/4096; //9,1K 910 OHM -> 11*3=33 (36)V MAX INPUT 
  HAL_ADC_Stop(&hadc1);
}

void adc2_convertion(void)
{
  HAL_ADC_Start(&hadc2);
  HAL_ADC_PollForConversion(&hadc2,100);
  uint16_t adc_2 = (uint32_t) HAL_ADC_GetValue(&hadc2);
  uint16_t volt_do_current =((adc_2*3300)/4096);
  if (volt_do_current <zero_ad712) current = (zero_ad712 - volt_do_current)*10; //ACS712 invert connect
  else current = 0;
  HAL_ADC_Stop(&hadc2);
}
