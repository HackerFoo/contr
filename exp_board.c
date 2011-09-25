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

#include <msp430fr5739.h>
#include "contr.h"
#include "stdlib.h"

#define F_CPU 8000000

/* PORT 4 */
#define BUTTON0 0x01
#define BUTTON1 0x02

enum reactives {
  R_TIME = 0,
  R_BUTTON0,
  R_BUTTON1,
  R_ACC_X,
  R_ACC_Y,
  R_ACC_Z
};

void led(uint8_t v) {
  P3OUT = (P3OUT & 0x0F) | (v & 0xF0);
  PJOUT = (PJOUT & 0xF0) | (v & 0x0F);
}

void led_set(uint8_t v) {
  P3OUT |= v & 0xF0;
  PJOUT |= v & 0x0F;
}

void led_toggle(uint8_t v) {
  P3OUT ^= v & 0xF0;
  PJOUT ^= v & 0x0F;
}

void led_clr(uint8_t v) {
  P3OUT &= ~(v & 0xF0);
  PJOUT &= ~(v & 0x0F);
}

static uint16_t timer_count = 0;
__attribute__((interrupt(TIMER0_A0_VECTOR)))
void timer_isr() {
  r_call_uint16_t(R_TIME, ++timer_count);
}

__attribute__((interrupt(PORT4_VECTOR)))
void button_isr() {
  switch(P4IV) {
    case P4IV_P4IFG0:
      P4IFG &= ~BUTTON0;
      P4IES ^= BUTTON0;
      r_call_uint16_t(R_BUTTON0, !(P4IN & BUTTON0));
      break;
    case P4IV_P4IFG1:
      P4IFG &= ~BUTTON1;
      P4IES ^= BUTTON1;
      r_call_uint16_t(R_BUTTON1, !(P4IN & BUTTON1));
      break;
  }
}

__attribute__((interrupt(ADC10_VECTOR)))
void adc10_isr() {
    uint16_t mem = ADC10MEM0;
    uint8_t channel = ADC10MCTL0 & ADC10INCH_15;
    switch(channel) {
      case 13: r_call_uint16_t(R_ACC_Z, mem); break;
      case 12: r_call_uint16_t(R_ACC_Y, mem); break;
      case 11: r_call_uint16_t(R_ACC_X, mem); break;
      default: /* reset sequence */
               ADC10CTL1 &= ~ADC10CONSEQ_3;
               ADC10CTL0 &= ~ADC10ENC;
               ADC10CTL1 |= ADC10CONSEQ_3;
               ADC10CTL0 |= ADC10ENC;
        break;
    }
}

#define ACC_X_PIN (1<<0)
#define ACC_Y_PIN (1<<1)
#define ACC_Z_PIN (1<<2)
#define ACC_PWR_PIN (1<<7)

void init_accel() {
  P3SEL0 |= ACC_X_PIN | ACC_Y_PIN | ACC_Z_PIN;
  P3SEL1 |= ACC_X_PIN | ACC_Y_PIN | ACC_Z_PIN;
  P3OUT &= ~(ACC_X_PIN | ACC_Y_PIN | ACC_Z_PIN);
  P3DIR &= ~(ACC_X_PIN | ACC_Y_PIN | ACC_Z_PIN);
  P3REN |= ACC_X_PIN | ACC_Y_PIN | ACC_Z_PIN;
  P2DIR |= ACC_PWR_PIN;
  P2OUT |= ACC_PWR_PIN;

  ADC10CTL0 &= ~ADC10ENC;
  ADC10CTL0 = ADC10ON | ADC10SHT_5 | ADC10MSC;
  ADC10CTL1 = ADC10SHS_2 | ADC10CONSEQ_3 | ADC10SSEL_0;
  ADC10CTL2 = ADC10RES;
  ADC10MCTL0 = ADC10SREF_0 | ADC10INCH_14;
  ADC10IV = 0x00;
  ADC10IE |= ADC10IE0;
  ADC10CTL0 |= ADC10ENC;

  // trigger ADC @ 100Hz
  TB0CCTL0 = CCIS_2 | CM_0 | OUTMOD_3;
  TB0CTL = MC_1 | ID_3 | TBSSEL_2;
  TB0CCR0 = 1250;
}

void init() {
  // stop watchdog timer
  WDTCTL = WDTPW | WDTHOLD;

  // LEDs
  P3DIR = 0xF0;
  P3REN = 0x0F;
  P3OUT = 0x00;
  PJDIR = 0x0F;
  PJREN = 0xF0;
  P3OUT = 0x00;

  // set timer for 10Hz
  TA0CCTL0 = CCIE;
  TA0CTL = MC_1 | ID_3 | TASSEL_2;
  TA0CCR0 = 12500;

  // Set up switches
  P4OUT = BUTTON0 | BUTTON1;
  P4DIR = 0;
  P4REN = BUTTON0 | BUTTON1;
  P4IES = BUTTON0 | BUTTON1;
  P4IE = BUTTON0 | BUTTON1;
  P4IFG = 0;

  init_accel();

  __eint();
}

int main() {
  word *start;
  static int16_t dx = 0;
  static uint16_t dt = 0;
  static uint16_t xn_1 = 0;
  static uint16_t timen_1 = 0;
  static int16_t v = 0;
  static int16_t pos = 0;
  static uint16_t cal_start = 0;
  static uint16_t cal_x = 512;
  init();

  GET_SP(start);

  uint16_t time = r_uint16_t(start, NULL, R_TIME, 0);
  uint16_t x = r_uint16_t(start, NULL, R_ACC_X, 0);
  uint16_t button0 = r_uint16_t(start, NULL, R_BUTTON0, 0);
  uint16_t button1 = r_uint16_t(start, NULL, R_BUTTON1, 0);
  if(time - cal_start < 50) {
    cal_x = (3*cal_x + x) >> 2;
    led(time & 2 ? 0xff : 0);
  } else if(button0) {
    cal_start = time;
    cal_x = x;
  } else {
    if(button1) pos > 0 ? -0x2000 : 0x2000;
    int acc = x - cal_x;
    if(abs(x) < 20) x = 0;
    v += - (v >> 5) /* friction */
         + (acc << 4); /* force */
    pos += v;
    /* bounce */
    if(pos > 0x3fff) { pos = 0x3fff; v = -v; }
    else if(pos < -0x4000) { pos = -0x4000; v = -v; }

    led(1 << ((pos + 0x4000) >> 12));
  }
  
  LPM0;
}
