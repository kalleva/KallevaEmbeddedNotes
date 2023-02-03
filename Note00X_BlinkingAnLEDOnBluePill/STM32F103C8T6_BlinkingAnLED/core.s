.syntax unified
.cpu cortex-m3
.fpu softvfp
.thumb

.type vector_table, %object
vector_table:
    .word _estack
    .word reset_handler
.size vector_table, .-vector_table

.type reset_handler, %function
reset_handler:
  LDR  r0, =_estack
  MOV  sp, r0

  LDR  r7, =0xDEADBEEF
  MOVS r0, #0
  main_loop:
    ADDS r0, r0, #1
    B    main_loop
.size reset_handler, .-reset_handler
