.syntax unified
.cpu cortex-m3
.fpu softvfp
.thumb

.equ RCC_BASE_ADDRESS, 0x40021000
.equ GPIOC_BASE_ADDRESS, 0x40011000
.equ GPIOC_CRH_PIN13_MASK, 0xF00000
.equ GPIOC_CRH_PIN13_OUTPUT_PUSH_PULL, 0x100000
.equ GPIOC_ODR_PIN13_MASK, 0x2000
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

  LDR r1, =RCC_BASE_ADDRESS     @ Load RCC registers base address
  LDR r2, [r1, #0x18]           @ Load RCC_APB2ENR register configuration
  ORR r2, r2, #0x10             @ Set bit to enable GPIOC
  STR r2, [r1, #0x18]           @ Store modified RCC_APB2ENR register configuration

  LDR r1, =GPIOC_BASE_ADDRESS   @ Load GPIOC registers base address
  LDR r2, [r1, #0x04]           @ Load GPIOC_CRH register configuration
  LDR r3, =GPIOC_CRH_PIN13_MASK @ Load GPIOC_CRH_PIN13_MASK so we can work with only PC13 pin configuration
  BIC r2, r2, r3                @ Clear configuration for PC13
  
  @ Configure PC13 as MODE Output mode, max speed 10 MHz: 01
  @ and CNF General purpose output push-pull: 00
  @ Other pins left as reset value 4 (0b0100) - Floating input
  LDR r3, =GPIOC_CRH_PIN13_OUTPUT_PUSH_PULL @ Load value of PC13 pin configuration
  ORR r2, r2, r3                @ Set this value in register
  STR r2, [r1, #0x04]

  LDR r0, =GPIOC_ODR_PIN13_MASK
  MOV r2, #0                    @ r1 register will hold value which we will set on GPIOC_ODR register
main_loop:
  EOR r2, r2, r0                @ XOR GPIOC_ODR_PIN13_MASK with current value in r1. This will have effect of flipping the bit for PIN13
  STR r2, [r1, #0x0C]           @ Set ODR configuration

  @ Loop million times for delay
  MOV r3, 0
  LDR r4, =LOOP_END
delay_loop:
  ADD r3, r3, #1
  CMP r3, r4
  BLT delay_loop
  
  @ Go to the next iteration of main loop
  B main_loop
.size reset_handler, .-reset_handler
