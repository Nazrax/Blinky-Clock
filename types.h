#ifndef _TYPES_H_
#define _TYPES_H_

typedef enum {false, true} bool_t;
typedef enum { UP, DOWN } button_state_t;
typedef enum { mode_off, mode_clock, mode_alarm, mode_nap, mode_seconds, mode_adjustment } mode_t;
typedef enum { status_none, status_success, status_error, status_alarm } status_t;

typedef struct {
  int16_t subseconds;
  int8_t seconds;
  int8_t minutes;
  int8_t hours;
} clock_t;

typedef struct {
  uint32_t update_time, down_time;
  button_state_t current, old, new;
  uint8_t pin;
  volatile uint8_t* port;
} button_t;

#endif
