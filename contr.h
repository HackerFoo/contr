/* Copyright 2011 Dustin DeWeese
 *
 * This file is part of contr.
 *
 * contr is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * contr is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with contr.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __CONT__
#define __CONT__

#include <stdint.h>
#include <setjmp.h>

typedef enum bool_e {
  FALSE = 0,
  TRUE = 1
} bool;

#define NULL 0

typedef int word;

#ifdef __X86_64__
  #define GET_SP(x) asm volatile("mov %%rsp, %0" : "=g" (x));
  #define SET_SP(x) asm volatile("mov %0, %%rsp" : : "g" (x));
#endif

#ifdef __X86__
  #define GET_SP(x) asm volatile("mov %%esp, %0" : "=g" (x));
  #define SET_SP(x) asm volatile("mov %0, %%esp" : : "g" (x));
#endif

#ifdef __MSP430__
  #define GET_SP(x) asm volatile("mov R1, %0" : "=g" (x));
  #define SET_SP(x) asm volatile("mov %0, R1" : : "g" (x));
  #ifndef MAX_STACK_SIZE
    #define MAX_STACK_SIZE 6
  #endif
  #ifndef MAX_CONTEXTS
    #define MAX_CONTEXTS 2
  #endif
#endif

#ifndef MAX_STACK_SIZE
  #define MAX_STACK_SIZE 20
#endif
#ifndef MAX_CONTEXTS
  #define MAX_CONTEXTS 10
#endif

/* Continuation datastructure. */
typedef struct cont {
  jmp_buf registers;
  int n;
  intptr_t val;
  word *start;
  word stack[MAX_STACK_SIZE];
} cont;

void save_stack(cont *c, word *start, word *here);

/* restore_stack is a macro to avoid messing up the stack */
/* memcpy fails here under optimization */
#define restore_stack(c, here)  \
do {                            \
  for(int i = 0; i < c.n; i++)  \
    here[i] = c.stack[i];       \
} while(0);

#define BLOCK(name) \
jmp_buf name; \
if(!setjmp(name))

#define REACTIVE_DECLARATION(type)               \
type r_##type(word *start, jmp_buf *skip, int id, type x);      \
void r_call_##type(int id, type x);

/* declare reactive types here */
REACTIVE_DECLARATION(int8_t)
REACTIVE_DECLARATION(uint8_t)
REACTIVE_DECLARATION(int16_t)
REACTIVE_DECLARATION(uint16_t)
#ifndef __MSP430__
REACTIVE_DECLARATION(int32_t)
REACTIVE_DECLARATION(uint32_t)
REACTIVE_DECLARATION(float)
#ifdef __X86_64__
REACTIVE_DECLARATION(int64_t)
REACTIVE_DECLARATION(uint64_t)
REACTIVE_DECLARATION(double)
#endif
#endif

#endif
