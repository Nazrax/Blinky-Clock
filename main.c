#define F_CPU 1000000

#include <avr/io.h>
#include <util/delay.h>

#define DELAY 250

typedef struct
{
  unsigned int bit0:1;
  unsigned int bit1:1;
  unsigned int bit2:1;
  unsigned int bit3:1;
  unsigned int bit4:1;
  unsigned int bit5:1;
  unsigned int bit6:1;
  unsigned int bit7:1;
} _io_reg; 

typedef struct {
  volatile uint8_t * reg;
  uint8_t pin;
} segment_t;

//segment_t segments[7];
volatile uint8_t * segment_ports[] = { &DDRD, &DDRC, &DDRD, &DDRC, &DDRC, &DDRB, &DDRD };
uint8_t segment_pins[] = { 1, 5, 2, 2, 0, 3, 0 };

#define segment_on(n) (*segment_ports[n] |= _BV(segment_pins[n]))
#define segment_off(n) (*segment_ports[n] &= ~(_BV(segment_pins[n])))

#define REGISTER_BIT(...) REGISTER_BIT_(__VA_ARGS__)
#define REGISTER_BIT_(rg,bt) ((volatile _io_reg*)&rg)->bit##bt 

#define segment_0 REGISTER_BIT(DDRD, 1)
#define segment_1 REGISTER_BIT(DDRC, 5)
#define segment_2 REGISTER_BIT(DDRD, 2)
#define segment_3 REGISTER_BIT(DDRC, 2)
#define segment_4 REGISTER_BIT(DDRC, 0)
#define segment_5 REGISTER_BIT(DDRB, 3)
#define segment_6 REGISTER_BIT(DDRD, 0)

//#define set_segment(N, V) segment_##N = V
#define set_segment(N, V) V ? segment_on(N) : segment_off(N)

uint8_t seven_seg_digits[10][7] = { { 1,1,1,1,1,1,0 },  // = 0
                                    { 0,1,1,0,0,0,0 },  // = 1
                                    { 1,1,0,1,1,0,1 },  // = 2
                                    { 1,1,1,1,0,0,1 },  // = 3
                                    { 0,1,1,0,0,1,1 },  // = 4
                                    { 1,0,1,1,0,1,1 },  // = 5
                                    { 1,0,1,1,1,1,1 },  // = 6
                                    { 1,1,1,0,0,0,0 },  // = 7
                                    { 1,1,1,1,1,1,1 },  // = 8
                                    { 1,1,1,0,0,1,1 }   // = 9
};

#define a_on() DDRD |= _BV(DDD1)
#define a_off() DDRD &= ~(_BV(DDD1))

int main(void) {
  int i,j;

  DDRB = _BV(DDB5) | _BV(DDB4); // Digits 4 (SCK), 5 (MISO)
  DDRC = _BV(DDC4) | _BV(DDC3)  | _BV(DDC1); // Digits 1, 2, L

  PORTC = _BV(PORTC4); // Digit 1

  for(;;) {
    for(i=0; i<10; i++) {
      for(j=0; j<7; j++) {
        set_segment(j, seven_seg_digits[i][j]);
      }
      _delay_ms(1000);
    }
  }
}

/*
#define a_on segment_on(seg_a)
#define a_off segment_off(seg_a)
#define b_on() DDRC |= _BV(DDC5)
#define b_off() DDRC &= ~(_BV(DDC5))
#define c_on() DDRD |= _BV(DDD2)
#define c_off() DDRD &= ~(_BV(DDD2))
#define d_on() DDRC |= _BV(DDC2)
#define d_off() DDRC &= ~(_BV(DDC2))
#define e_on() DDRC |= _BV(DDC0)
#define e_off()DDRC &= ~(_BV(DDC0))
#define f_on() DDRB |= _BV(DDB3)
#define f_off()DDRB &= ~(_BV(DDB3))
#define g_on() DDRD |= _BV(DDD0)
#define g_off() DDRD &= ~(_BV(DDD0))

int main(void) {
  int i;


  for(;;) {
    //a_on();
    _delay_ms(1000);
    //a_off();
    _delay_ms(1000);
  }
}
*/


