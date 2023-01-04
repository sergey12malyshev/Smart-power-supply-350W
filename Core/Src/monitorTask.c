#include <stdio.h>
#include <string.h>	
#include <stdbool.h>

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "task.h"
#include "main.h"
#include "hardware.h"

extern IWDG_HandleTypeDef hiwdg;
extern UART_HandleTypeDef huart1;

extern uint16_t zero_ad712;
extern uint16_t voltage;
extern uint16_t current;

uint8_t hello_string[]= "Controller Power Supply\r\n";
uint8_t enter_help[]= "Enter HELP\r\n";
uint8_t version[]="v0.1\r\n";
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

uint8_t input_mon[1]={0};
uint8_t input_mon_buff[5]={0};

uint8_t str[50];
uint8_t monitorTest = 0; // global flag TEST

//-------------- UART -------------------//
const uint16_t uartBlock_ms = 120;

void clear_uart_buff(void)
{
  memset(input_mon_buff, 0, sizeof(input_mon_buff));
}

void UART_receve_IT(void)
{
  HAL_UART_Receive_IT(&huart1,(uint8_t*) input_mon, 1);
}

void sendUART_WARNING(void)
{
  HAL_UART_Transmit(&huart1, WARNING, 0, uartBlock_ms); 
}

static void sendUART_symbolTerm(void)
{
  HAL_UART_Transmit(&huart1, symbol_term, 1, uartBlock_ms);
}

void sendUART_hello(void)
{
  HAL_UART_Transmit(&huart1, hello_string, 25, uartBlock_ms);
  HAL_UART_Transmit(&huart1, version, 6, uartBlock_ms);
  HAL_UART_Transmit(&huart1, enter_help, 12, uartBlock_ms);
  sendUART_symbolTerm();
}

static void sendUART_OK(void)
{
  HAL_UART_Transmit(&huart1, mon_OK, 4, uartBlock_ms); 
}

static void sendUART_r_n(void)
{
  HAL_UART_Transmit(&huart1, r_n, 2, uartBlock_ms);
}

static void sendUART_error(void)
{
  HAL_UART_Transmit(&huart1, error, 7, uartBlock_ms); 
}


static void monitor(void)
{
  static uint8_t rec_len = 0;

  if((huart1.RxXferCount==0)&&(HAL_UART_Receive_IT (&huart1, input_mon, 1) != HAL_BUSY))
  {                        
#if 1
    HAL_UART_Transmit(&huart1, input_mon, 1, uartBlock_ms); //Local echo
#endif
	if(input_mon[0] == 13)
    { // enter key
      sendUART_r_n();
		if (((input_mon_buff[0] == 'H')||(input_mon_buff[0] == 'h'))&&((input_mon_buff[1] == 'E')||(input_mon_buff[1] == 'e'))\
		&&((input_mon_buff[2] == 'L')||(input_mon_buff[2] == 'l'))&&((input_mon_buff[3] == 'P')||(input_mon_buff[3] == 'p')))
        { // enter HELP
		  HAL_UART_Transmit(&huart1, mon_comand, strlen((char *)mon_comand), uartBlock_ms);
        }
        else if (((input_mon_buff[0] == 'T')||(input_mon_buff[0] == 't'))&&((input_mon_buff[1] == 'E')||(input_mon_buff[1] == 'e'))\
		  &&((input_mon_buff[2] == 'S')||(input_mon_buff[2] == 's'))&&((input_mon_buff[3] == 'T')||(input_mon_buff[3] == 't'))){ // enter TEST
		  monitorTest = 3;		
		  sendUART_OK();
		}
        else if ((input_mon_buff[0] == 'V')&&(input_mon_buff[1] == 'O')&&(input_mon_buff[2] == 'L')&&(input_mon_buff[3] == 'T')){ // enter VOLT
		  monitorTest = 1;
	      sendUART_OK();
		}
        else if ((input_mon_buff[0] == 'C')&&(input_mon_buff[1] == 'U')&&(input_mon_buff[2] == 'R')&&(input_mon_buff[3] == 'R')){ // enter CURR
		  monitorTest = 2;
		  sendUART_OK();
        }
        else if ((input_mon_buff[0] == 'O')&&(input_mon_buff[1] == 'N')&&(input_mon_buff[2] == 'P')&&(input_mon_buff[3] == 'S')){ // enter ONPS
		  sendUART_OK();		
		  on_ps();
		}
        else if ((input_mon_buff[0] == 'O')&&(input_mon_buff[1] == 'F')&&(input_mon_buff[2] == 'P')&&(input_mon_buff[3] == 'S')){ // enter OFPS
		  sendUART_OK();				
		  off_ps();
		}
        else if ((input_mon_buff[0] == 'R')&&(input_mon_buff[1] == 'S')&&(input_mon_buff[2] == 'T')&&(input_mon_buff[3] == 'W')){ // enter RSTW
		  sendUART_OK();	
		  vTaskSuspendAll();				
		  while(1); 			
		}
        else if ((input_mon_buff[0] == 'R')&&(input_mon_buff[1] == 'S')&&(input_mon_buff[2] == 'T')&&(input_mon_buff[3] == 'T')){ // enter RSTT
		  sendUART_OK();	
		  HAL_NVIC_SystemReset();
		}
        else if ((input_mon_buff[0] == 'V')&&(input_mon_buff[1] == 'I')&&(input_mon_buff[2] == 'E')&&(input_mon_buff[3] == 'W')){ // enter VIEW
		  sendUART_OK();	
		  monitorTest = 4;
		}
        else
        {
          if (input_mon_buff[0] == 0)
          {
	        sendUART_symbolTerm();
	        clear_uart_buff();
            rec_len = 0;
	        monitorTest = 0;  //reset TEST
          }
          else
          {
		    sendUART_error(); 
		    sendUART_symbolTerm();
          }
		}
        
        clear_uart_buff();
        rec_len = 0;	
	}
    else
    {
      if (rec_len <= 5)
      {
        input_mon_buff[rec_len++]=input_mon[0]; // load char do string
      }  
	}
  }
}

static void monitor_out_test(void)
{
  uint32_t power;

	switch(monitorTest)
    {
		case 1: 
		  sprintf((char *)str,"%d\r\n", voltage); // out Voltage
			HAL_UART_Transmit(&huart1, str, strlen((char *)str), uartBlock_ms);
			osDelay(100);	
			break;
		case 2: 		
			sprintf((char *)str,"%d\r\n", current); // out Curent
			HAL_UART_Transmit(&huart1, str, strlen((char *)str), uartBlock_ms);
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
			HAL_UART_Transmit(&huart1, str, strlen((char *)str), uartBlock_ms);
			sprintf((char *)str,"%d.%d\t", current/1000,current%1000); // out Curent
			HAL_UART_Transmit(&huart1, str, strlen((char *)str), uartBlock_ms);
			power = voltage * current;
			sprintf((char *)str,"%d.%d\r\n", power/100000,(power/10000)%10); 
		    HAL_UART_Transmit(&huart1, str, strlen((char *)str), uartBlock_ms);
			osDelay(100);
		  break;		
		default:;
	}
}


void monitorTask(void)
{
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 20 / portTICK_PERIOD_MS; // 20 ms period TASK
  xLastWakeTime = xTaskGetTickCount();

  for(;;)
  {
    monitor();
    monitor_out_test();
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}
