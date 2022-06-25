/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>			
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define reset_WDT    HAL_IWDG_Refresh(&hiwdg)
#define TRUE 1
#define FALSE 0

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;

IWDG_HandleTypeDef hiwdg;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

uint8_t hello_string[]= "Controller Power Supply\r\n";
uint8_t enter_help[]= "Enter HELP\r\n";
uint8_t version[]="v0.1\r\n";
uint8_t input_mon[]="    ";
uint8_t r_n[]="\r\n";
uint8_t error[]="ERROR\r\n";
uint8_t WARNING[]="WARNING:Power switch faulty!\r\n";
uint8_t mon_OK[]="OK\r\n";
uint8_t mon_comand[]="Enter monitor comman:\r\n\
HELP-See existing commands\r\n\
RSTT-Restart\r\n\
RSTW-restart using WDT\r\n\
TEST-Set LED\r\n\
ADC1-show out voltage\r\n\
ADC2-show out current\r\n\
ONPS-On Power switch\r\n\
OFPS-Off Power switch\r\n";

uint16_t zero_ad712;
uint16_t voltage;
uint16_t current;
uint8_t state_now_power; 
uint8_t state_set_pwr;

uint8_t str[50];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_IWDG_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void on_ps (void){
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);
		state_set_pwr = TRUE;
		HAL_Delay(10);
}	
void off_ps (void){
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
		state_set_pwr= FALSE;
}	

void calibr_zero_AD712 (void){
		HAL_ADC_Start(&hadc2);
		HAL_ADC_PollForConversion(&hadc2,100);
		uint16_t adc_2 = (uint32_t) HAL_ADC_GetValue(&hadc2);
		zero_ad712 =((adc_2*3300)/4096);
	  HAL_ADC_Stop(&hadc2);
}

void adc1_convertion(void){

    HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1,100);
		uint16_t adc_1 = (uint32_t) HAL_ADC_GetValue(&hadc1);
		voltage = (adc_1*33*107)/4096; //9,1K 910 OHM -> 11*3=33 (36)V MAX INPUT
	  HAL_ADC_Stop(&hadc1);
}

void adc2_convertion(void){

    HAL_ADC_Start(&hadc2);
		HAL_ADC_PollForConversion(&hadc2,100);
		uint16_t adc_2 = (uint32_t) HAL_ADC_GetValue(&hadc2);
		uint16_t volt_do_current =((adc_2*3300)/4096);
		if (volt_do_current <zero_ad712) current = (zero_ad712 - volt_do_current)*10; //ACS712 invert connect
		else current = 0;
	  HAL_ADC_Stop(&hadc2);
}


void monitor (void){
	if(huart1.RxXferCount==0){                        
		//HAL_UART_Transmit(&huart1,input_mon,4,0xFFFF); //Local echo
			HAL_UART_Transmit(&huart1,r_n,2,0xFFFF);
		  
				if ((input_mon[0] == 'H')&&(input_mon[1] == 'E')&&(input_mon[2] == 'L')&&(input_mon[3] == 'P')){ // enter HELP
				HAL_UART_Transmit(&huart1,mon_comand,195,0xFFFF);

				
			}else if ((input_mon[0] == 'T')&&(input_mon[1] == 'E')&&(input_mon[2] == 'S')&&(input_mon[3] == 'T')){ // enter TEST
				HAL_UART_Transmit(&huart1,mon_OK,4,0xFFFF);
				on_ps();
				HAL_Delay(400);
				off_ps();
				reset_WDT;
				HAL_Delay(400);
				reset_WDT;
			
			}else if ((input_mon[0] == 'A')&&(input_mon[1] == 'D')&&(input_mon[2] == 'C')&&(input_mon[3] == '1')){ // enter ADC1
				for(int i=1; i<95; i++) {
					adc1_convertion();
					sprintf(str,"%d\r\n", voltage);
					HAL_UART_Transmit(&huart1, str, strlen((char *)str),0xFFFF);
					reset_WDT;
					HAL_Delay(150);	
				}
				HAL_UART_Transmit(&huart1,mon_OK,4,0xFFFF);
			
			}else if ((input_mon[0] == 'A')&&(input_mon[1] == 'D')&&(input_mon[2] == 'C')&&(input_mon[3] == '2')){ // enter ADC2
				for(int i=1; i<95; i++) {
					adc2_convertion();
					sprintf(str,"%d\r\n", current);
					HAL_UART_Transmit(&huart1, str, strlen((char *)str),0xFFFF);
					reset_WDT;
					HAL_Delay(150);
				}
				HAL_UART_Transmit(&huart1,mon_OK,4,0xFFFF);

			}else if ((input_mon[0] == 'O')&&(input_mon[1] == 'N')&&(input_mon[2] == 'P')&&(input_mon[3] == 'S')){ // enter ONPS
				HAL_UART_Transmit(&huart1,mon_OK,4,0xFFFF);		
				on_ps();
				
			}else if ((input_mon[0] == 'O')&&(input_mon[1] == 'F')&&(input_mon[2] == 'P')&&(input_mon[3] == 'S')){ // enter OFPS
				HAL_UART_Transmit(&huart1,mon_OK,4,0xFFFF);				
				off_ps();
			
			}else if ((input_mon[0] == 'R')&&(input_mon[1] == 'S')&&(input_mon[2] == 'T')&&(input_mon[3] == 'W')){ // enter RSTW
				HAL_UART_Transmit(&huart1,mon_OK,4,0xFFFF);				
				for(int i=1; i<100; i++) {
				HAL_Delay(10);
				}
			}else if ((input_mon[0] == 'R')&&(input_mon[1] == 'S')&&(input_mon[2] == 'T')&&(input_mon[3] == 'T')){ // enter RSTT
				HAL_NVIC_SystemReset();
				
			}else HAL_UART_Transmit(&huart1,error,7,0xFFFF); 

			memset(input_mon,0,sizeof(input_mon));
			HAL_UART_Receive_IT(&huart1,(uint8_t*) input_mon,4);
		}
}

void Check_Task(void){
	if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5)==0) state_now_power=TRUE;
	else state_now_power = FALSE;
	
	if (state_now_power !=state_set_pwr) {
		HAL_UART_Transmit(&huart1,WARNING,30,0xFFFF); 
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init(); 												//https://istarik.ru/blog/stm32/113.html
  MX_ADC2_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
	
	off_ps(); 
	
	memset(input_mon,0,sizeof(input_mon));
  HAL_UART_Receive_IT(&huart1,(uint8_t*) input_mon,4);
	
	HAL_Delay(1);
	HAL_UART_Transmit(&huart1,hello_string,25,0xFFFF);
	HAL_Delay(1);
	HAL_UART_Transmit(&huart1,version,6,0xFFFF);
	HAL_Delay(1);
	HAL_UART_Transmit(&huart1,enter_help,12,0xFFFF);
	
	calibr_zero_AD712();
	
	HAL_ADCEx_Calibration_Start(&hadc1);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		adc1_convertion();
		adc2_convertion();
		monitor();
		Check_Task();
		reset_WDT;
		
		HAL_Delay(10);
	
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */
  /** Common config
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_16;
  hiwdg.Init.Reload = 1250;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5)==0) state_now_power=FALSE;
	else state_now_power=TRUE;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/