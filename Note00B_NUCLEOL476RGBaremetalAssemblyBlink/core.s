.syntax unified
.cpu cortex-m4
.thumb

.equ RCC_BASE_ADDRESS, 0x40021000
.equ RCC_AHB2ENR_ADDRESS_OFFSET, 0x4C
.equ RCC_AHB2ENR_GPIOA_EN,0x01
.equ GPIOA_BASE_ADDRESS, 0x48000000
.equ GPIOA_MODER_ADDRESS_OFFSET, 0x00
.equ GPIO_PIN5_MODE_OFFSET, 10 
.equ GPIO_MODER_PIN5_MASK, 0x03 << GPIO_PIN5_MODE_OFFSET
.equ GPIO_OUTPUT_MODE, 0x01
.equ GPIO_MODER_PIN5_OUTPUT, 0x01 << GPIO_PIN5_MODE_OFFSET 
.equ GPIOA_OTYPER_ADDRESS_OFFSET, 0x04
.equ GPIOA_OSPEEDR_ADDRESS_OFFSET, 0x08
.equ GPIOA_BSRR_ADDRESS_OFFSET, 0x18
.equ GPIOA_BSRR_PIN5_SET_MASK, 1 << 5 
.equ GPIOA_BSRR_PIN5_RESET_MASK, 1 << 21
.equ LOOP_END, 1000000

.type vector_table, %object
vector_table:
    .word _estack
    .word reset_handler
.size vector_table, .-vector_table

.type reset_handler, %function
reset_handler:
  LDR  r0, =_estack
  MOV  sp, r0
	
	LDR r3, =RCC_AHB2ENR_ADDRESS_OFFSET
  LDR r1, =RCC_BASE_ADDRESS     				@ Load RCC registers base address
  LDR r2, [r1, r3] 											@ Load RCC_AHB2ENR register configuration
  ORR r2, r2, RCC_AHB2ENR_GPIOA_EN      @ Set bit to enable GPIOA
  STR r2, [r1, r3] 											@ Store modified RCC_APB2ENR register configuration
	
	LDR r3, =GPIOA_MODER_ADDRESS_OFFSET
  LDR r1, =GPIOA_BASE_ADDRESS   				@ Load GPIOA registers base address
  LDR r2, [r1, r3] 											@ Load GPIOA_MODER Register value in r2
  LDR r4, =GPIO_MODER_PIN5_MASK 				@ Load GPIOA_MODER_PIN5_MASK so we can work with only PA5 pin configuration
  BIC r2, r2, r4                				@ Clear configuration for PA5
 
 	@ Configure pin as general purpose output mode
  LDR r4, =GPIO_MODER_PIN5_OUTPUT				@ Load value of PC5 pin configuration
  ORR r2, r2, r4                				@ Set this value in register
  STR r2, [r1, r3] 											@ Store value back to the register
	
	@ Leave GPIOA_OTYPER Register PIN5 configuration to reset value: Output push-pull
	@ Leave GPIOA_OSPEEDR Register PIN5 configuration to reset value: Low speed 	
	
	LDR r0, =GPIOA_BSRR_ADDRESS_OFFSET
	LDR r2, =GPIOA_BSRR_PIN5_SET_MASK
	LDR r3, =GPIOA_BSRR_PIN5_RESET_MASK
	LDR r6, =GPIOA_BSRR_PIN5_SET_MASK    	@ Configuration for PA5 pin (to turn LED ON or OFF)

main_loop:
  STR r6, [r1, r0]											@ Store configuration to BSRR	register

	EOR r6, r2 														@ Flip PIN5 Enable bit. 
	EOR r6, r3 														@ Flip PIN5 Disable bit
	@ Because it started with Enable bit set and Disable bit reset
	@ it will flip to opposite states, so Enable and Disable will
	@ not be 1 at the same time

	@ Loop million times for delay
  MOV r4, 0
  LDR r5, =LOOP_END
delay_loop:
 	ADD r4, r4, #1
 	CMP r4, r5
	BLT delay_loop

  B main_loop 													@ Go to the next iteration of main loop
  
.size reset_handler, .-reset_handler
