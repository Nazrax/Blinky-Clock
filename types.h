#ifndef _TYPES_H_
#define _TYPES_H_

typedef enum {false, true} bool_t;
typedef enum { UP, DOWN } button_state_t;

typedef struct {
  unsigned int subseconds:10;
  unsigned char seconds:6;
  unsigned char minutes:6;
  unsigned char hours:5;
} clock_t;

typedef struct {
  uint32_t update_time, down_time;
  button_state_t current, old, new;
  uint8_t pin;
  volatile uint8_t* port;
} button_t;

#endif
