#ifndef _DISPLAY_H_
#define _DISPLAY_H_

typedef enum {CHAR_0, CHAR_1, CHAR_2, CHAR_3, CHAR_4, CHAR_5, CHAR_6, CHAR_7, CHAR_8, CHAR_9,
              CHAR_A, CHAR_C, CHAR_D, CHAR_E, CHAR_J, CHAR_K, CHAR_L, CHAR_M, CHAR_N, CHAR_P, 
              CHAR_R, CHAR_S, CHAR_COLON, CHAR_BLANK, CHAR_MINUS } char_t;


extern uint8_t toDisplay[5];

void display_init(void);
void display_show(void);
void display_hide(void);

#endif
