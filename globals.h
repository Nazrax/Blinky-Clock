#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <avr/io.h>
#include "types.h"

#define F_CPU 8000000

#define BUTTON_COUNT 4
#define TICKS_PER_SECOND 16

#include <avr/io.h>
#include <util/delay.h>


// Clock stuff
extern clock_t clock;
extern volatile uint32_t clock_ticks;

// Buttons
button_t buttons[BUTTON_COUNT];
bool_t longPress, multiPress;


#endif
