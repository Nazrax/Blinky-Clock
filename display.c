#include "globals.h"
#include "display.h"

#include <avr/interrupt.h>

static volatile uint8_t * segment_ports[] = { &DDRD, &DDRC, &DDRD, &DDRC, &DDRC, &DDRB, &DDRD };
static uint8_t segment_pins[] = { 1, 5, 2, 2, 0, 3, 0 };

#define segment_on(n) (*segment_ports[n] |= _BV(segment_pins[n]))
#define segment_off(n) (*segment_ports[n] &= ~(_BV(segment_pins[n])))
#define set_segment(n, v) v ? segment_on(n) : segment_off(n)

uint8_t seven_seg_digits[20][7] = { { 1,1,1,1,1,1,0 }, // 0
                                    { 0,1,1,0,0,0,0 }, // 1
                                    { 1,1,0,1,1,0,1 }, // 2
                                    { 1,1,1,1,0,0,1 }, // 3
                                    { 0,1,1,0,0,1,1 }, // 4
                                    { 1,0,1,1,0,1,1 }, // 5
                                    { 1,0,1,1,1,1,1 }, // 6
                                    { 1,1,1,0,0,0,0 }, // 7
                                    { 1,1,1,1,1,1,1 }, // 8
                                    { 1,1,1,0,0,1,1 }, // 9
                                    { 1,1,1,0,1,1,1 }, // A 10
                                    { 1,0,0,1,1,1,0 }, // C 11
                                    { 0,1,1,1,1,0,1 }, // d 12
                                    { 0,1,1,1,1,0,0 }, // J 13
                                    { 1,0,1,0,1,1,1 }, // K 14
                                    { 0,0,0,1,1,1,0 }, // L 15
                                    { 1,0,1,0,1,0,1 }, // M 16
                                    { 0,0,1,0,1,0,1 }, // n 17
                                    { 1,1,0,0,1,1,1 }, // P 18
                                    { 0,0,0,0,1,0,1 }  // r 19
};
  

static uint8_t current_digit = 0;
uint8_t toDisplay[] = {8,8,8,8,8};

void display_blank(void);

void display_init() {
  int j;
  // Put digit pins into output mode
  DDRB = _BV(DDB5) | _BV(DDB4); // Digits 4 (SCK), 5 (MISO)
  DDRC = _BV(DDC4) | _BV(DDC3)  | _BV(DDC1); // Digits 1, 2, L

  PORTC = _BV(PORTC3); // Digit 2
  for(j=0; j<7; j++) {
    set_segment(j, seven_seg_digits[8][j]);
  }
}

void display_show() {
  TCCR0A |= _BV(WGM01); // CTC mode
  TCCR0B |= _BV(CS02) | _BV(CS00); // Clock / 1024
  OCR0A = 20; // Fire about 40 times / second (@8Mhz)
  TIMSK0 |= _BV(OCIE0A); // Enable CTC interrupt for OCR0A
}

void display_blank() {
  PORTB &= ~(_BV(PORTB5) | _BV(PORTB4));
  PORTC &= ~(_BV(PORTC4) | _BV(PORTC3) | _BV(PORTC1));
}

void display_hide() {
  // Stop timer
  TCCR0B = 0;

  display_blank();
}

ISR(TIMER0_COMPA_vect) {
  int i;
  display_blank();

  current_digit++;
  if (current_digit > 4) {
    current_digit = 0;
  }

  if (current_digit == 0) {
    PORTC |= _BV(PORTC4);
  } else if (current_digit == 1) {
    PORTC |= _BV(PORTC3);
  } else if (current_digit == 2) {
    PORTB |= _BV(PORTB5);
  } else if (current_digit == 3) {
    PORTB |= _BV(PORTB4);
  } else if (current_digit == 4) { // L1L2L3
    PORTB |= _BV(PORTC1);
  }

  for(i=0; i<7; i++) {
    set_segment(i, seven_seg_digits[toDisplay[current_digit]][i]);
  }
}
