# Setting Up AT Commands over UART

AT commands are old format for sending text commands to device over some interface.
General syntax of AT Command is as follows:

```
AT+<parameter>=?\r\n - Get value of <parameter>.
AT+<parameter>=<value>\r\n - Set <parameter> to <value>.
AT+<command>\r\n - Execute command.
```

Sometimes you need to build some app on your PC that will be interacting with device with this AT commands,
so device need some way to signal back to user that it's finished transmission. In this post I will use ```\r\n\r\n```
sequence to signal end of the transmission. Every command will return ```[OK]\r\n\r\n``` on success and ```[ERROR:<error_code>]\r\n\r\n``` on error.

We will build up on code from my [previous post about UART](https://kalleva.bearblog.dev/setting-up-uart-with-nucleo-l476rg-sending-strings-with-printf-and-recieving-lines-with-interrupts-to-ringbuffer/) so check it out before going further and get your project to echo back to you what you send to it over UART.

## AT Commands

Goal of this post is to create three AT Commands:

- ```AT+LED_ON=<0, 1>``` - Command to Disable (0) \ Enable (1) LED on your NUCLEO.
- ```AT+RESET``` - Perform Reset of the device.
- ```AT+HELP``` - Print available commands.

### Prerequisites

- NUCLEO-L476RG board but it should be fairly easy to adapt this to your board (I testes it with L0 board)
- [Stm32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)
- [Termite terminal](https://www.compuphase.com/software_termite.htm) to interact with device via UART.

### Adding AT Commands

1. Create ```at.h``` in ```Inc``` folder and ```at.c``` in ```Src``` folder.

2. In ```at.h``` create enum with all your commands:

```C
typedef enum
{
  AT_Command_NONE,
  AT_Command_LED_ON,
  AT_Command_RESET,
  AT_Command_HELP
} AT_Command;
```

3. In ```at.h``` define strings that will trigger these commands:

```C
#define AT_PREFIX "AT+"
#define AT_END "\r\n"
#define AT_LED_ON "LED_ON"
#define AT_RESET "RESET"
#define AT_HELP "HELP"
#define AT_EQUAL "="
```

4. AT command has to be one of the three types: get, set, or execute. Define them in enum in ```at.h```.

```C
typedef enum
{
  AT_Command_TypeNotSet,
  AT_Command_GET,
  AT_Command_SET,
  AT_Command_Execute
} AT_OperationType;
```

5. In ```at.h``` define errors to return back from the device, right now we define two possible errors:

```C
typedef enum
{
    AT_ERROR_UNKNOWN_COMMAND = 1,
    AT_ERROR_WRONG_PARAMETER_TO_COMMAND = 2,
    UNKNOWN_ERROR = 3
} AT_Error_Codes;
```

6. In ```at.c``` define buffer to hold command copied from ringbuffer.

```C
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
```

7. In ```at.c``` define function ```at_command_init``` that will set the pointer to ringbuffer from which we would get received AT Command.

``` C
static Ringbuffer *uart_ring_buffer;
static void (*uart_init)(void) = NULL;

void at_command_init(Ringbuffer *uartrb, void (*uart_init_func)(void))
{
  uart_ring_buffer = uartrb;
  uart_init = uart_init_func;
}
```

8. In ```at.c``` define function ```at_command_process```.
In this function we will first copy received AT command from ringbuffer to AT Command buffer, so we will be able to work with string functions on
it more straightforwardly.

```C
uint8_t ch = '\0';
while ((ch = ringbuffer_read(uart_ring_buffer)) != '\0')
{
    at_command_buffer.buffer[at_command_buffer.length++] = ch;
}
```

After that we will go through big ```if {} else if {}``` chain that will allow us to determine which AT Command we have received.

```C
if ((command_ptr = strstr(at_command_buffer.buffer, AT_PREFIX AT_LED_ON AT_EQUAL)) != NULL)
  {
    command = AT_Command_LED_ON;
```

Depending if we our commands ends with ```?``` we differentiate between Get and Set commands.
If we have set commands we parse parameter passed with command.

```C
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
```

9. After this if we have determined command type we call ```at_command_execute``` which will execute handler for this command.
If command type is not set we return ```AT_ERROR_UNKNOWN_COMMAND``` error and call ```uart_init``` function.
This step will allow us to have more reliable communication and will allow to recover from error if they will occur.

10. ```at_command_execute``` contains a big switch statement on command types that will execute particular handler based on

```C
switch (command)
{
}
```

AT Command and its type. 
For ```LED_ON``` command we will set the pin to the passed value if it valid with Set command type. And signal ```AT_ERROR_WRONG_PARAMETER_TO_COMMAND``` if it is not.
Or display state at which LED Pin is currently resides with Get command type.

```C
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
```

For ```RESET``` command we just print ```[OK]``` and just reset after some timeout so printed status is not cut short by the reset.

```C
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
```

For ```HELP``` command we print all the available commands with some description.

```C
case AT_Command_HELP:
    printf("'AT+LED_ON=<0, 1>' - Command to Disable (0) \\ Enable (1) LED on your NUCLEO.\r\n");
    HAL_Delay(15);
    printf("'AT+RESET' - Perform Reset of the device\r\n");
    HAL_Delay(15);
    printf("[OK]\r\n");
    break;
```

This concludes our AT Commands module. Now we need to use it in ```main.c```.

### Using AT Commands

At the ```main.c```:

- Include ```at.h``` at the head of the file.
- Call ```at_command_init``` function before going to endless loop in ```main``` function.
Pass it ringbuffer to which characters are passed from UART interrupt handler routine and also pass it function that allows to reinit UART. 

```C
/* USER CODE BEGIN 2 */
at_command_init(&uart_rb, MX_USART2_UART_Init);
```

- Inside ```while (1)``` loop inside ```main``` function comment code that makes device echo back lines that we pass to it.
And add call to ```at_command_process```:

```C
if (newline)
{
    newline = false;
    at_command_process();
}
```

### Testing

Open [Termite terminal](https://www.compuphase.com/software_termite.htm) configured with default UART settings:
```
Baud rate: 115200
Data bits: 8
Stop bits: 1
Parity: none
```

And you should be able to turn LED on your board ON and OFF and also cause board reboot with added AT Commands.