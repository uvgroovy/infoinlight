
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>


#include <Arduino.h>
#include <TimerOne.h>


#define ONE_FREQ 1500 // 667 hz
#define ZERO_FREQ 1250 // 800hz
#define SYNC_FREQ 1000 // 1000 hz

#define ID_BIT_FREQ(i) (((ID_OF_DEV >> i ) & 0x1) ? ONE_FREQ : ZERO_FREQ )
#define NEED_REFRESH_CYCLE(i) (((ID_OF_DEV >> i ) & 0x1) ? ONE_FREQ : ZERO_FREQ )

volatile int PWM = 512;

#define OUTPUT_PIN 9


void start_timer() {
  pinMode(OUTPUT_PIN, OUTPUT);
  Timer1.initialize(SYNC_FREQ);   // 1ms = 1000hz
  Timer1.setPwmDuty(OUTPUT_PIN, PWM);

// TIMER2 not working from some reason :()
  // use timer two so we don't break millis (that uses timer0)
  #define PRESACEL1024 (_BV(CS02) |  _BV(CS00))

  TIMSK0 = _BV(OCIE0A);   // Enable Interrupt TimerCounter0 Compare Match A
  TCCR0A = _BV(WGM01);    // Mode = CTC
  TCCR0B = PRESACEL1024;  // Clock/1024,
  OCR0A = 0xFF;    // about 61 ticks per second
  // technically could have done it with normal timer and overflow interrupt..
  // but this was more flexible for my experiments.. whatever..


  set_sleep_mode(SLEEP_MODE_IDLE);
  // sleep_enable();

  Timer1.start();
  Timer1.pwm(OUTPUT_PIN, PWM);
}

void setup() {

  // get my id in the chain...

  start_timer();

}

const long interval = 50;

void loop() {
  return;
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if ((currentMillis - previousMillis) < interval) {
    return;
  }
  // sleep_cpu();

    PWM += 10;
    if (PWM >= 900) {
      PWM = 50;
      Timer1.setPwmDuty(OUTPUT_PIN, PWM);
    }
}

long int intrrupt_i = 0;
// gets called about every 61ms
volatile int ID_OF_DEV = 49;

ISR(TIMER0_COMPA_vect, ISR_BLOCK) {

    #define NEED_REFRESH_CYCLE(i) ( ((ID_OF_DEV >> i ) & 1) ^ ((ID_OF_DEV >> (i+1))  & 1 ) )

    intrrupt_i++;
//    intrrupt_i %= 512;
//    intrrupt_i &= (32-1);
    intrrupt_i %= 20;

    switch (intrrupt_i) {
    case 0:
      Timer1.setPeriod(ID_BIT_FREQ(7));
      Timer1.setPwmDuty(OUTPUT_PIN, PWM);
      break;
    case 2*1:
      if (NEED_REFRESH_CYCLE(6)) {
        Timer1.setPeriod(ID_BIT_FREQ(6));
        Timer1.setPwmDuty(OUTPUT_PIN, PWM);
      }
      break;
    case 2*2:
      if (NEED_REFRESH_CYCLE(5)) {
        Timer1.setPeriod(ID_BIT_FREQ(5));
        Timer1.setPwmDuty(OUTPUT_PIN, PWM);
      }
      break;
    case 2*3:
      if (NEED_REFRESH_CYCLE(4)) {
        Timer1.setPeriod(ID_BIT_FREQ(4));
        Timer1.setPwmDuty(OUTPUT_PIN, PWM);
      }
      break;
    case 2*4:
      if (NEED_REFRESH_CYCLE(3)) {
        Timer1.setPeriod(ID_BIT_FREQ(3));
        Timer1.setPwmDuty(OUTPUT_PIN, PWM);
      }
      break;
    case 2*5:
      if (NEED_REFRESH_CYCLE(2)) {
        Timer1.setPeriod(ID_BIT_FREQ(2));
        Timer1.setPwmDuty(OUTPUT_PIN, PWM);
      }
      break;
    case 2*6:
      if (NEED_REFRESH_CYCLE(1)) {
        Timer1.setPeriod(ID_BIT_FREQ(1));
        Timer1.setPwmDuty(OUTPUT_PIN, PWM);
      }
      break;
    case 2*7:
      if (NEED_REFRESH_CYCLE(0)) {
        Timer1.setPeriod(ID_BIT_FREQ(0));
        Timer1.setPwmDuty(OUTPUT_PIN, PWM);
      }
      break;
    case 2*8:
      // done transmitting bits, set sync frequncy
      Timer1.setPeriod(SYNC_FREQ);
      // changing the period so need to re-calc the duty cycle
      Timer1.setPwmDuty(OUTPUT_PIN, PWM);
      break;
    }
}
