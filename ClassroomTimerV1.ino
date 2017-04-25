#include "PinChangeInt.h"
#include "TimerOne.h"

//We always have to include the library
#include "LedControlAC.h"

#define NBR_MTX 4
//LedControl(DataIn_Pin, Clock_Pin, Load_Pin, Number_of_Cascaded_Matrix_Displays);
LedControl lc = LedControl(2, 4, 3, NBR_MTX);

#define TIMER_DELAY 822 //One second

enum classroom_timer_states {
  STATE_STOPPED,
  STATE_COUNTING_DOWN,
  STATE_PAUSED,
};

#define BUTTON_PIN_STOP 10
#define BUTTON_PIN_START 11
#define BUTTON_PIN_UP 8
#define BUTTON_PIN_DOWN 9


char seconds_ones, seconds_tens, minutes_ones, minutes_tens;
char reload_seconds_ones, reload_seconds_tens, reload_minutes_ones, reload_minutes_tens;
char classroom_timer_state;

void reset_timer_update_display();
void decrement_update_display();
void button_attach_interrupts();
void button_press_ISR();
void increment_reload_values();
void decrement_reload_values();

void setup() {
  classroom_timer_state = STATE_COUNTING_DOWN;
  reload_seconds_ones = 0;
  reload_seconds_tens = 0;
  reload_minutes_ones = 1;
  reload_minutes_tens = 0;

  button_attach_interrupts();
  interrupts();

  for (int i = 0; i < NBR_MTX; i++) {
    lc.shutdown(i, false);
    lc.setIntensity(i, 8);
    lc.clearDisplay(i);
  }
  lc.clearAll();

  reset_timer_update_display();

  delay(TIMER_DELAY);
}

void loop() {
  switch (classroom_timer_state) {
    case STATE_STOPPED :
      break;
    case STATE_COUNTING_DOWN :
      decrement_update_display();
      delay(TIMER_DELAY);
      if ((seconds_ones == 0) && (seconds_tens == 0) && (minutes_ones == 0) && (minutes_tens == 0)) {
        delay(TIMER_DELAY);
        delay(TIMER_DELAY);
        delay(TIMER_DELAY);
        reset_timer_update_display();
        classroom_timer_state = STATE_STOPPED;
      }
      break;
    case STATE_PAUSED :
      break;
    default :
      classroom_timer_state = STATE_STOPPED;
      break;
  }
}




void button_press_ISR() {
  char latest_interrupted_pin;
  latest_interrupted_pin = PCintPort::arduinoPin;
  switch (latest_interrupted_pin) {
    case BUTTON_PIN_STOP: {
        switch (classroom_timer_state) {
          case STATE_STOPPED : classroom_timer_state = STATE_STOPPED; break;
          case STATE_COUNTING_DOWN : classroom_timer_state = STATE_PAUSED; break;
          case STATE_PAUSED : reset_timer_update_display(); classroom_timer_state = STATE_STOPPED; break;
          default : break;
        };
      }; break;
    case BUTTON_PIN_START: {
        switch (classroom_timer_state) {
          case STATE_STOPPED : classroom_timer_state = STATE_COUNTING_DOWN; break;
          case STATE_COUNTING_DOWN : break;
          case STATE_PAUSED : classroom_timer_state = STATE_COUNTING_DOWN; break;
          default : break;
        };
      }; break;
    case BUTTON_PIN_UP: {
        switch (classroom_timer_state) {
          case STATE_STOPPED : reset_timer_update_display(); increment_reload_values(); reset_timer_update_display(); break;
          case STATE_COUNTING_DOWN : break;
          case STATE_PAUSED : break;
          default : break;
        };
      }; break;
    case BUTTON_PIN_DOWN: {
        switch (classroom_timer_state) {
          case STATE_STOPPED : reset_timer_update_display(); decrement_reload_values(); reset_timer_update_display(); break;
          case STATE_COUNTING_DOWN : break;
          case STATE_PAUSED : break;
          default : break;
        };
      }; break;
    default: break;
  };

  //Detach all pinchange interrupts for debouncing purposes
  PCintPort::detachInterrupt(BUTTON_PIN_STOP);
  PCintPort::detachInterrupt(BUTTON_PIN_START);
  PCintPort::detachInterrupt(BUTTON_PIN_UP);
  PCintPort::detachInterrupt(BUTTON_PIN_DOWN);
  Timer1.initialize(1000000); // set a timer of length 1000000 microseconds (or 1 sec)
  Timer1.attachInterrupt( Timer1ISR ); // attach the service routine here
}

void Timer1ISR() //use Timer Interrupt to debounce
{
  Timer1.detachInterrupt();
  button_attach_interrupts();
}

void button_attach_interrupts() {
  //Stop (Red Button)
  pinMode(BUTTON_PIN_STOP, INPUT);  // set Pin as Input (default)
  digitalWrite(BUTTON_PIN_STOP, HIGH); // enable pullup resistor
  PCintPort::attachInterrupt(BUTTON_PIN_STOP, &button_press_ISR, FALLING);

  //Start (Green Button)
  pinMode(BUTTON_PIN_START, INPUT);  // set Pin as Input (default)
  digitalWrite(BUTTON_PIN_START, HIGH); // enable pullup resistor
  PCintPort::attachInterrupt(BUTTON_PIN_START, &button_press_ISR, FALLING);

  //Up (Yellow Button)
  pinMode(BUTTON_PIN_UP, INPUT);  // set Pin as Input (default)
  digitalWrite(BUTTON_PIN_UP, HIGH); // enable pullup resistor
  PCintPort::attachInterrupt(BUTTON_PIN_UP, &button_press_ISR, FALLING);

  //Down (Blue Button)
  pinMode(BUTTON_PIN_DOWN, INPUT);  // set Pin as Input (default)
  digitalWrite(BUTTON_PIN_DOWN, HIGH); // enable pullup resistor
  PCintPort::attachInterrupt(BUTTON_PIN_DOWN, &button_press_ISR, FALLING);
}

void increment_reload_values() {
  if ( reload_seconds_tens == 0) {
    reload_seconds_tens = 3;
  }
  else {
    reload_seconds_tens = 0;
    if (reload_minutes_ones == 9) {
      reload_minutes_tens++;
      reload_minutes_ones = 0;
    } else {
      reload_minutes_ones ++;
    }
    if (reload_minutes_tens >= 6) {
      reload_seconds_ones = 0;
      reload_seconds_tens = 3;
      reload_minutes_ones = 0;
      reload_minutes_tens = 0;
    }
  }
}

void decrement_reload_values() {
  if ( reload_seconds_tens == 3) {
    reload_seconds_tens = 0;
    if ( (reload_seconds_ones == 0) && (reload_seconds_tens == 0) && (reload_minutes_ones == 0) && (reload_minutes_tens == 0)) {
      reload_seconds_ones = 0;
      reload_seconds_tens = 3;
      reload_minutes_ones = 9;
      reload_minutes_tens = 5;
    }
  }
  else {
    reload_seconds_tens = 3;
    
    if (reload_minutes_ones == 0) {
      reload_minutes_tens--;
      reload_minutes_ones = 9;
    } else {
      reload_minutes_ones --;
    }
    if (reload_minutes_tens < 0) {
      reload_seconds_ones = 0;
      reload_seconds_tens = 3;
      reload_minutes_ones = 9;
      reload_minutes_tens = 5;
    }
  }
}

void decrement_update_display() {
  noInterrupts();
  seconds_ones--;
  if (seconds_ones < 0) {
    seconds_ones = 9;
    seconds_tens --;
  }
  if (seconds_tens < 0) {
    seconds_ones = 9;
    seconds_tens = 5;
    minutes_ones--;
  }
  if (minutes_ones < 0) {
    seconds_ones = 9;
    seconds_tens = 5;
    minutes_ones = 9;
    minutes_tens--;
  }
  if (minutes_tens < 0) {
    seconds_ones = 9;
    seconds_tens = 5;
    minutes_ones = 9;
    minutes_tens = 5;
  }
  lc.displayChar(0, lc.getCharArrayPosition(minutes_tens + 0x30));
  lc.displayChar(1, lc.getCharArrayPosition(minutes_ones + 0x30));
  lc.displayChar(2, lc.getCharArrayPosition(seconds_tens + 0x30));
  lc.displayChar(3, lc.getCharArrayPosition(seconds_ones + 0x30));
  lc.setLed(2, 2, 7, true);
  lc.setLed(2, 5, 7, true);
  interrupts();
}

void reset_timer_update_display() {
  noInterrupts();
  seconds_ones = reload_seconds_ones;
  seconds_tens = reload_seconds_tens;
  minutes_ones = reload_minutes_ones;
  minutes_tens = reload_minutes_tens;
  lc.displayChar(0, lc.getCharArrayPosition(minutes_tens + 0x30));
  lc.displayChar(1, lc.getCharArrayPosition(minutes_ones + 0x30));
  lc.displayChar(2, lc.getCharArrayPosition(seconds_tens + 0x30));
  lc.displayChar(3, lc.getCharArrayPosition(seconds_ones + 0x30));
  lc.setLed(2, 2, 7, true);
  lc.setLed(2, 5, 7, true);
  interrupts();
}


