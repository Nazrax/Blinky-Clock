#include "globals.h"
#include "display.h"
#include "clock.h"

#include <avr/interrupt.h>

void init(void);

void init() {
  CLKPR = _BV(CLKPCE); // Enable prescaler change
  CLKPR = 0; // No prescaler = run at 8 Mhz

  DDRD |= _BV(DDD6);

  display_init();
}

int main(void) {
  init();

  clock_init();
  display_show();

  sei();

  for(;;) {
    clock_update();
    toDisplay[0] = (clock.seconds / 10) % 10;
    toDisplay[1] = clock.seconds % 10; 
    toDisplay[2] = (clock.subseconds / 10) % 10;
    toDisplay[3] = (clock.subseconds % 10);
    toDisplay[4] = (clock.seconds % 2) ? 2 : 15;
  }
}

