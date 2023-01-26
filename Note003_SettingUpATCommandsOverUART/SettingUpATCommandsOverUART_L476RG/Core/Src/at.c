#include <string.h>
#include <stdlib.h>
#include "at.h"
#include "main.h"

#define AT_COMMAND_BUFFER_SIZE 128

typedef struct ATCommandBuffer_
{
  char* buffer;
  int length;
} ATCommandBuffer;

static char at_command_buffer_array[AT_COMMAND_BUFFER_SIZE] = {0};
static ATCommandBuffer at_command_buffer =
{
  .length = 0,
  .buffer = at_command_buffer_array
};

static Ringbuffer *uart_ring_buffer;
static void (*uart_init)(void) = NULL;

static void at_copy_command_to_buffer(void);
static void at_command_execute(AT_Command command, AT_OperationType command_type, uint8_t param_uint8);

void at_command_init(Ringbuffer *uartrb, void (*uart_init_func)(void))
{
  uart_ring_buffer = uartrb;
  uart_init = uart_init_func;
}

void at_command_process(void)
{
  at_copy_command_to_buffer();
  AT_Command command = AT_Command_NONE;
  AT_OperationType command_type = AT_Command_TypeNotSet;

  char *command_ptr = NULL;
  uint8_t param_uint8 = 0;

  /* Go through all at commands and find which one is in buffer */
  if ((command_ptr = strstr(at_command_buffer.buffer, AT_PREFIX AT_LED_ON AT_EQUAL)) != NULL)
  {
    command = AT_Command_LED_ON;
    if (*(command_ptr + strlen(AT_PREFIX AT_LED_ON AT_EQUAL)) == '?')
    {
      command_type = AT_Command_GET;
    }
    else
    {
      command_type = AT_Command_SET;
      char *param_ptr = command_ptr + strlen(AT_PREFIX AT_LED_ON AT_EQUAL);
      param_uint8 = (uint8_t)strtoul(param_ptr, NULL, 10);
    }
  }
  else if ((command_ptr = strstr(at_command_buffer.buffer, AT_PREFIX AT_RESET)) != NULL)
  {
    command_type = AT_Command_Execute;
    command = AT_Command_RESET;
  }
  else if ((command_ptr = strstr(at_command_buffer.buffer, AT_PREFIX AT_HELP)) != NULL)
  {
    command_type = AT_Command_Execute;
    command = AT_Command_HELP;
  }

  /* Clean up buffer */
  memset(at_command_buffer.buffer, 0, at_command_buffer.length);
  at_command_buffer.length = 0;

  if (command != AT_Command_NONE)
      at_command_execute(command, command_type, param_uint8);
  else
  {
    printf("[ERROR:%d]\r\n\r\n", AT_ERROR_UNKNOWN_COMMAND);
    HAL_Delay(15);
    uart_init();
  }
}

static void at_command_execute(AT_Command command, AT_OperationType command_type, uint8_t param_uint8)
{
    switch (command)
    {
    case AT_Command_LED_ON:
    {
      if (command_type == AT_Command_GET)
      {
        GPIO_PinState state = HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin);
        printf("%u\r\n[OK]\r\n", state);
      }
      else if (command_type == AT_Command_SET)
      {
        if (param_uint8 == GPIO_PIN_SET || param_uint8 == GPIO_PIN_RESET)
        {
          HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, param_uint8);
          printf("[OK]\r\n");
        }
        else
        {
          printf("[ERROR:%u]\r\n", AT_ERROR_WRONG_PARAMETER_TO_COMMAND);
        }
      }
      break;
    }


    case AT_Command_RESET:
    {
      if (command_type == AT_Command_Execute)
      {
        printf("[OK]\r\n\r\n");
        HAL_Delay(25);
        NVIC_SystemReset();
      }
      break;
    }

    case AT_Command_HELP:
        printf("'AT+LED_ON=<0, 1>' - Command to Disable (0) \\ Enable (1) LED on your NUCLEO.\r\n");
        HAL_Delay(15);
        printf("'AT+RESET' - Perform Reset of the device\r\n");
        HAL_Delay(15);
        printf("[OK]\r\n");
        break;

    default:
        break;
    }

    /* Adds empty line which signifies end of the transmission */
    printf("\r\n");
}

static void at_copy_command_to_buffer(void)
{
    uint8_t ch = '\0';
    while ((ch = ringbuffer_read(uart_ring_buffer)) != '\0')
    {
        at_command_buffer.buffer[at_command_buffer.length++] = ch;
    }
    // for (int i = 0; i < at_command_buffer.length; i++)
    // {
    //     putchar(at_command_buffer.buffer[i]);
    // }
}

