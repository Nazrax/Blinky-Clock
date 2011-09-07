#ifndef _BUTTONS_H_
#define _BUTTONS_H_

extern uint32_t last_button_press_ticks;

void buttons_age(void);
void buttons_update(void);
bool_t pressed(button_t*);

extern button_t buttons[BUTTON_COUNT];
extern bool_t longPress, multiPress;

#endif
