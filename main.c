#include "globals.h"
#include "display.h"
#include "clock.h"
#include "buttons.h"

#include <avr/interrupt.h>
#include <avr/sleep.h>

void init(void);
void handle_buttons(void);
void handle_display(void);
void handle_display_setting(void);
void handle_display_modename(void);
void handle_alarm(void);

static mode_t mode = mode_off;
static uint32_t last_mode_switch_ticks = 0;
static uint16_t alarm_time = 0;
static uint16_t nap_time = 0;

void init() {
  CLKPR = _BV(CLKPCE); // Enable prescaler change
  CLKPR = 0; // No prescaler = run at 8 Mhz

  // What's this for?
  DDRD |= _BV(DDD6);

  // Configure pull-ups for buttons
  PORTB |= _BV(PORTB0);
  PORTD |= _BV(PORTD3) | _BV(PORTD5) | _BV(PORTD7);

  // Configure button structure
  buttons[0].pin = PIND7; // Up
  buttons[0].port = &PIND;
  buttons[1].pin = PINB0; // Down
  buttons[1].port = &PINB;
  buttons[2].pin = PIND3; // Mode
  buttons[2].port = &PIND;
  buttons[3].pin = PIND5; // Top
  buttons[3].port = &PIND;

  display_init();
}

// 0: Up
// 1: Down
// 2: Mode
// 3: Top
void handle_buttons() {
  if (!multiPress) {
    int8_t delta = 0;
    
    if (longPress) { // User is holding down a button
      if (clock_ticked) { // Quickly move up/down
        if (buttons[0].current) { // Up
          delta = 1;
        } else if (buttons[1].current) { // Down
          delta = -1;
        }
      }
    } else { // Not long press
      if (pressed(&buttons[0])) { // Up
        delta = 1;
      } else if (pressed(&buttons[1])) { // Down
        delta = -1;
      } else if (pressed(&buttons[2])) { // Mode
        if (mode >= mode_nap) {
          mode = mode_clock;
        } else {
          mode++;
        }
      } else if (pressed(&buttons[3])) { // Top
        // TODO: Set or clear alarm
      }
    }

    if (delta != 0) {
      if (mode == mode_clock) {
        clock.minutes += delta;
        if (clock.minutes > 59) {
          clock.minutes = 0;
          clock.hours++;
          if (clock.hours > 23) {
            clock.hours = 0;
          }
        } else if (clock.minutes <0) {
          clock.minutes = 59;
          clock.hours--;
          if (clock.hours < 0) {
            clock.hours = 23;
          }
        }
      } else if (mode == mode_alarm) {
        alarm_time++;
        if (alarm_time >= 60 * 24) {
          alarm_time = 0;
        } else if (alarm_time < 0) {
          alarm_time = 60 * 24 - 1;
        }
      } else if (mode == mode_nap) {
        nap_time++;
        if (nap_time >= 60 * 24) {
          nap_time = 0;
        } else if (nap_time < 0) {
          nap_time = 60 * 24 - 1;
        }
      }
    }
  }
}

void handle_display_modename() {
  toDisplay[4] = CHAR_BLANK;
  if (mode == mode_clock) {
    toDisplay[0] = CHAR_C;
    toDisplay[1] = CHAR_L;
    toDisplay[2] = CHAR_C;
    toDisplay[3] = CHAR_K;
  } else if (mode == mode_alarm) {
    toDisplay[0] = CHAR_A;
    toDisplay[1] = CHAR_L;
    toDisplay[2] = CHAR_R;
    toDisplay[3] = CHAR_M;
  } else if (mode == mode_nap) {
    toDisplay[0] = CHAR_N;
    toDisplay[1] = CHAR_A;
    toDisplay[2] = CHAR_P;
    toDisplay[3] = CHAR_BLANK;
  } else if (mode == mode_seconds) {
    toDisplay[0] = CHAR_S;
    toDisplay[1] = CHAR_E;
    toDisplay[2] = CHAR_C;
    toDisplay[3] = CHAR_S;
  } else if (mode == mode_adjustment) {
    toDisplay[0] = CHAR_A;
    toDisplay[1] = CHAR_D;
    toDisplay[2] = CHAR_J;
    toDisplay[3] = CHAR_BLANK;
  }
}

void handle_display_setting() {
  if (mode == mode_clock) {
    toDisplay[0] = (clock.hours / 10) % 10;
    toDisplay[1] = clock.hours % 10; 
    toDisplay[2] = (clock.minutes / 10) % 10;
    toDisplay[3] = (clock.minutes % 10);
    toDisplay[4] = (clock.seconds % 2 == 0) ? CHAR_BLANK : CHAR_COLON;
  } else if (mode == mode_alarm) {
    uint8_t alarm_minutes = alarm_time % 60;
    uint8_t alarm_hours = alarm_time / 60;
    toDisplay[0] = (alarm_hours / 10) % 10;
    toDisplay[1] = alarm_hours % 10; 
    toDisplay[2] = (alarm_minutes / 10) % 10;
    toDisplay[3] = (alarm_minutes % 10);
    toDisplay[4] = CHAR_BLANK;
  } else if (mode == mode_nap) {
    uint8_t nap_minutes = nap_time % 60;
    uint8_t nap_hours = nap_time / 60;
    toDisplay[0] = (nap_hours / 10) % 10;
    toDisplay[1] = nap_hours % 10; 
    toDisplay[2] = (nap_minutes / 10) % 10;
    toDisplay[3] = (nap_minutes % 10);
    toDisplay[4] = CHAR_BLANK;
  } else if (mode == mode_seconds) {
    toDisplay[0] = (clock.seconds / 10) % 10;
    toDisplay[1] = clock.seconds % 10; 
    toDisplay[2] = (clock.subseconds / 10) % 10;
    toDisplay[3] = (clock.subseconds % 10);
    toDisplay[4] = (clock.seconds % 2 == 0) ? CHAR_BLANK : CHAR_COLON;
  } else if (mode == mode_adjustment) {
    toDisplay[0] = (adjustment < 0) ? CHAR_MINUS : CHAR_BLANK;
    toDisplay[1] = (adjustment / 100) % 10;
    toDisplay[2] = (adjustment / 10) % 10;
    toDisplay[3] = adjustment % 10;
    toDisplay[4] = CHAR_BLANK;
  }
}

void handle_display() {
  if (mode == mode_off) {
    // Do nothing
  } else {
    uint32_t ticks_since_mode_change = clock_ticks - last_mode_switch_ticks;
    uint32_t ticks_since_button = clock_ticks - last_button_press_ticks;

    if (ticks_since_button > TICKS_PER_SECOND * 90) {
      display_hide();
    } else if (ticks_since_mode_change > TICKS_PER_SECOND) {
      handle_display_setting();
    } else {
      handle_display_modename();
   }
  }
}

void handle_alarm() {
}

int main(void) {
  init();

  clock_init();
  display_show();

  sei();

  for(;;) {
    clock_update();
    buttons_update();

    handle_buttons();
    handle_display();
    handle_alarm();

    buttons_age();
    clock_ticked = false;

    // Check display timeout

    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();
    sleep_cpu();
    sleep_disable();
  }
}

