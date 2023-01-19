In good part of my projects I need to enable some device logging over and frequently I need to send something to device over UART.
This post sums up process of adding ```printf``` and ability to receive characters from UART in the device.

### Resources this post is based on:

[Bare Metal STM32 Programming (Part 10): UART Communication](https://vivonomicon.com/2020/06/28/bare-metal-stm32-programming-part-10-uart-communication/) - part on UART from great series by **vivonomicon**.


### Prerequisites

- NUCLEO-L476RG board
- [Stm32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)
- [Termite terminal](https://www.compuphase.com/software_termite.htm) to interact with device via UART.

### 1. Starting up

Select **NUCLEO-L476RG** in Stm32CubeIDE board selector menu.
You will be prompted with proposal to initialize all peripherals to their default value, select Yes.
Now you have project to work with.
Default UART settings:

```
Baud rate: 115200
Data bits: 8
Stop bits: 1
Parity: none
```

Our application will wait for ```\n``` newline so it's useful to check in Termite in section Transmitted text ```Append CR-LF``` checkbox.

### 2. Adding ability to printf to UART

We will use ```UART2``` for communications.
Include ```<stdio.h>``` on top of ```main.c``` file, to be able to use ```printf```.
Add this function to the bottom of ```main.c```.

```C
int _write(int handle, char* data, int size)
{
  int count = size;
  while (count--)
  {
    while (!(USART2->ISR & USART_ISR_TXE)) __NOP();
    USART2->TDR = *data++;
  }
  return size;
}
```

Now you should be able to use ```printf```.
To test this add these two lines to ```while (1)``` inside ```main``` function.

```C
printf("Hello, World!\r\n");
HAL_Delay(1000);
```

### 3. Getting data from UART

1. We need to enable interrupts for Rx line of ```UART2```.
Go back to STM32CubeMx screen inside IDE.
Select ```UART2```>```NVIC Settings``` and check ```USART2 global interrupt``` checkbox, after that regenerate code.

2. To configure ```UART2``` to wait for incoming character go to file ```stm32l4xx_hal_msp.c``` and insert at the bottom of
```HAL_UART_MspInit``` function:

```C
USART2->CR1 |= (USART_CR1_RE |
                USART_CR1_TE |
                USART_CR1_UE |
                USART_CR1_RXNEIE);
```

3. We will write incoming over UART characters to ring buffer.
So we need to add ```ringbuffer.h``` header with this code to ```Inc``` folder of a project.

```C
#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <stdint.h>

typedef struct Ringbuffer_
{
    uint32_t len;
    volatile uint8_t *buffer;
    volatile uint32_t head;
    volatile uint32_t tail;
} Ringbuffer;

#define ringbuffer_write(rb, x) \
    rb.buffer[rb.head] = x; \
    if ((rb.head + 1) >= rb.len) rb.head = 0; \
    else rb.head = rb.head + 1;

/* Read from a buffer. Returns '\0' if there is nothing to read. */
static inline char ringbuffer_read(Ringbuffer* buf) {
    if (buf->tail == buf->head) return '\0';
    uint8_t read = buf->buffer[buf->tail];
    buf->tail = (buf->tail < ( buf->len - 1 )) ? (buf->tail + 1) : 0;
    return read;
}

#endif /* RINGBUFFER_H_ */
```

4. Include ```ringbuffer.h``` to ```main.c```.

5. Add variables to ```/* USER CODE BEGIN PV */``` section in ```main.c```:

```C
#define UART_RING_BUFFER_SIZE 512

volatile uint8_t rb_buffer[UART_RING_BUFFER_SIZE + 1];
Ringbuffer uart_rb = {
    .len = UART_RING_BUFFER_SIZE,
    .buffer = rb_buffer,
    .head = 0,
    .tail = 0
};

volatile bool newline = false;
```

6. Comment out ```USART2_IRQHandler``` inside ```stm32l4xx_it.c```.
And place ```USART2_IRQHandler``` inside ```main.c```. It receives characters and puts them to ringbuffer.
If we encounter newline character we set up ```newline``` flag.

```C
void USART2_IRQHandler(void)
{
  if (USART2->ISR & USART_ISR_RXNE) {
    /* Copy new data into the buffer. */
    uint8_t c = USART2->RDR;
    ringbuffer_write(uart_rb, c);
    if (c == '\n')
      newline = true;
  }
}
```

7. Inside ```while(1)``` in function ```main``` comment out previously added code.
And add this code to echo received line back to UART after we received newline character:

```C
uint8_t ch = '\0';
while ((ch = ringbuffer_read(&uart_rb)) != '\0')
    putchar(ch); /* Echo to UART */
``` 

At this point you should be able to send lines ended with ```\n``` via UART and get them printed back to you.
