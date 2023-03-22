#include <stdbool.h>
#include "main.h"
#include "hardware.h"

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
}

void off_ps(void)
{
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

bool PinPowerEnableState(void)
{
  if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)== GPIO_PIN_RESET)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool checkStatePower(void)
{
  if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5)== GPIO_PIN_RESET)
  {
    return true;
  }
  else
  {
    return false;
  }
}


