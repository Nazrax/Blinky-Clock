#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <avr/io.h>
#include "types.h"

#define F_CPU 8000000

#define BUTTON_COUNT 4
#define TICKS_PER_SECOND 32

#define SWEEP_SPEED 1

#include <avr/io.h>
#include <util/delay.h>

extern int16_t alarm_time;
extern int16_t nap_time;
extern bool_t nap_enabled;
extern bool_t alarm_active;


#endif
