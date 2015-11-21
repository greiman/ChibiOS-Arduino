// Example to demonstrate thread definition, semaphores, and thread sleep.
#include <ChibiOS_ARM.h>

// The LED is attached to pin 13 on Arduino.
const uint8_t LED_PIN = 13;

// Declare a semaphore with an inital counter value of zero.
SEMAPHORE_DECL(sem, 0);
//------------------------------------------------------------------------------
// Thread 1, turn the LED off when signalled by thread 2.

// 64 byte stack beyond task switch and interrupt needs
static THD_WORKING_AREA(waThread1, 64);

static THD_FUNCTION(Thread1, arg) {

  while (!chThdShouldTerminateX()) {
    // Wait for signal from thread 2.
    chSemWait(&sem);

    // Turn LED off.
    digitalWrite(LED_PIN, LOW);
  }
}
//------------------------------------------------------------------------------
// Thread 2, turn the LED on and signal thread 1 to turn the LED off.

// 64 byte stack beyond task switch and interrupt needs
static THD_WORKING_AREA(waThread2, 64);

static THD_FUNCTION(Thread2, arg) {
  pinMode(LED_PIN, OUTPUT);
  while (1) {
    digitalWrite(LED_PIN, HIGH);

    // Sleep for 200 milliseconds.
    chThdSleepMilliseconds(200);

    // Signal thread 1 to turn LED off.
    chSemSignal(&sem);

    // Sleep for 200 milliseconds.
    chThdSleepMilliseconds(200);
  }
}
//------------------------------------------------------------------------------
void setup() {

  chBegin(chSetup);
  // chBegin never returns, main thread continues with mainThread()
  while(1) {
  }
}
//------------------------------------------------------------------------------
// main thread runs at NORMALPRIO
void chSetup() {

  // start blink thread
  chThdCreateStatic(waThread1, sizeof(waThread1),
    NORMALPRIO + 2, Thread1, NULL);

  chThdCreateStatic(waThread2, sizeof(waThread2),
    NORMALPRIO + 1, Thread2, NULL);

}
//------------------------------------------------------------------------------
void loop() {
  // not used
}

