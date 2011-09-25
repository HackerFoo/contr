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

#define R_A 0
#define R_B 1

void main() {
  word *start;
  uint32_t a, b;
  static int i = 0;
  static int j = 0;

  GET_SP(start);
  BLOCK(a_block) {
    a = r_uint32_t(start, &a_block, R_A, 0);
    printf("a_block run\n");
  }
  BLOCK(b_block) {
    b = r_uint32_t(start, &b_block, R_B, 0);
    printf("b_block run\n");
  }

  printf("a = %d, b = %d\n", a, b);

  if(i < j) {
    r_call_uint32_t(R_A, ++i);
  }
  if(j < 5) {
    r_call_uint32_t(R_B, ++j);
  }
}
