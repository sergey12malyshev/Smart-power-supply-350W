#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "task.h"
#include "main.h"
#include "hardware.h"

#define mon_strcmp(ptr, cmd) (!strcmp(ptr, cmd))

extern IWDG_HandleTypeDef hiwdg;
extern UART_HandleTypeDef huart1;

extern uint16_t zero_ad712;
extern uint16_t voltage;
extern uint16_t current;

enum COMAND
{
  HELP = 0,
  RST,
  R,
  TEST,
  VOLTAGE,
  CURRENT,
  ONPS,
  OFF,
  POWER
};

static uint8_t hello_string[] = "Controller Power Supply\r\n";
static uint8_t enter_help[] = "Enter HELP\r\n";
static uint8_t version[] = "v0.1\r\n";
static uint8_t r_n[] = "\r\n";
static uint8_t error[] = "ERROR\r\n";
static uint8_t WARNING[] = "WARNING:Power switch faulty!\r\n";
static uint8_t mon_OK[] = "OK\r\n";
static uint8_t backspace_str[] = " \b";
static uint8_t mon_comand[] = "Enter monitor comman:\r\n\
HELP-see existing commands\r\n\
RST-restart\r\n\
R-restart using WDT\r\n\
TEST-Set LED\r\n\
VOLTAGE-show out voltage 0.01V\r\n\
CURRENT-show out current\r\n\
ON-On Power switch\r\n\
OFF-Off Power switch\r\n\
POWER-voltage current power view\r\n\
>";
static uint8_t symbol_term[] = ">";

uint8_t input_mon[1] = {0};
const uint8_t sizeBuff = 9;
char input_mon_buff[sizeBuff] = {0};

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
  HAL_UART_Receive_IT(&huart1, (uint8_t *)input_mon, 1);
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

void sendUART_help(void)
{
  HAL_UART_Transmit(&huart1, mon_comand, strlen((char *)mon_comand), uartBlock_ms);
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

static void sendBackspaceStr(void)
{
  HAL_UART_Transmit(&huart1, backspace_str, 2, uartBlock_ms);
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
#if 1
    HAL_UART_Transmit(&huart1, input_mon, 1, uartBlock_ms); // Local echo
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
      else
      {
        if (input_mon_buff[0] == 0)
        {
          sendUART_symbolTerm();
          clear_uart_buff();
          rec_len = 0;
          monitorTest = 0; // reset TEST
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
  case VOLTAGE:
    sprintf((char *)str, "%d\r\n", voltage);
    HAL_UART_Transmit(&huart1, str, strlen((char *)str), uartBlock_ms);
    osDelay(100);
    break;
  case CURRENT:
    sprintf((char *)str, "%d\r\n", current);
    HAL_UART_Transmit(&huart1, str, strlen((char *)str), uartBlock_ms);
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
    HAL_UART_Transmit(&huart1, str, strlen((char *)str), uartBlock_ms);
    sprintf((char *)str, "%d.%d\t", current / 1000, current % 1000);
    HAL_UART_Transmit(&huart1, str, strlen((char *)str), uartBlock_ms);
    power = voltage * current;
    sprintf((char *)str, "%d.%d\r\n", power / 100000, (power / 10000) % 10);
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

  for (;;)
  {
    monitor();
    monitor_out_test();
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}
