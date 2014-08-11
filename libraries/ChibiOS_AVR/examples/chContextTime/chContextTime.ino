// Connect a scope to pin 13
// Measure difference in time between first pulse with no context switch
// and second pulse started in thread 2 and ended in thread 1.
// Difference should be about 15-16 usec on a 16 MHz 328 Arduino.
#include <ChibiOS_AVR.h>

const uint8_t LED_PIN = 13;

// Semaphore to trigger context switch
Semaphore sem;
//------------------------------------------------------------------------------
// thread 1 - high priority thread to set pin low
// 64 byte stack beyond task switch and interrupt needs
static WORKING_AREA(waThread1, 64);

static msg_t Thread1(void *arg) {

  while (TRUE) {
    chSemWait(&sem);
    digitalWrite(LED_PIN, LOW);
  }
  return 0;
}
//------------------------------------------------------------------------------
// thread 2 - lower priority thread to toggle LED and trigger thread 1
// 64 byte stack beyond task switch and interrupt needs
static WORKING_AREA(waThread2, 64);

static msg_t Thread2(void *arg) {
  pinMode(LED_PIN, OUTPUT);
  while (TRUE) {
    // first pulse to get time with no context switch
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED_PIN, LOW);
    // start second pulse
    digitalWrite(LED_PIN, HIGH);
    // trigger context switch for task that ends pulse
    chSemSignal(&sem);
    // sleep until next tick (1024 microseconds tick on Arduino)
    chThdSleep(1);
  }
  return 0;
}
//------------------------------------------------------------------------------
void setup() {
  chBegin(chSetup);
  while (1) {}
}
//------------------------------------------------------------------------------
void chSetup() {
  // initialize semaphore
  chSemInit(&sem, 0);

  // start high priority thread
  chThdCreateStatic(waThread1, sizeof(waThread1),
    NORMALPRIO+2, Thread1, NULL);

  // start lower priority thread
  chThdCreateStatic(waThread2, sizeof(waThread2),
    NORMALPRIO+1, Thread2, NULL);
}
//------------------------------------------------------------------------------
void loop() {
  // not used
}