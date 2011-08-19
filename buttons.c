#include "globals.h"
#include "buttons.h"


static void _update(button_t *button);

void buttons_age() {
  int i;
  uint8_t pressCount = 0;

  for(i=0; i<BUTTON_COUNT; i++) {
    buttons[i].old = buttons[i].current;
    if (buttons[i].current) {
      pressCount++;
    }
  }

  multiPress = (pressCount > 1);
}

void buttons_update() {
  int i;
  uint8_t pressCount = 0;

  longPress = false;
  
  for(i=0; i<BUTTON_COUNT; i++) {
    _update(&buttons[i]);
    if (buttons[i].current == DOWN) {
      pressCount++;
      uint32_t duration = clock_ticks - buttons[i].update_time;
      if (duration > TICKS_PER_SECOND)
        longPress = true;
    }
  }

  multiPress = (pressCount > 1) || multiPress;
}

bool_t pressed(button_t *button) {
  return (button->old == UP) && (button->current == DOWN);
}

static void _update(button_t *button) {
  button_state_t state = !(*button->port & _BV(button->pin));

  if (state != button->current) {
    if (button->update_time + 1 < clock_ticks) {
      button->old = button->current;
      button->current = state;
      button->update_time = clock_ticks;

      if (state == DOWN) 
        button->down_time = clock_ticks;
    }
  }
}
