#include "globals.h"
#include "display.h"

#include <avr/interrupt.h>

static volatile uint8_t * segment_ports[] = { &DDRD, &DDRD, &DDRC, &DDRC, &DDRC, &DDRB, &DDRD };
static uint8_t segment_pins[] = { 0, 2, 5, 2, 0, 3, 1 };

#define segment_on(n) (*segment_ports[n] |= _BV(segment_pins[n]))
#define segment_off(n) (*segment_ports[n] &= ~(_BV(segment_pins[n])))
#define set_segment(n, v) v ? segment_on(n) : segment_off(n)

uint8_t seven_seg_digits[25][7] = { 
  { 1,1,1,1,1,1,0 }, // 0
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
  { 1,0,0,1,1,1,1 }, // E 13
  { 0,1,1,1,1,0,0 }, // J 14
  { 1,0,1,0,1,1,1 }, // K 15
  { 0,0,0,1,1,1,0 }, // L 16
  { 1,0,1,0,1,0,1 }, // M 17
  { 0,0,1,0,1,0,1 }, // n 18
  { 1,1,0,0,1,1,1 }, // P 19
  { 0,0,0,0,1,0,1 }, // r 20
  { 1,0,1,1,0,1,1 }, // S 21
  { 1,1,0,0,0,0,0 }, // colon 22
  { 0,0,0,0,0,0,0 }, // blank 23
  { 0,0,0,0,0,0,1 }  // minus 24
};
  

static uint8_t current_digit = 0;
uint8_t toDisplay[] = {8,8,8,8,8};

void display_blank(void);

void display_init() {
  // Put digit pins into output mode
  DDRB = _BV(DDB5) | _BV(DDB4); // Digits 4 (SCK), 5 (MISO)
  DDRC = _BV(DDC4) | _BV(DDC3)  | _BV(DDC1); // Digits 1, 2, L
}

void display_show() {
  TCCR1A = 0; // Normal mode (Can't get CTC to work right, so reset TCNT1 in the interrupt)
  TCCR1B |= _BV(CS12) | _BV(CS10); // Clock / 1024
  OCR1A = 20; // Fire about 40 times / second (@8Mhz)
  TIMSK1 |= _BV(OCIE1A); // Enable CTC interrupt for OCRaA
}

void display_blank() {
  int i;
  PORTB &= ~(_BV(PORTB5) | _BV(PORTB4));
  PORTC &= ~(_BV(PORTC4) | _BV(PORTC3) | _BV(PORTC1));

  for(i=0; i<7; i++) {
    set_segment(i, seven_seg_digits[CHAR_BLANK][i]);
  }
}

void display_hide() {
  // Stop timer
  TCCR1B = 0;

  display_blank();
}

ISR(TIMER1_COMPA_vect) {
  int i;
  TCNT1 = 0;
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
