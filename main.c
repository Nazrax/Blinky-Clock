#include "globals.h"
#include "display.h"
#include "clock.h"

void init(void);

void init() {
  CLKPR = _BV(CLKPCE); // Enable prescaler change
  CLKPR = 0; // No prescaler = run at 8 Mhz

  display_init();
}

int main(void) {
  init();

  clock_init();

  display_init();
  display_show();

  for(;;) {
    toDisplay[0] = (clock.seconds / 10) % 10;
    toDisplay[1] = clock.seconds % 10; 
    toDisplay[2] = (clock.subseconds / 10) % 10;
    toDisplay[3] = (clock.subseconds % 10);
    toDisplay[4] = (clock.seconds % 2) ? 2 : 15;
  }
}

