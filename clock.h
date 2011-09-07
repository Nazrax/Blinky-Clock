#ifndef _CLOCK_H_
#define _CLOCK_H_

extern clock_t clock;
extern volatile uint32_t clock_ticks;
extern int8_t adjustment;

extern bool_t clock_ticked;

void clock_init(void);
void clock_update(void);

#endif
