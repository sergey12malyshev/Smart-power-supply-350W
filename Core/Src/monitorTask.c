#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "monitorTask.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "task.h"
#include "main.h"
#include "hardware.h"
#include "mainTask.h"
#include "ADC.h"

#define mon_strcmp(ptr, cmd) (!strcmp(ptr, cmd))
#define LOCAL_ECHO_EN  1

extern IWDG_HandleTypeDef hiwdg;
extern UART_HandleTypeDef huart1;

extern uint16_t voltage;
extern uint16_t current;

typedef enum
{
  NONE = 0,
  RST,
  R,
  TEST,
  ADC,
  VOLTAGE,
  CURRENT,
  ON,
  OFF,
  POWER,
  INFO
}COMAND;

static uint8_t hello_string[] = "Controller Power Supply\r\n";
static uint8_t enter_help[] = "Enter HELP\r\n";
static uint8_t r_n[] = "\r\n";
static uint8_t error[] = "incorrect enter\r\n";
static uint8_t WARNING[] = "WARNING:Power switch faulty!\r\n";
static uint8_t mon_OK[] = "OK\r\n";
static uint8_t backspace_str[] = " \b";
static uint8_t mon_comand[] = "Enter monitor command:\r\n\
HELP-see existing commands\r\n\
RST-restart\r\n\
R-restart using WDT\r\n\
TEST- switch test\r\n\
ADC-show ADC chanel\r\n\
VOLTAGE-show out voltage (0.01V)\r\n\
CURRENT-show out current (mA)\r\n\
ON-On Power switch\r\n\
OFF-Off Power switch\r\n\
POWER-voltage current power view\r\n\
INFO - read about project\r\n\
>";
static uint8_t symbol_term[] = ">";

uint8_t input_mon[1] = {0};
const uint8_t sizeBuff = 9;
char input_mon_buff[sizeBuff] = {0};

uint8_t str[50];

COMAND monitorTest = NONE; // global flag TEST

//-------------- UART -------------------//

void clear_uart_buff(void)
{
  memset(input_mon_buff, 0, sizeof(input_mon_buff));
}

void sendUART(uint8_t TxBufferUartLocal[])
{ //передача в блокирующем режиме
  const uint8_t block_timeout_ms = 40; //t(sec)=(FRAME/BOUND+MINT)*N = (10/115200+0.00001)*100 = 19 мс

  HAL_UART_Transmit(&huart1, (uint8_t*) TxBufferUartLocal, strlen((char *) TxBufferUartLocal), block_timeout_ms);
}

void UART_receve_IT(void)
{
  HAL_UART_Receive_IT(&huart1, (uint8_t *)input_mon, 1);
}

void sendUART_WARNING(void)
{
  sendUART(WARNING);
}

static void sendUART_symbolTerm(void)
{
  sendUART(symbol_term);
}

static void sendSNversion(void)
{
  uint8_t str[14];

  sprintf((char *)str, "Version: %d", VERSION_MAJOR);
  sendUART(str);
  sendUART(".");
  sprintf((char *)str, "%d", VERSION_MINOR);
  sendUART(str);
  sendUART(".");
  sprintf((char *)str, "%d\r\n", VERSION_PATCH);
  sendUART(str);
}

void sendUART_hello(void)
{
  sendUART(hello_string);
  sendSNversion();
  sendUART(enter_help);
  sendUART_symbolTerm();
}

void sendUART_help(void)
{
  sendUART(mon_comand);
}

static void sendUART_OK(void)
{
  sendUART(mon_OK);
}

static void sendUART_r_n(void)
{
  sendUART(r_n);
}

static void sendUART_error(void)
{
  sendUART(error);
}

static void sendBackspaceStr(void)
{
  sendUART(backspace_str);
}

static void convertToUppercase(void)
{
  static char *copy_ptr;

  copy_ptr = input_mon_buff;
  while (*copy_ptr != 0)
  {
    *copy_ptr = toupper(*copy_ptr);
    copy_ptr++;
  }
}

static void monitor(void)
{
  static uint8_t rec_len = 0;
  const uint8_t enter = 13;
  const uint8_t Backspace = 0x08;

  if ((huart1.RxXferCount == 0) && (HAL_UART_Receive_IT(&huart1, input_mon, 1) != HAL_BUSY))
  {
#if LOCAL_ECHO_EN
    HAL_UART_Transmit(&huart1, input_mon, 1, 50); // Local echo
#endif
    if (input_mon[0] == enter)
    {
      convertToUppercase();
      sendUART_r_n();
      if ((input_mon_buff[0] == 'H') && (input_mon_buff[1] == 'E') && (input_mon_buff[2] == 'L') && (input_mon_buff[3] == 'P')) // enter HELP
      {
        sendUART_help();
      }
      else if ((input_mon_buff[0] == 'T') && (input_mon_buff[1] == 'E') && (input_mon_buff[2] == 'S') && (input_mon_buff[3] == 'T'))
      { // enter TEST
        monitorTest = TEST;
        sendUART_OK();
      }
       else if ((input_mon_buff[0] == 'A') && (input_mon_buff[1] == 'D') && (input_mon_buff[2] == 'C'))
      { // enter ADC
        monitorTest = ADC;
        sendUART_OK();
      }
      else if (mon_strcmp(input_mon_buff, (void *)"VOLTAGE"))
      {
        monitorTest = VOLTAGE;
        sendUART_OK();
      }
      else if (mon_strcmp(input_mon_buff, "CURRENT"))
      {
        monitorTest = CURRENT;
        sendUART_OK();
      }
      else if ((input_mon_buff[0] == 'O') && (input_mon_buff[1] == 'N'))
      { // enter ON
        sendUART_OK();
        on_ps();
        osDelay(10);
      }
      else if ((input_mon_buff[0] == 'O') && (input_mon_buff[1] == 'F') && (input_mon_buff[2] == 'F'))
      { // enter OFF
        sendUART_OK();
        off_ps();
      }
      else if ((input_mon_buff[0] == 'R') && (input_mon_buff[1] == 'S') && (input_mon_buff[2] == 'T'))
      { // enter RST
        sendUART_OK();
        vTaskSuspendAll();
        while (1);
      }
      else if ((input_mon_buff[0] == 'R'))
      {
        sendUART_OK();
        HAL_NVIC_SystemReset();
      }
      else if (mon_strcmp(input_mon_buff, "POWER"))
      {
        sendUART_OK();
        monitorTest = POWER;
      }
      else if (mon_strcmp(input_mon_buff, "INFO"))
      {
        sendUART_OK();
        sendUART("https://github.com/sergey12malyshev/Smart-power-supply-350W.git\r\n");
      }
      else
      {
        if (input_mon_buff[0] == 0)
        {
          sendUART_symbolTerm();
          clear_uart_buff();
          rec_len = 0;
          monitorTest = NONE; // reset TEST
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
      if (input_mon[0] == Backspace)
      {
        if (rec_len != 0)
        {
          input_mon_buff[rec_len - 1] = 0;
          rec_len--;
          sendBackspaceStr();
        }
      }
      else
      {
        if (rec_len <= sizeBuff)
        {
          input_mon_buff[rec_len++] = input_mon[0]; // load char do string
        }
      }
    }
  }
}

static void monitor_out_test(void)
{
  uint32_t power;

  switch (monitorTest)
  {
    case ADC:
      sprintf((char *)str, "%d\t", getADC1value());
      sendUART(str);
      sprintf((char *)str, "%d\r\n", getADC2value());
      sendUART(str);
      osDelay(100);
      break;
    case VOLTAGE:
      sprintf((char *)str, "%d\r\n", voltage);
      sendUART(str);
      osDelay(100);
      break;
    case CURRENT:
      sprintf((char *)str, "%d\r\n", current);
      sendUART(str);
      osDelay(100);
      break;
    case TEST:
      on_ps();
      osDelay(900);
      off_ps();
      osDelay(900);
      break;
    case POWER:
      sprintf((char *)str, "%d.%d\t", voltage / 100, voltage % 100);
      sendUART(str);
      sprintf((char *)str, "%d.%d\t", current / 1000, current % 1000);
      sendUART(str);
      power = voltage * current;
      sprintf((char *)str, "%d.%d\r\n", power / 100000, (power / 10000) % 10);
      sendUART(str);
      osDelay(100);
      break;
    default:;
  }
}

/*
Задача монитора-отладчика, запуск и исполнение тестов
*/

void monitorTask(void)
{
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 20 / portTICK_PERIOD_MS; // 20 ms period TASK
  xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    monitor();
    monitor_out_test();
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}
