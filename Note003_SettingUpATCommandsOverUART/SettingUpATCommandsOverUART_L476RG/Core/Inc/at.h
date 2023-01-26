#ifndef AT_H__
#define AT_H__

#include <stdio.h>
#include <stdbool.h>
#include "ringbuffer.h"

#define AT_PREFIX "AT+"
#define AT_END "\r\n"
#define AT_LED_ON "LED_ON"
#define AT_RESET "RESET"
#define AT_HELP "HELP"
#define AT_EQUAL "="


typedef enum
{
  AT_Command_NONE,
  AT_Command_LED_ON,
  AT_Command_RESET,
  AT_Command_HELP
} AT_Command;

typedef enum
{
  AT_Command_TypeNotSet,
  AT_Command_GET,
  AT_Command_SET,
  AT_Command_Execute
} AT_OperationType;

typedef enum
{
    AT_ERROR_UNKNOWN_COMMAND = 1,
    AT_ERROR_WRONG_PARAMETER_TO_COMMAND = 2,
    UNKNOWN_ERROR = 3
} AT_Error_Codes;


void at_command_init(Ringbuffer *uartrb, void (*uart_init_func)(void));
void at_command_process(void);

#endif /* AT_H__ */
