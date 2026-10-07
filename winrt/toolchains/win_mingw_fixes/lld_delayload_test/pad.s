  .syntax unified
  .thumb
  .text
  .globl pad_fn
  .def pad_fn; .scl 2; .type 32; .endef
pad_fn:
  bx lr
  .space 20000000
