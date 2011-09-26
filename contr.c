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

#include <stdio.h>
#include "contr.h"

#ifdef DEBUG
#define dump(x) _dump(x, sizeof(x))
void _dump(void *x, int n) {
  int i;
  for(i = 0; i < n; i++)
    printf("%.2hhx ", ((unsigned char *)x)[i]);
  printf("\n");
}
#endif

static cont contexts[MAX_CONTEXTS] = {0};
static r_active = MAX_CONTEXTS;

void save_stack(cont *c, word *start, word *here) {
  int n = start - here;
  /* fail if there's not enough space */
  if(n > MAX_STACK_SIZE) {
#ifdef DEBUG
    printf("not enough stack space (n = %d); please increase MAX_STACK_SIZE\n", n);
#endif
    return;
  }
  c->n = n;
  for(int i = 0; i < n; i++) c->stack[i] = start[i-n];
}

#define REACTIVE_INSTANCE(type)                                              \
type r_##type(word *start, jmp_buf *skip, int id, type x) {                  \
  word *here;                                                                \
  if(!setjmp(contexts[id].registers)) { /* if we didn't longjmp here  */     \
    if(contexts[id].n) { /* the context is set */                            \
      if(skip != NULL && r_active != id) longjmp(*skip, TRUE);               \
    } else { /* the context is not set */                                    \
      contexts[id].val = x;                                                  \
      contexts[id].start = start;                                            \
      GET_SP(here);                                                          \
      save_stack(&contexts[id], start, here);                                \
    }                                                                        \
  }                                                                          \
                                                                             \
  return contexts[id].val;                                                   \
}                                                                            \
                                                                             \
void r_call_##type(int id, type x) {                                         \
  word *here;                                                                \
  if(!contexts[id].n) return;                                                \
  contexts[id].val = x;                                                      \
  here = contexts[id].start - contexts[id].n;                                \
  restore_stack(contexts[id], here);                                         \
  register int rid = id; /* local variables not available after SET_SP() */  \
  SET_SP(here); /* no local variables from now on */                         \
  r_active = id;                                                             \
  longjmp(contexts[rid].registers,1);                                        \
}

/* instantiate reactive functions for each type */
REACTIVE_INSTANCE(int8_t)
REACTIVE_INSTANCE(uint8_t)
REACTIVE_INSTANCE(int16_t)
REACTIVE_INSTANCE(uint16_t)
#ifndef __MSP430__
REACTIVE_INSTANCE(int32_t)
REACTIVE_INSTANCE(uint32_t)
REACTIVE_INSTANCE(float)
#ifdef __X86_64__
REACTIVE_INSTANCE(int64_t)
REACTIVE_INSTANCE(uint64_t)
REACTIVE_INSTANCE(double)
#endif
#endif
