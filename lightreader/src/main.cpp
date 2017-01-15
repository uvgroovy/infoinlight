
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


#include <Arduino.h>
//#include <FreqCount/FreqCount.h>
#include <FreqMeasure/FreqMeasure.h>

#define PRESACEL1024 (_BV(CS02) |  _BV(CS00))
#define PRESACEL256 (_BV(CS02) )
#define PRESACEL64  (_BV(CS01) |  _BV(CS00))
#define PRESACEL8  (_BV(CS01) )
#define PRESACEL1  (_BV(CS00))


void setup() {
    Serial.begin(57600);
    // FreqMeasure uses pin 8 on arduino uno
    FreqMeasure.begin();
    Serial.println("high 2! low! hello");
}


// ok so:
// 1000hz is sync frequency - when it changes we need to measure
// 667hz is one
// 800hz is zero

#define SYNC_FREQ_THRESH 900
#define ZERO_FREQ_THRESH 725
#define BOTTOM_FREQ_THRESH 650
#define TOP_FREQ_THRESH 810


// save frequencies here so we can average them out dynamically later..
#define FREQ_BUF_SIZE 256
float freqs_buff[FREQ_BUF_SIZE] = {0.0};
int freqs_buff_ind = 0;

int getid();


void loop() {
static bool begin = false;

  if (FreqMeasure.available()) {
    float frequency = FreqMeasure.countToFrequency(FreqMeasure.read());
      if (frequency < SYNC_FREQ_THRESH) {
        // frequency is below 1000hz, we can measure
          begin = false;
          freqs_buff[freqs_buff_ind] = frequency;
          freqs_buff_ind++;
          if (freqs_buff_ind == FREQ_BUF_SIZE) {
            freqs_buff_ind = 0;
            Serial.println("Index overflow error!!! this reading will not work");
          }
      } else if (begin == false) {
          // frequency is above 1000hz - it is sync time.
          // only do this the first time we hit sync frequency.
          // compute the id:
          int b= getid();
          Serial.print("ID ");
          Serial.println(b);

          // zero ingex, so that after sync we get fresh capture
          freqs_buff_ind = 0;
          begin = true;
      }
  }
}

int getid() {

    // time to parse
    int id = 0;

    // round down to nearest 8
    int size = freqs_buff_ind & ~7;
    int seg = size >> 3;

    if (size > FREQ_BUF_SIZE) {
      Serial.println("too many samples");
      return -1;
    }
    if (seg < 15) {
      Serial.print(seg);
      Serial.println(" too little samples");
      return -1;
    }

//    Serial.println("AAA");

  // average frequncies to get bits
    for(int i = 0; i < 8; i++) {
      float avg = 0;
      for(int j = 0; j < seg; ++j) {
        avg += freqs_buff[i*seg+j];
      }
      avg = avg/seg;

//      Serial.println(avg);
      if (avg < BOTTOM_FREQ_THRESH) {
        // that shouldn't happen..
        return -1;
      }
      if (avg > TOP_FREQ_THRESH) {
        // that shouldn't happen..
        return -1;
      }

      id <<= 1;
      if (avg < ZERO_FREQ_THRESH) {
        // freq is around 667, we have a 1 bit
        id |= 1;
      }
    }

    /*
if (id != 49) {
Serial.println("BBB");
Serial.println(freqs_buff_ind);

    for(int i = 0; i < freqs_buff_ind; i++) {
      Serial.println(freqs_buff[i]);
    }
    Serial.println("CCC");
}
*/

    return id;
}
