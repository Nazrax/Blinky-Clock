#include "globals.h"
#include "display.h"
#include "clock.h"
#include "buttons.h"
#include "alarm.h"

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>

void init(void);
void handle_buttons(void);
void handle_display(void);
void handle_display_setting(void);
void handle_display_modename(void);
void handle_mode_switch(void);
void handle_led(void);
void update_eeprom(void);
void fetch_eeprom(void);

static mode_t mode = mode_off;
static uint32_t last_mode_switch_ticks = 0;
static int16_t nap_duration = 0;
static bool_t dirty;

int16_t alarm_time = 0;
uint32_t nap_time = 0;
bool_t nap_enabled = false;
uint32_t alarm_activated_at = 0;
status_t status = status_none;
uint32_t status_ticks = 0;

static uint8_t EEMEM eeprom_adjustment = 0;
static uint16_t EEMEM eeprom_nap_duration = 0;
static uint16_t EEMEM eeprom_alarm_time = 0;

void update_eeprom() {
  if (dirty) {
    if (mode == mode_alarm) {
      eeprom_write_word(&eeprom_alarm_time, alarm_time);
    } else if (mode == mode_nap) {
      eeprom_write_word(&eeprom_nap_duration, nap_duration);
    } else if (mode == mode_adjustment) {
      eeprom_write_byte(&eeprom_adjustment, adjustment);
    }
    dirty = false;
  }
}

void fetch_eeprom() {
  alarm_time = eeprom_read_word(&eeprom_alarm_time);
  nap_duration = eeprom_read_word(&eeprom_nap_duration);
  adjustment = eeprom_read_byte(&eeprom_adjustment);
}

void init() {
  CLKPR = _BV(CLKPCE); // Enable prescaler change
  CLKPR = 0; // No prescaler = run at 8 Mhz

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
  fetch_eeprom();
}

void handle_mode_switch() {
  update_eeprom();
  last_mode_switch_ticks = clock_ticks;
  display_show();
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
        handle_mode_switch();

        if (mode >= mode_nap) {
          mode = mode_clock;
        } else {
          mode++;
        }
      } else if (pressed(&buttons[3])) { // Top
        if (status == status_alarm) {
          nap_enabled = false;
          alarm_off();
        } else {
          nap_enabled = true;
          nap_time = clock_ticks + nap_duration * 60 * TICKS_PER_SECOND;
        }
      }
    }

    if (delta != 0) {
      if (mode == mode_clock) {
        cli();
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
        sei();
      } else if (mode == mode_alarm) {
        dirty = true;
        alarm_time += delta;
        if (alarm_time >= 60 * 24) {
          alarm_time = 0;
        } else if (alarm_time < 0) {
          alarm_time = 60 * 24 - 1;
        }
      } else if (mode == mode_nap) {
        dirty = true;
        nap_duration += delta;
        if (nap_duration >= 60 * 24) {
          nap_duration = 0;
        } else if (nap_duration < 0) {
          nap_duration = 60 * 24 - 1;
        }
      } else if (mode == mode_adjustment) {
        dirty = true;
        adjustment += delta;
      } else if (mode == mode_seconds) {
        cli();
        clock.subseconds = 0;
        clock.seconds = 0;
        sei();
      }
    }
  } else { // multipress
    if (buttons[2].current) {
      if (pressed(&buttons[0])) {
        handle_mode_switch();
        mode = mode_seconds;
      } else if (pressed(&buttons[1])) {
        handle_mode_switch();
        mode = mode_adjustment;
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
    uint8_t nap_minutes = nap_duration % 60;
    uint8_t nap_hours = nap_duration / 60;
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
      update_eeprom();
      mode = mode_off;
    } else if (ticks_since_mode_change > TICKS_PER_SECOND) {
      handle_display_setting();
    } else {
      handle_display_modename();
   }
  }
}

void handle_led() {
  if (status == status_none) {
    led_off();
  } else if (status == status_success) {
    uint32_t elapsed = clock_ticks - status_ticks;
    if (elapsed < TICKS_PER_SECOND * 2) {
      if (elapsed % (TICKS_PER_SECOND / 2) < (TICKS_PER_SECOND / 4)) {
        led_on();
      } else {
        led_off();
      }
    } else {
      status = status_none;
    }
  } else if (status == status_error) {
    uint32_t elapsed = clock_ticks - status_ticks;
    if (elapsed < TICKS_PER_SECOND * 3) {
      led_on();
    } else {
      status = status_none;
    }
  } else if (status == status_alarm) {
    alarm_sweep();
  }
}

int main(void) {
  init();
  alarm_init();
  clock_init();
  display_init();

  sei();

  for(;;) {
    clock_update();
    buttons_update();

    handle_buttons();
    handle_display();
    handle_led();

    buttons_age();
    clock_ticked = false;

    // Check display timeout

    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();
    sleep_cpu();
    sleep_disable();
  }
}

