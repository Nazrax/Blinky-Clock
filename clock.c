#include "globals.h"
#include "clock.h"
#include "alarm.h"

#include <avr/interrupt.h>

volatile uint32_t clock_ticks;
clock_t clock;

bool_t clock_ticked = false;
int8_t adjustment = 0;

// The clock uses timer 2 in async mode
void clock_init() {
  ASSR |= _BV(AS2); // Enable async clock
  TCCR2A |= _BV(WGM21); // CTC mode
  OCR2A = 3; // Fire every 4th clock tick (16 times / second)
  TCCR2B |= _BV(CS22) | _BV(CS21);// | _BV(CS20); // Clock / 1024
  TIMSK2 |= _BV(OCIE2A); // Enable CTC interrupt for OCR2A
}

void clock_update() {
  while (clock.subseconds > (TICKS_PER_SECOND-1)) {
    clock.subseconds -= TICKS_PER_SECOND;
    clock.seconds++;
    
    if (clock.seconds > 59) {
      clock.seconds = 0;
      clock.minutes++;
      
      if (clock.minutes > 59) {
        clock.minutes = 0;
        clock.hours++;

        if (clock.hours > 23) {
          clock.hours = 0;
          clock.subseconds += adjustment;
        }
      }

      alarm_check();
    }
  }
}

// The clock
ISR(TIMER2_COMPA_vect) {
  clock_ticks++;
  clock.subseconds++;
  clock_ticked = true;
}
