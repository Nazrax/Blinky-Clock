#include "globals.h"
#include "alarm.h"
#include "clock.h"

#include <avr/io.h>

static int16_t brightness = 0;
static int8_t direction = 1;

int led_status(void) {
  return PORTD & _BV(PORTD6);
}

void alarm_check() {
  uint8_t alarm_minutes = alarm_time % 60;
  uint8_t alarm_hours = alarm_time / 60;

  if ((alarm_minutes == clock.minutes && alarm_hours == clock.hours) ||
      (nap_enabled && nap_time < clock_ticks)) {
    alarm_on();
  } else if ((status == status_alarm) && clock_ticks > alarm_activated_at + (uint32_t)TICKS_PER_SECOND * 60 * 90) {
    alarm_off();
  }
}

void alarm_on(void) {
  if (!(status == status_alarm)) {
    alarm_activated_at = clock_ticks;
  }

  brightness = 0;
  direction = 1;

  TCCR0A = _BV(COM0A1) | _BV(WGM01) | _BV(WGM00); // Fast PWM, non-inverting mode
  TCCR0B = _BV(CS00); // F_CPU
  OCR0A = brightness;

  status = status_alarm;
}

void alarm_off(void) {
  TCCR0A = 0;
  TCCR0B = 0;
  TCNT0 = 0;
  led_off();
  status = status_none;
}

void alarm_sweep(void) {
  if (status == status_alarm) {
    brightness += direction * SWEEP_SPEED;

    if (brightness < 1) {
      brightness = 1;
      direction = 1;
    } else if (brightness > 254) {
      brightness = 254;
      direction = -1;
    }

    OCR0A = brightness;
  }
}

void led_on(void) {
  PORTD |= _BV(PORTD6); // Turn on LED
}

void led_toggle(void) {
  PORTD ^= _BV(PORTD6); // Toggle LED
}

void led_off(void) {
  PORTD &= ~(_BV(PORTD6)); // Turn off LED
}

void alarm_init(void) {
  DDRD = _BV(DDD6); // LED is an output
  led_on();
}
