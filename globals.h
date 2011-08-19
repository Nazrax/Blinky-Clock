#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <avr/io.h>
#include "types.h"

#define F_CPU 8000000

#include <avr/io.h>
#include <util/delay.h>

#define DELAY 250

// Clock stuff
extern clock_t clock;
extern volatile uint32_t clock_ticks;


#endif
