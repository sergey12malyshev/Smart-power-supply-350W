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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>	
#include <stdbool.h>
#include "task.h"
#include "monitorTask.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;

IWDG_HandleTypeDef hiwdg;

UART_HandleTypeDef huart1;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for mainTask */
osThreadId_t mainTaskHandle;
const osThreadAttr_t mainTask_attributes = {
  .name = "mainTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for checkTask */
osThreadId_t checkTaskHandle;
const osThreadAttr_t checkTask_attributes = {
  .name = "checkTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for monitorTask */
osThreadId_t monitorTaskHandle;
const osThreadAttr_t monitorTask_attributes = {
  .name = "monitorTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* USER CODE BEGIN PV */

uint8_t hello_string[]= "Controller Power Supply\r\n";
uint8_t enter_help[]= "Enter HELP\r\n";
uint8_t version[]="v0.1\r\n";
uint8_t input_mon[]=" ";
uint8_t r_n[]="\r\n";
uint8_t error[]="ERROR\r\n";
uint8_t WARNING[]="WARNING:Power switch faulty!\r\n";
uint8_t mon_OK[]="OK\r\n";
uint8_t mon_comand[]="Enter monitor comman:\r\n\
HELP-See existing commands\r\n\
RSTT-Restart\r\n\
RSTW-restart using WDT\r\n\
TEST-Set LED\r\n\
VOLT-show out voltage 0.01V\r\n\
CURR-show out current\r\n\
ONPS-On Power switch\r\n\
OFPS-Off Power switch\r\n\
VIEW-voltage current power view\r\n\
>";
uint8_t symbol_term[]=">";

uint16_t zero_ad712;
uint16_t voltage;
uint16_t current;
uint8_t state_now_power; 
uint8_t state_set_pwr;

uint32_t task10msCnt = 0;

uint8_t input_mon_buff[]="    ";
uint8_t monitorTest = 0; // global flag TEST

uint8_t str[50];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_IWDG_Init(void);
void StartDefaultTask(void *argument);
void StartMainTask(void *argument);
void StartCheckTask(void *argument);
void StartMonitorTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void reset_WDT(void)
{
  HAL_IWDG_Refresh(&hiwdg);
}  

void on_ps (void)
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);
	state_set_pwr = true;
	HAL_Delay(10);
}	
void off_ps (void)
{
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
  state_set_pwr= false;
}	

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

void clear_uart_buff(void)
{
	memset(input_mon_buff,0,sizeof(input_mon_buff));
}

void monitor (void)
{
	static uint8_t rec_len = 0;

	if((huart1.RxXferCount==0)&&(HAL_UART_Receive_IT (&huart1, input_mon, 1) != HAL_BUSY)){                        
#if 1
		  HAL_UART_Transmit(&huart1,input_mon,1,0xFFFF); //Local echo
#endif
			if(input_mon[0] == 13){ // enter key
				HAL_UART_Transmit(&huart1,r_n,2,0xFFFF);
				HAL_UART_Transmit(&huart1,symbol_term,2,0xFFFF);
				rec_len =0;
				clear_uart_buff();
				monitorTest =0;  //reset TEST
			}else{
					input_mon_buff[rec_len++]=input_mon[0]; // load char do string
					if (rec_len>3){
						rec_len = 0;
						
					//HAL_UART_Transmit(&huart1,input_mon_buff,4,0xFFFF);
					HAL_UART_Transmit(&huart1,r_n,2,0xFFFF);
					if (((input_mon_buff[0] == 'H')||(input_mon_buff[0] == 'h'))&&((input_mon_buff[1] == 'E')||(input_mon_buff[1] == 'e'))\
						&&((input_mon_buff[2] == 'L')||(input_mon_buff[2] == 'l'))&&((input_mon_buff[3] == 'P')||(input_mon_buff[3] == 'p'))){ // enter HELP
						HAL_UART_Transmit(&huart1,mon_comand,236,0xFFFF);

					
					}else if (((input_mon_buff[0] == 'T')||(input_mon_buff[0] == 't'))&&((input_mon_buff[1] == 'E')||(input_mon_buff[1] == 'e'))\
						&&((input_mon_buff[2] == 'S')||(input_mon_buff[2] == 's'))&&((input_mon_buff[3] == 'T')||(input_mon_buff[3] == 't'))){ // enter TEST
								monitorTest = 3;		
								HAL_UART_Transmit(&huart1,mon_OK,4,0xFFFF);
				
					}else if ((input_mon_buff[0] == 'V')&&(input_mon_buff[1] == 'O')&&(input_mon_buff[2] == 'L')&&(input_mon_buff[3] == 'T')){ // enter VOLT
								monitorTest = 1;
								HAL_UART_Transmit(&huart1,mon_OK,4,0xFFFF);
				
					}else if ((input_mon_buff[0] == 'C')&&(input_mon_buff[1] == 'U')&&(input_mon_buff[2] == 'R')&&(input_mon_buff[3] == 'R')){ // enter CURR
								monitorTest = 2;
								HAL_UART_Transmit(&huart1,mon_OK,4,0xFFFF);

					}else if ((input_mon_buff[0] == 'O')&&(input_mon_buff[1] == 'N')&&(input_mon_buff[2] == 'P')&&(input_mon_buff[3] == 'S')){ // enter ONPS
								HAL_UART_Transmit(&huart1,mon_OK,4,0xFFFF);		
								on_ps();
					
					}else if ((input_mon_buff[0] == 'O')&&(input_mon_buff[1] == 'F')&&(input_mon_buff[2] == 'P')&&(input_mon_buff[3] == 'S')){ // enter OFPS
								HAL_UART_Transmit(&huart1,mon_OK,4,0xFFFF);				
								off_ps();
				
					}else if ((input_mon_buff[0] == 'R')&&(input_mon_buff[1] == 'S')&&(input_mon_buff[2] == 'T')&&(input_mon_buff[3] == 'W')){ // enter RSTW
								HAL_UART_Transmit(&huart1,mon_OK,4,0xFFFF);	
								vTaskSuspendAll();				
								while(1); 
					
					}else if ((input_mon_buff[0] == 'R')&&(input_mon_buff[1] == 'S')&&(input_mon_buff[2] == 'T')&&(input_mon_buff[3] == 'T')){ // enter RSTT
								HAL_UART_Transmit(&huart1,mon_OK,4,0xFFFF);	
								HAL_NVIC_SystemReset();
					
					}else if ((input_mon_buff[0] == 'V')&&(input_mon_buff[1] == 'I')&&(input_mon_buff[2] == 'E')&&(input_mon_buff[3] == 'W')){ // enter VIEW
								HAL_UART_Transmit(&huart1,mon_OK,4,0xFFFF);	
								monitorTest = 4;
					}else {
								HAL_UART_Transmit(&huart1,error,7,0xFFFF); 
								HAL_UART_Transmit(&huart1,symbol_term,1,0xFFFF);
					}
				clear_uart_buff();
				HAL_UART_Receive_IT(&huart1,(uint8_t*) input_mon,1);
		 }
	  }
	}
}

void monitor_out_test(void)
{
  uint32_t power;

	switch(monitorTest){
		case 1: 
		  sprintf((char *)str,"%d\r\n", voltage); // out Voltage
			HAL_UART_Transmit(&huart1, str, strlen((char *)str),0xFFFF);
			osDelay(100);	
			break;
		case 2: 		
			sprintf((char *)str,"%d\r\n", current); // out Curent
			HAL_UART_Transmit(&huart1, str, strlen((char *)str),0xFFFF);
			osDelay(100);
			break;
		case 3:                    // out TEST
			on_ps();
			osDelay(900);
			off_ps();
			osDelay(900);
			break;
		case 4:                    // out GLOB_TEST - dopisat obrabotchic
			sprintf((char *)str,"%d.%d\t", voltage/100, voltage%100); // out Voltage
			HAL_UART_Transmit(&huart1, str, strlen((char *)str),0xFFFF);
			sprintf((char *)str,"%d.%d\t", current/1000,current%1000); // out Curent
			HAL_UART_Transmit(&huart1, str, strlen((char *)str),0xFFFF);
			power = voltage * current;
			sprintf((char *)str,"%d.%d\r\n", power/100000,(power/10000)%10); 
		  HAL_UART_Transmit(&huart1, str, strlen((char *)str),0xFFFF);
			osDelay(100);
		  break;		
		default:;
	}
}

void Check_Task(void)
{
	if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5)== false) state_now_power = true;
	else state_now_power = false;
	
	if (state_now_power != state_set_pwr) 
  {
		HAL_UART_Transmit(&huart1, WARNING, 0, 0xFFFF); 
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
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
	
	off_ps(); 
	
	clear_uart_buff();
  HAL_UART_Receive_IT(&huart1,(uint8_t*) input_mon, 1);
	
	HAL_UART_Transmit(&huart1, hello_string, 25, 0xFFFF);
	HAL_UART_Transmit(&huart1, version, 6, 0xFFFF);
	HAL_UART_Transmit(&huart1, enter_help, 12, 0xFFFF);
	HAL_UART_Transmit(&huart1, symbol_term, 1, 0xFFFF);
	
	calibr_zero_AD712();
	
	HAL_ADCEx_Calibration_Start(&hadc1);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of mainTask */
  mainTaskHandle = osThreadNew(StartMainTask, NULL, &mainTask_attributes);

  /* creation of checkTask */
  checkTaskHandle = osThreadNew(StartCheckTask, NULL, &checkTask_attributes);

  /* creation of monitorTask */
  monitorTaskHandle = osThreadNew(StartMonitorTask, NULL, &monitorTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
	if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5)==0) state_now_power=false;
	else state_now_power=true;
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
		reset_WDT();
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartMainTask */
/**
* @brief Function implementing the mainTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMainTask */
void StartMainTask(void *argument)
{
  /* USER CODE BEGIN StartMainTask */
	 TickType_t xLastWakeTime;
   const TickType_t xFrequency = 10 / portTICK_PERIOD_MS; // 10 ms period TASK
   xLastWakeTime = xTaskGetTickCount();
  /* Infinite loop */
  for(;;)
  {
		task10msCnt++;
		reset_WDT();
		adc1_convertion();
		adc2_convertion();
		
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    //osDelay(1);
  }
  /* USER CODE END StartMainTask */
}

/* USER CODE BEGIN Header_StartCheckTask */
/**
* @brief Function implementing the checkTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCheckTask */
void StartCheckTask(void *argument)
{
  /* USER CODE BEGIN StartCheckTask */
		TickType_t xLastWakeTime;
		const TickType_t xFrequency = 10 / portTICK_PERIOD_MS; // 10 ms period TASK
		xLastWakeTime = xTaskGetTickCount();
  /* Infinite loop */
  for(;;)
  {
		Check_Task();
		reset_WDT();
		
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
  /* USER CODE END StartCheckTask */
}

/* USER CODE BEGIN Header_StartMonitorTask */
/**
* @brief Function implementing the monitorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMonitorTask */
void StartMonitorTask(void *argument)
{
  /* USER CODE BEGIN StartMonitorTask */
  monitorTask();
  /* USER CODE END StartMonitorTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
