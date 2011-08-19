#include "globals.h"
#include "display.h"
#include "clock.h"
#include "buttons.h"

#include <avr/interrupt.h>
#include <avr/sleep.h>

void init(void);

void init() {
  CLKPR = _BV(CLKPCE); // Enable prescaler change
  CLKPR = 0; // No prescaler = run at 8 Mhz

  DDRD |= _BV(DDD6);


  // Configure buttons
  PORTB |= _BV(PORTB0);
  PORTD |= _BV(PORTD3) | _BV(PORTD5) | _BV(PORTD7);

  buttons[0].pin = PIND7;
  buttons[0].port = &PIND;
  buttons[1].pin = PINB0;
  buttons[1].port = &PINB;
  buttons[2].pin = PIND3;
  buttons[2].port = &PIND;
  buttons[3].pin = PIND5;
  buttons[3].port = &PIND;

  display_init();
}

int main(void) {
  init();

  clock_init();
  display_show();

  sei();

  for(;;) {
    clock_update();
    buttons_update();

    if (pressed(&buttons[0])) {
      PORTD ^= _BV(PORTD6);
    }

      /*
    if (longPress) {
      PORTD |= _BV(PORTD6);
    } else {
      PORTD &= ~(_BV(PORTD6));
    }
      */
    /*
    toDisplay[0] = (clock.seconds / 10) % 10;
    toDisplay[1] = clock.seconds % 10; 
    toDisplay[2] = (clock.subseconds / 10) % 10;
    toDisplay[3] = (clock.subseconds % 10);
    */

    toDisplay[0] = buttons[0].current;
    toDisplay[1] = buttons[1].current;
    toDisplay[2] = buttons[2].current;
    toDisplay[3] = buttons[3].current;
    buttons_age();

    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();
    sleep_cpu();
    sleep_disable();
  }
}

