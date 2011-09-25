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

#include <msp430g2231.h>
#include "contr.h"

#define LED1 0x01
#define LED2 0x40
#define SWITCH 0x08
#define PUMP 0x80

enum reactives {
  R_TIME = 0,
  R_BUTTON
};

void off(uint8_t x)    { P1OUT &= ~x; }
void on(uint8_t x)     { P1OUT |=  x; }
void toggle(uint8_t x) { P1OUT ^=  x; }

__attribute__((interrupt(TIMERA0_VECTOR)))
void timer_isr() {
  static uint16_t timer_cnt = 0;
  P1IES = (P1IES & ~SWITCH) | (P1IN & SWITCH);
  r_call_uint16_t(R_TIME, timer_cnt++);
}

__attribute__((interrupt(PORT1_VECTOR)))
void button_isr() {
  P1IES ^= SWITCH;  /* catch the other edge */
  P1IFG &= ~SWITCH; /* clear the interrupt flag */
  r_call_uint16_t(R_BUTTON, !(P1IN & SWITCH));
}

void init() {

  // disable watchdog
  WDTCTL = WDTPW | WDTHOLD;

  // LEDs, switch, and pump
  P1DIR = LED1 | LED2 | PUMP;
  P1OUT = SWITCH | PUMP;
  P1REN = ~P1DIR;
  P1IES = SWITCH;
  P1IE = SWITCH;
  P1IFG = 0;

  // timer, ~8.5Hz, ~512 ticks per minute
  TACCTL0 = CCIE;
  TACTL = MC_1 | ID_3 | TASSEL_2;
  TACCR0 = 16845; /* DCO = ~1.15Mhz */

  __eint();
}

void main() {
  uint16_t time, button;
  static uint16_t ts = 0, mode = 1;
  uint16_t pump;
  word *start;

  init();
  GET_SP(start);

  time = r_uint16_t(start, NULL, R_TIME, 0);
  button = r_uint16_t(start, NULL, R_BUTTON, 0);
  if(button) {
    /* debounce mode selection */
    int t = time - ts;
    if(t == 2 && ++mode > 4) mode = 0;

    /* blink back the mode */
    if(t & 2) {
      if(t >> 2 <= mode) on(LED1);
    } else off(LED1);
  } else {
    off(LED1);
    ts = time;
  }

  /* control pump */
  pump = (time >> 9) % 5 <= mode;
  if(pump) on(PUMP);
  else     off(PUMP);

  /* status LED */
  if(time & (pump ? 1 : 8)) on(LED2);
  else off(LED2);

  LPM0; /* Turn off the CPU */
}
