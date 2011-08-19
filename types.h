#ifndef _TYPES_H_
#define _TYPES_H_

typedef enum {false, true} bool_t;
typedef enum { DOWN, UP } button_state_t;

typedef struct {
  unsigned int subseconds:10;
  unsigned char seconds:6;
  unsigned char minutes:6;
  unsigned char hours:5;
} clock_t;

typedef struct {
  uint32_t update_time, down_time; // 64 bits
  button_state_t current, old, new; // 16 bits
  uint8_t pin; // 8 bits
} button_t; // 88 bits / 11 bytes


#endif
