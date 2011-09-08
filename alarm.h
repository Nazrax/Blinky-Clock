#ifndef _ALARM_H_
#define _ALARM_H_

void alarm_init(void);

void alarm_on(void);
void alarm_off(void);
void alarm_sweep(void);
void alarm_check(void);

void led_on(void);
void led_toggle(void);
void led_off(void);
int led_status(void);

#endif // _ALARM_H_
